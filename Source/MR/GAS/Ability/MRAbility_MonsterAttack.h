// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRGameplayAbility.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "MRAbility_MonsterAttack.generated.h"

class UAnimMontage;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

/**
 * 몬스터 공격 어빌리티 베이스.
 * AttackMontage를 재생하고, AnimNotify(MRAnimNotify_MeleeHit)가 발송한
 * Event.Attack.Hit 이벤트를 받아 대상에게 데미지를 적용한다.
 * Blueprint 서브클래스에서 몬타주·DamageEffectClass·AssetTags를 설정하여 사용.
 */
UCLASS(Blueprintable, Abstract)
class MR_API UMRAbility_MonsterAttack : public UMRGameplayAbility
{
	GENERATED_BODY()

public:
	UMRAbility_MonsterAttack();

	/** 재생할 공격 몬타주 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	TObjectPtr<UAnimMontage> AttackMontage;

	/** 몬타주 재생 속도 배율 */
	UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (ClampMin = "0.1"))
	float PlayRate = 1.0f;

	/** 데미지 적용 GE. 기본값 UMREffect_AttackDamage. */
	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/** AttackPower에 곱하는 데미지 배율. 공격 종류별로 조정 */
	UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (ClampMin = "0.1"))
	float DamageMultiplier = 1.0f;

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

	UFUNCTION()
	void OnHitEventReceived(FGameplayEventData Payload);

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> HitEventTask;
};
