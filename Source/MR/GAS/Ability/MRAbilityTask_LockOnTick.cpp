#include "MRAbilityTask_LockOnTick.h"
#include "MRGameplayTags.h"
#include "MRPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"

UMRAbilityTask_LockOnTick* UMRAbilityTask_LockOnTick::CreateTask(
	UGameplayAbility* OwningAbility,
	AActor* InTarget,
	float InMaxDistance,
	float InCameraInterpSpeed)
{
	UMRAbilityTask_LockOnTick* Task = NewAbilityTask<UMRAbilityTask_LockOnTick>(OwningAbility);
	Task->bTickingTask      = true;
	Task->LockedTarget      = InTarget;
	Task->MaxDistance       = InMaxDistance;
	Task->CameraInterpSpeed = InCameraInterpSpeed;
	return Task;
}

void UMRAbilityTask_LockOnTick::Activate()
{
	Super::Activate();

	if (!IsTargetValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("MRAbilityTask_LockOnTick: 초기 대상이 유효하지 않아 즉시 종료합니다."));
		OnTargetLost.Broadcast();
		EndTask();
	}
}

void UMRAbilityTask_LockOnTick::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	if (!IsTargetValid())
	{
		UE_LOG(LogTemp, Log, TEXT("MRAbilityTask_LockOnTick: 대상 유효성 상실 — 록온 해제"));
		OnTargetLost.Broadcast();
		EndTask();
		return;
	}

	AMRPlayerCharacter* Player = Cast<AMRPlayerCharacter>(GetAvatarActor());
	if (!Player)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(Player->GetController());
	if (!PC)
	{
		return;
	}

	UCameraComponent* Camera = Player->GetFollowCamera();
	if (!Camera)
	{
		return;
	}

	// 카메라 위치에서 타겟 방향 벡터 계산
	const FVector ToTarget = LockedTarget->GetActorLocation() - Camera->GetComponentLocation();
	FRotator TargetRot = ToTarget.Rotation();

	// 카메라 피치를 너무 올리거나 내리지 않도록 클램프, Roll은 항상 0
	TargetRot.Pitch = FMath::Clamp(TargetRot.Pitch, -30.f, 10.f);
	TargetRot.Roll  = 0.f;

	const FRotator NewRot = FMath::RInterpTo(PC->GetControlRotation(), TargetRot, DeltaTime, CameraInterpSpeed);
	PC->SetControlRotation(NewRot);
}

void UMRAbilityTask_LockOnTick::OnDestroy(bool bInOwnerFinished)
{
	Super::OnDestroy(bInOwnerFinished);
}

bool UMRAbilityTask_LockOnTick::IsTargetValid() const
{
	if (!LockedTarget.IsValid())
	{
		return false;
	}

	// 대상의 ASC에서 Dead 태그 확인
	if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(LockedTarget.Get()))
	{
		if (UAbilitySystemComponent* TargetASC = ASI->GetAbilitySystemComponent())
		{
			if (TargetASC->HasMatchingGameplayTag(MRGameplayTags::Character_State_Dead))
			{
				return false;
			}
		}
	}

	// 플레이어와 대상 사이의 거리 확인
	AActor* Avatar = GetAvatarActor();
	if (Avatar && FVector::DistSquared(Avatar->GetActorLocation(), LockedTarget->GetActorLocation()) > FMath::Square(MaxDistance))
	{
		return false;
	}

	return true;
}
