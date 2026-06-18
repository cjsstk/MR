// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRGameplayAbility.h"
#include "MRAbility_MonsterSpreadFire.generated.h"

class UAbilityTask_PlayMontageAndWait;

/**
 * 몬스터 범위 화염 브레스 어빌리티.
 * 몬타주를 재생하면서 일정 간격으로 전방 콘 영역의 모든 타겟에 데미지를 적용한다.
 */
UCLASS(Blueprintable)
class MR_API UMRAbility_MonsterSpreadFire : public UMRGameplayAbility
{
	GENERATED_BODY()

public:
	UMRAbility_MonsterSpreadFire();

	UPROPERTY(EditDefaultsOnly, Category = "SpreadFire")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "SpreadFire", meta = (ClampMin = "0.1"))
	float PlayRate = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "SpreadFire")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/** AttackPower에 곱하는 틱당 데미지 배율 */
	UPROPERTY(EditDefaultsOnly, Category = "SpreadFire", meta = (ClampMin = "0.01"))
	float DamageMultiplier = 0.3f;

	/** 브레스 콘 반각 (도) */
	UPROPERTY(EditDefaultsOnly, Category = "SpreadFire", meta = (ClampMin = "1.0", ClampMax = "180.0"))
	float ConeHalfAngleDeg = 45.f;

	/** 브레스 최대 사거리 */
	UPROPERTY(EditDefaultsOnly, Category = "SpreadFire", meta = (ClampMin = "50.0"))
	float ConeRange = 800.f;

	/** 데미지 판정 간격 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "SpreadFire", meta = (ClampMin = "0.05"))
	float DamageTickInterval = 0.3f;

	/** 몬타주 시작 후 첫 데미지까지 대기 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "SpreadFire", meta = (ClampMin = "0.0"))
	float DamageStartDelay = 0.5f;

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

	void StartDamageTimer();
	void ApplyConeDamage();

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	FTimerHandle DamageDelayTimer;
	FTimerHandle DamageTickTimer;
};
