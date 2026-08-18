// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/MRGatherableComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Subsystem/CMSSubsystem.h"
#include "Engine/GameInstance.h"
#include "MRAbility_Gather.h"

// Sets default values for this component's properties
UMRGatherableComponent::UMRGatherableComponent(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	// 인터랙션 볼륨. 평상시에는 QueryOnly로 Pawn 오버랩만 감지.
	InteractionVolume = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(this);
	InteractionVolume->SetSphereRadius(200.f);
	InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	InteractionWidget = ObjectInitializer.CreateDefaultSubobject<UWidgetComponent>(this, TEXT("InteractionWidget"));
	InteractionWidget->SetupAttachment(this);
}

void UMRGatherableComponent::BeginPlay()
{
	Super::BeginPlay();

	InteractionVolume->OnComponentBeginOverlap.AddDynamic(this, &UMRGatherableComponent::OnGatherVolumeOverlapBegin);
	InteractionVolume->OnComponentEndOverlap.AddDynamic(this, &UMRGatherableComponent::OnGatherVolumeOverlapEnd);
	
	// ...
	
}

void UMRGatherableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UMRGatherableComponent::OnGatherVolumeOverlapBegin(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	//MRGatherableInteract::EnterRange(this, OtherActor);
	
	InteractionWidget->SetVisibility(true);
}

void UMRGatherableComponent::OnGatherVolumeOverlapEnd(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	//MRGatherableInteract::ExitRange(this, OtherActor);
	
	InteractionWidget->SetVisibility(false);
}

void UMRGatherableComponent::EnterDepletedAndScheduleRespawn()
{
	// bDepleted = true;
	// SetActorHiddenInGame(true);
	// InteractionVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//
	// // 범위 내에 남아있는 플레이어의 프롬프트를 수동으로 정리한다 (오버랩 End가 발생하지 않으므로).
	// TArray<AActor*> Overlapping;
	// GetOverlappingActors(Overlapping, AMRPlayerCharacter::StaticClass());
	// for (AActor* Actor : Overlapping)
	// {
	// 	MRGatherableInteract::ExitRange(this, Actor);
	// }
	//
	// const float Delay = CachedRow ? CachedRow->RespawnDelay : 60.f;
	// GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AMRGatherableActor::Respawn, Delay, false);
	//
	// UE_LOG(LogTemp, Log, TEXT("[MRGatherableActor] %s 채집 소진 — %.1f초 후 리스폰"), *GetName(), Delay);
}

void UMRGatherableComponent::Respawn()
{
	// bDepleted = false;
	// RemainingGathers = CachedRow ? CachedRow->GatherCount : RemainingGathers;
	// SetActorHiddenInGame(false);
	// InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//
	// // 리스폰 시점에 이미 범위 내에 있는 플레이어는 오버랩 Begin이 발생하지 않으므로 수동 재감지.
	// TArray<AActor*> Overlapping;
	// GetOverlappingActors(Overlapping, AMRPlayerCharacter::StaticClass());
	// for (AActor* Actor : Overlapping)
	// {
	// 	MRGatherableInteract::EnterRange(this, Actor);
	// }
	//
	// UE_LOG(LogTemp, Log, TEXT("[MRGatherableActor] %s 리스폰: 채집 횟수 %d"), *GetName(), RemainingGathers);
}