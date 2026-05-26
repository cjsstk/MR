// Fill out your copyright notice in the Description page of Project Settings.

#include "MRAnimNotify_SpawnProjectile.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "MRGameplayTags.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"

#if ENABLE_DRAW_DEBUG
static TAutoConsoleVariable<int32> CVarProjectileDebug(
	TEXT("mr.ProjectileDebug"),
	0,
	TEXT("활 발사체 디버그 표시 (0=꺼짐, 1=켜짐). 발사 방향 라인 5초 표시"),
	ECVF_Default
);
#endif

FString UMRAnimNotify_SpawnProjectile::GetNotifyName_Implementation() const
{
	return TEXT("SpawnProjectile");
}

void UMRAnimNotify_SpawnProjectile::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SpawnProjectile] MeshComp or ProjectileClass is null"));
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		return;
	}

	// ASC 보유 여부 확인 — 발사 이벤트 전송에 필요
	IAbilitySystemInterface* ASCOwner = Cast<IAbilitySystemInterface>(Owner);
	if (!ASCOwner)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SpawnProjectile] Owner %s has no ASC"), *Owner->GetName());
		return;
	}
	UAbilitySystemComponent* ASC = ASCOwner->GetAbilitySystemComponent();

	// 소켓 위치: 존재하면 소켓 사용, 없으면 액터 위치
	const bool bSocketExists = !SpawnSocketName.IsNone() && MeshComp->DoesSocketExist(SpawnSocketName);
	const FVector SpawnLocation = bSocketExists
		? MeshComp->GetSocketLocation(SpawnSocketName)
		: Owner->GetActorLocation();

	// ── 발사 방향 계산 ──────────────────────────────────────────────────────
	FVector LaunchDir;

	const bool bAiming = ASC && ASC->HasMatchingGameplayTag(MRGameplayTags::Character_State_Aiming);
	if (bAiming)
	{
		// 조준 모드: 카메라 중심에서 LineTrace → 목표점 계산
		FVector CameraLocation;
		FRotator CameraRotation;

		APawn* PawnOwner = Cast<APawn>(Owner);
		APlayerController* PC = PawnOwner ? Cast<APlayerController>(PawnOwner->GetController()) : nullptr;

		if (PC && PC->PlayerCameraManager)
		{
			CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
			CameraRotation = PC->PlayerCameraManager->GetCameraRotation();
		}
		else
		{
			// 폴백: 액터 눈높이 위치 + 제어 회전
			CameraLocation = Owner->GetActorLocation() + FVector(0.f, 0.f, 80.f);
			CameraRotation = Owner->GetInstigatorController()
				? Owner->GetInstigatorController()->GetControlRotation()
				: Owner->GetActorRotation();
		}

		const FVector TraceEnd = CameraLocation + CameraRotation.Vector() * AimTraceDistance;

		FHitResult TraceHit;
		FCollisionQueryParams TraceParams;
		TraceParams.AddIgnoredActor(Owner);

		FVector TargetPoint;
		if (Owner->GetWorld()->LineTraceSingleByChannel(
			TraceHit,
			CameraLocation,
			TraceEnd,
			ECC_Visibility,
			TraceParams))
		{
			TargetPoint = TraceHit.ImpactPoint;
		}
		else
		{
			TargetPoint = TraceEnd;
		}

		// 소켓 → 목표점 방향
		LaunchDir = (TargetPoint - SpawnLocation).GetSafeNormal();
	}
	else
	{
		// 비조준 모드: 캐릭터 Right 벡터를 축으로 전방을 위로 기울임
		// FRotator::RotateVector는 월드 축 기준이므로 캐릭터 방향과 무관하게 틀어질 수 있다.
		// 음의 각도를 사용해야 위(+Z) 방향으로 기울어진다.
		const FVector Forward = Owner->GetActorForwardVector();
		const FVector Right   = Owner->GetActorRightVector();
		const FQuat PitchQuat(Right, FMath::DegreesToRadians(-UnamedLaunchPitchOffset));
		LaunchDir = PitchQuat.RotateVector(Forward).GetSafeNormal();
	}

	// ── 발사체 스폰 ─────────────────────────────────────────────────────────
	UWorld* World = Owner->GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator  = Cast<APawn>(Owner);
	SpawnParams.Owner       = Owner;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 발사 방향으로 발사체 회전 설정
	const FRotator SpawnRotation = LaunchDir.Rotation();

	AMRProjectile* Projectile = World->SpawnActor<AMRProjectile>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (!Projectile)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SpawnProjectile] SpawnActor failed for class %s"), *ProjectileClass->GetName());
		return;
	}

	Projectile->InitProjectile(Owner, LaunchDir, LaunchSpeed, GravityScale);

	UE_LOG(LogTemp, Log, TEXT("[SpawnProjectile] Spawned %s at %s, Dir: %s, Speed: %.1f, Aiming: %s"),
		*Projectile->GetName(),
		*SpawnLocation.ToString(),
		*LaunchDir.ToString(),
		LaunchSpeed,
		bAiming ? TEXT("true") : TEXT("false"));

#if ENABLE_DRAW_DEBUG
	if (CVarProjectileDebug.GetValueOnGameThread() != 0)
	{
		DrawDebugLine(
			World,
			SpawnLocation,
			SpawnLocation + LaunchDir * 500.f,
			FColor::Yellow,
			false,
			5.f,
			0,
			2.f
		);
	}
#endif
}
