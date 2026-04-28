// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRGameplayAbility.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayEffectTypes.h"
#include "MREffect_StaminaDrain.h"
#include "MRAbility_Sprint.generated.h"

/**
 * 스프린트 어빌리티.
 * 입력을 누르는 동안 MaxWalkSpeed를 높이고 "Character.State.Sprinting" 태그를 유지한다.
 * GE_StaminaDrain을 통해 스태미너를 소모하며, 소모 중에는 회복이 차단된다.
 */
UCLASS(Blueprintable)
class MR_API UMRAbility_Sprint : public UMRGameplayAbility
{
	GENERATED_BODY()

public:
	UMRAbility_Sprint();

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

	/** 스프린트 중 MaxWalkSpeed 값 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sprint")
	float SprintSpeed = 900.f;

	/** 스태미너 드레인 GE - BP 서브클래스를 지정 */
	UPROPERTY(EditDefaultsOnly, Category = "Sprint|Effects")
	TSubclassOf<UMREffect_StaminaDrain> StaminaDrainEffectClass;

private:
	/** 스프린트 전 원래 속도 저장 */
	float OriginalMaxWalkSpeed = 0.f;

	/** 현재 적용 중인 드레인 GE 핸들 */
	FActiveGameplayEffectHandle DrainEffectHandle;

	/** 스태미너 고갈 감지 콜백 */
	void OnStaminaChanged(const FOnAttributeChangeData& Data);
};
