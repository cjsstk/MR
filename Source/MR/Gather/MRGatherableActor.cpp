// Fill out your copyright notice in the Description page of Project Settings.

#include "Gather/MRGatherableActor.h"
#include "Gather/MRGatherHelper.h"
#include "MRAbility_Gather.h"
#include "MRPlayerCharacter.h"
#include "Subsystem/CMSSubsystem.h"
#include "MRStrongId.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMesh.h"
#include "Animation/AnimMontage.h"
#include "Engine/GameInstance.h"

AMRGatherableActor::AMRGatherableActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);

	// 인터랙션 볼륨. 평상시에는 QueryOnly로 Pawn 오버랩만 감지.
	InteractionVolume = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(MeshComponent);
	InteractionVolume->SetSphereRadius(200.f);
	InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AMRGatherableActor::BeginPlay()
{
	Super::BeginPlay();

	UGameInstance* GI = GetGameInstance();
	UCMSSubsystem* CMS = GI ? GI->GetSubsystem<UCMSSubsystem>() : nullptr;
	if (!CMS)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MRGatherableActor] %s: CMSSubsystem 없음"), *GetName());
		InteractionVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		return;
	}

	CachedRow = CMS->GetGatherableRow(GatherableType);
	if (!CachedRow)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MRGatherableActor] %s: GatherableType=%d Row 없음 — 비활성화"),
			*GetName(), GatherableType);
		InteractionVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		return;
	}

	RemainingGathers = CachedRow->GatherCount;

	// 메시 소프트 참조가 지정되어 있으면 로드해 반영 (에디터에서 직접 지정하지 않은 경우 대비)
	if (!CachedRow->Mesh.IsNull())
	{
		if (UStaticMesh* Mesh = CachedRow->Mesh.LoadSynchronous())
		{
			MeshComponent->SetStaticMesh(Mesh);
		}
	}

	// 오버랩 이벤트 바인딩
	InteractionVolume->OnComponentBeginOverlap.AddDynamic(this, &AMRGatherableActor::OnGatherVolumeOverlapBegin);
	InteractionVolume->OnComponentEndOverlap.AddDynamic(this, &AMRGatherableActor::OnGatherVolumeOverlapEnd);

	UE_LOG(LogTemp, Log, TEXT("[MRGatherableActor] %s 초기화: Type=%d GatherCount=%d"),
		*GetName(), GatherableType, RemainingGathers);
}

void AMRGatherableActor::GetGatherSpec(FMRGatherSpec& OutSpec) const
{
	if (!CachedRow)
	{
		return;
	}

	OutSpec.MovementPolicy  = CachedRow->MovementPolicy;
	OutSpec.GatherType      = CachedRow->GatherType;
	OutSpec.InteractionText = CachedRow->InteractionText;
}

void AMRGatherableActor::PerformGather(UMRAbility_Gather* Ability)
{
	if (!CanBeGathered())
	{
		UE_LOG(LogTemp, Warning, TEXT("[MRGatherableActor] PerformGather 실패: 채집 불가 상태 (bDepleted=%d, Remaining=%d)"),
			bDepleted, RemainingGathers);
		return;
	}

	if (!Ability || !CachedRow)
	{
		return;
	}

	// 공용 헬퍼로 드롭 계산 + 인벤토리 지급 + 결과 팝업 디스패치
	AMRPlayerCharacter* Player = Cast<AMRPlayerCharacter>(Ability->GetAvatarActorFromActorInfo());
	const FMRDropResult Result = UMRGatherHelper::GrantDropToPlayer(Player, FDropTableId(CachedRow->DropTableId));

	UE_LOG(LogTemp, Log, TEXT("[MRGatherableActor] %s 채집: ItemId=%d x%d (남은 횟수 %d → %d)"),
		*GetName(), Result.ItemId, Result.Count, RemainingGathers, RemainingGathers - 1);

	// 결과 유무와 무관하게 채집 시도 1회로 카운트 소모
	if (--RemainingGathers <= 0)
	{
		EnterDepletedAndScheduleRespawn();
	}
}

void AMRGatherableActor::OnGatherVolumeOverlapBegin(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	MRGatherableInteract::EnterRange(this, OtherActor);
}

void AMRGatherableActor::OnGatherVolumeOverlapEnd(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	MRGatherableInteract::ExitRange(this, OtherActor);
}

void AMRGatherableActor::EnterDepletedAndScheduleRespawn()
{
	bDepleted = true;
	SetActorHiddenInGame(true);
	InteractionVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 범위 내에 남아있는 플레이어의 프롬프트를 수동으로 정리한다 (오버랩 End가 발생하지 않으므로).
	TArray<AActor*> Overlapping;
	GetOverlappingActors(Overlapping, AMRPlayerCharacter::StaticClass());
	for (AActor* Actor : Overlapping)
	{
		MRGatherableInteract::ExitRange(this, Actor);
	}

	const float Delay = CachedRow ? CachedRow->RespawnDelay : 60.f;
	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AMRGatherableActor::Respawn, Delay, false);

	UE_LOG(LogTemp, Log, TEXT("[MRGatherableActor] %s 채집 소진 — %.1f초 후 리스폰"), *GetName(), Delay);
}

void AMRGatherableActor::Respawn()
{
	bDepleted = false;
	RemainingGathers = CachedRow ? CachedRow->GatherCount : RemainingGathers;
	SetActorHiddenInGame(false);
	InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	// 리스폰 시점에 이미 범위 내에 있는 플레이어는 오버랩 Begin이 발생하지 않으므로 수동 재감지.
	TArray<AActor*> Overlapping;
	GetOverlappingActors(Overlapping, AMRPlayerCharacter::StaticClass());
	for (AActor* Actor : Overlapping)
	{
		MRGatherableInteract::EnterRange(this, Actor);
	}

	UE_LOG(LogTemp, Log, TEXT("[MRGatherableActor] %s 리스폰: 채집 횟수 %d"), *GetName(), RemainingGathers);
}
