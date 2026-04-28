// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRGameplayAbility.h"
#include "MRAbility_ShieldMode.generated.h"

/**
 * 한손검 방패 모드 토글 어빌리티.
 *
 * 흐름:
 *   첫 입력 → ActivateAbility → Character.State.ShieldMode 태그 부착, 어빌리티 유지
 *   재입력 → MRPlayerCharacter에서 IsActive 확인 후 CancelAbility
 *   EndAbility → Character.State.ShieldMode 태그 제거
 *
 * 방패 모드 중 공격/피격 시 해제는 외부(공격 어빌리티, 피격 핸들러)에서 CancelAbility로 처리.
 */
UCLASS(Blueprintable)
class MR_API UMRAbility_ShieldMode : public UMRGameplayAbility
{
	GENERATED_BODY()

public:
	UMRAbility_ShieldMode();

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
};
