// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRGameplayAbility.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "MRAbility_BowAttack.generated.h"

class UAnimMontage;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

/**
 * 활 일반(비조준) 공격 어빌리티.
 *
 * 몽타주 재생 → AnimNotify_SpawnProjectile이 발사체를 스폰 →
 * 발사체가 Event.Attack.Hit를 발사자 ASC로 전송 →
 * OnHitEventReceived에서 DamageEffectClass 적용.
 *
 * BP 설정 필요 항목:
 *   - FireMontage: 발사 애니메이션 몽타주
 *   - DamageEffectClass: 히트 시 적용할 데미지 GE (UMREffect_AttackDamage 서브클래스)
 *   - StaminaCostEffectClass: 스태미나 소모 GE (UMREffect_AttackStaminaCost 서브클래스)
 */
UCLASS(Blueprintable)
class MR_API UMRAbility_BowAttack : public UMRGameplayAbility
{
	GENERATED_BODY()

public:
	UMRAbility_BowAttack();

	/** 발사 애니메이션 몽타주. BP에서 설정. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bow|Attack")
	TObjectPtr<UAnimMontage> FireMontage;

	/** 히트 시 적용할 데미지 GE. BP에서 설정. */
	UPROPERTY(EditDefaultsOnly, Category = "Bow|Attack")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/** 발사 시 소모할 스태미나 GE. BP에서 설정. */
	UPROPERTY(EditDefaultsOnly, Category = "Bow|Attack")
	TSubclassOf<UGameplayEffect> StaminaCostEffectClass;

	/** 기본 스태미나 소모량 */
	UPROPERTY(EditDefaultsOnly, Category = "Bow|Attack", meta = (ClampMin = "0.0"))
	float BaseStaminaCost = 10.f;

	/** AttackPower에 곱하는 데미지 배율. FinalDamage = AttackPower * MotionValue */
	UPROPERTY(EditDefaultsOnly, Category = "Bow|Attack", meta = (ClampMin = "0.1"))
	float MotionValue = 0.6f;

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
	/** 스태미나를 즉시 소모한다. */
	void ApplyStaminaCost();

	/** 타겟 ASC에 데미지 GE를 적용한다. */
	void ApplyDamageToTarget(UAbilitySystemComponent* TargetASC);

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
