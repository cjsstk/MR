// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRGameplayAbility.h"
#include "MRAbility_Aim.generated.h"

/**
 * 활 조준 어빌리티.
 *
 * 흐름:
 *   RMB 누름 → ActivateAbility → Character.State.Aiming 태그 부착, 컨트롤러 회전 활성화
 *   RMB 해제 → MRPlayerCharacter::OnHeavyAttackCompleted → HeavyAttackAbilityHandle CancelAbility
 *   EndAbility → Aiming 태그 제거, 회전 설정 복원
 *
 * 조준 중 발사(LMB)는 AttackAbility에서 Aiming 태그를 확인하여 처리.
 */
UCLASS(Blueprintable)
class MR_API UMRAbility_Aim : public UMRGameplayAbility
{
	GENERATED_BODY()

public:
	UMRAbility_Aim();

	/** 조준 중 이동 속도. BP에서 조정 가능. */
	UPROPERTY(EditDefaultsOnly, Category = "Aim")
	float AimMoveSpeed = 200.f;

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

private:
	float OriginalMaxWalkSpeed = 0.f;
};
