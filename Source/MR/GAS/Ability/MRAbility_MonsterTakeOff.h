// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRGameplayAbility.h"
#include "MRAbility_MonsterTakeOff.generated.h"

class UAbilityTask_PlayMontageAndWait;

/**
 * 몬스터 이륙 어빌리티.
 * TakeOff 몬타주를 재생하고 완료 시 비행 상태로 전환한다.
 * 비행 전환: Character.State.Flying 태그 추가, CharacterMovement MOVE_Flying, BB IsFlying=true.
 */
UCLASS(Blueprintable)
class MR_API UMRAbility_MonsterTakeOff : public UMRGameplayAbility
{
	GENERATED_BODY()

public:
	UMRAbility_MonsterTakeOff();

	UPROPERTY(EditDefaultsOnly, Category = "TakeOff")
	TObjectPtr<UAnimMontage> TakeOffMontage;

	UPROPERTY(EditDefaultsOnly, Category = "TakeOff", meta = (ClampMin = "0.1"))
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
