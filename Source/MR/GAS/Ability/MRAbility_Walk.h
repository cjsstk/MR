// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRGameplayAbility.h"
#include "MRAbility_Walk.generated.h"

class UBlendSpace;

/**
 * 플레이어 이동(걷기/달리기) 어빌리티.
 * "Character.State.Moving" 태그를 관리하며, AnimBP가 이 태그로 이동/대기 상태를 전환한다.
 * 애니메이션 에셋 로딩은 AMRPlayerCharacter::LoadAndApplyWeaponAnims에서 담당한다.
 */
UCLASS(Blueprintable)
class MR_API UMRAbility_Walk : public UMRGameplayAbility
{
	GENERATED_BODY()

public:
	UMRAbility_Walk();

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
