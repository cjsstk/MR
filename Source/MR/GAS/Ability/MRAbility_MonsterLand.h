// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRGameplayAbility.h"
#include "MRAbility_MonsterLand.generated.h"

class UAbilityTask_PlayMontageAndWait;

/**
 * 몬스터 착지 어빌리티.
 * Land 몬타주를 재생하고 완료 시 지상 상태로 전환한다.
 * 착지 전환: Character.State.Flying 태그 제거, CharacterMovement MOVE_Walking, BB IsFlying=false.
 */
UCLASS(Blueprintable)
class MR_API UMRAbility_MonsterLand : public UMRGameplayAbility
{
	GENERATED_BODY()

public:
	UMRAbility_MonsterLand();

	UPROPERTY(EditDefaultsOnly, Category = "Land")
	TObjectPtr<UAnimMontage> LandMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Land", meta = (ClampMin = "0.1"))
	float PlayRate = 1.0f;

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
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;
};
