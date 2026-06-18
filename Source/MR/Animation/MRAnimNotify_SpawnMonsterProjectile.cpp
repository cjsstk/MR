// Fill out your copyright notice in the Description page of Project Settings.

#include "MRAnimNotify_SpawnMonsterProjectile.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "MRAIController.h"

FString UMRAnimNotify_SpawnMonsterProjectile::GetNotifyName_Implementation() const
{
	return TEXT("SpawnMonsterProjectile");
}

void UMRAnimNotify_SpawnMonsterProjectile::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SpawnMonsterProjectile] MeshComp or ProjectileClass is null"));
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		return;
	}

	// 소켓 위치: 존재하면 소켓 사용, 없으면 액터 위치
	const bool bSocketExists = !SpawnSocketName.IsNone() && MeshComp->DoesSocketExist(SpawnSocketName);
	const FVector SpawnLocation = bSocketExists
		? MeshComp->GetSocketLocation(SpawnSocketName)
		: Owner->GetActorLocation();

	// ── 발사 방향 계산 ──────────────────────────────────────────────────────
	// 기본값: 캐릭터 전방. BB에 TargetActor가 있으면 그 방향으로 재계산.
	FVector LaunchDir = Owner->GetActorForwardVector();

	if (APawn* PawnOwner = Cast<APawn>(Owner))
	{
		if (AAIController* AIC = Cast<AAIController>(PawnOwner->GetController()))
		{
			if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
			{
				if (AActor* Target = Cast<AActor>(BB->GetValueAsObject(AMRAIController::BBKey_TargetActor)))
				{
					const FVector ToTarget = Target->GetActorLocation() - SpawnLocation;
					if (!ToTarget.IsNearlyZero())
					{
						LaunchDir = ToTarget.GetSafeNormal();
					}
				}
			}
		}
	}

	// ── 발사체 스폰 ─────────────────────────────────────────────────────────
	UWorld* World = Owner->GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = Cast<APawn>(Owner);
	SpawnParams.Owner = Owner;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FRotator SpawnRotation = LaunchDir.Rotation();

	AMRProjectile* Projectile = World->SpawnActor<AMRProjectile>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams);

	if (!Projectile)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SpawnMonsterProjectile] SpawnActor failed for class %s"),
			*ProjectileClass->GetName());
		return;
	}

	Projectile->InitProjectile(Owner, LaunchDir, LaunchSpeed, GravityScale);

	UE_LOG(LogTemp, Log, TEXT("[SpawnMonsterProjectile] Spawned %s at %s, Dir: %s, Speed: %.1f"),
		*Projectile->GetName(),
		*SpawnLocation.ToString(),
		*LaunchDir.ToString(),
		LaunchSpeed);
}
