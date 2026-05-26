// Fill out your copyright notice in the Description page of Project Settings.

#include "MRProjectile.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "MRGameplayTags.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AMRProjectile::AMRProjectile(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	// 루트: 콜리전 구체
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(10.f);

	// Pawn → Overlap (히트 이벤트 발송)
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn,        ECR_Overlap);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic,  ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);

	SetRootComponent(CollisionComponent);

	// 시각적 메시 (BP에서 메시 에셋 설정)
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CollisionComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 발사체 이동
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 5000.f;
	ProjectileMovement->MaxSpeed = 10000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.3f;

	// 충돌 이벤트 바인딩
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AMRProjectile::OnBeginOverlap);
	CollisionComponent->OnComponentHit.AddDynamic(this, &AMRProjectile::OnHit);
}

void AMRProjectile::InitProjectile(AActor* InInstigatorActor, const FVector& LaunchDirection, float Speed, float GravityScale)
{
	// 발사한 캐릭터를 Instigator로 등록
	SetInstigator(Cast<APawn>(InInstigatorActor));

	// 발사체 자신을 발사한 캐릭터와의 충돌에서 제외
	if (InInstigatorActor)
	{
		CollisionComponent->MoveIgnoreActors.Add(InInstigatorActor);
	}

	// 발사 속도 및 중력 설정
	ProjectileMovement->Velocity = LaunchDirection * Speed;
	ProjectileMovement->ProjectileGravityScale = GravityScale;

	// 수명 설정 — 시간이 지나면 자동 소멸
	SetLifeSpan(LifeSpan);

	UE_LOG(LogTemp, Log, TEXT("[MRProjectile] InitProjectile - Instigator: %s, Direction: %s, Speed: %.1f, Gravity: %.2f"),
		InInstigatorActor ? *InInstigatorActor->GetName() : TEXT("None"),
		*LaunchDirection.ToString(), Speed, GravityScale);
}

void AMRProjectile::OnBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// Pawn 채널 오버랩 = ASC를 가진 캐릭터 히트 처리
	if (OtherActor && OtherActor != GetInstigator() && Cast<IAbilitySystemInterface>(OtherActor))
	{
		HandleImpact(OtherActor);
	}
}

void AMRProjectile::OnHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	// WorldStatic / WorldDynamic 충돌 = 지형/사물에 박힘 → 소멸
	UE_LOG(LogTemp, Log, TEXT("[MRProjectile] Hit static geometry: %s"), OtherActor ? *OtherActor->GetName() : TEXT("None"));
	Destroy();
}

void AMRProjectile::HandleImpact(AActor* HitActor)
{
	if (!HitActor)
	{
		return;
	}

	// 중복 히트 방지
	if (HitActors.Contains(HitActor))
	{
		return;
	}
	HitActors.Add(HitActor);

	// 발사자 ASC에 Event.Attack.Hit 이벤트 전송
	// BowAttack 어빌리티의 WaitGameplayEvent 태스크가 이를 수신해 데미지를 적용한다.
	APawn* InstigatorPawn = GetInstigator();
	if (!InstigatorPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MRProjectile] HandleImpact - Instigator is null, cannot send hit event"));
		Destroy();
		return;
	}

	FGameplayEventData EventData;
	EventData.Instigator = InstigatorPawn;
	EventData.Target     = HitActor;

	UE_LOG(LogTemp, Log, TEXT("[MRProjectile] HandleImpact - Instigator: %s → Target: %s"),
		*InstigatorPawn->GetName(), *HitActor->GetName());

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		InstigatorPawn,
		MRGameplayTags::Event_Attack_Hit,
		EventData
	);

	Destroy();
}
