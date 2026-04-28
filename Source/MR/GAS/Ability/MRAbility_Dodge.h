// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRGameplayAbility.h"
#include "MRAbility_Dodge.generated.h"

class UAbilityTask_PlayMontageAndWait;
class ACharacter;

/** 록온/조준 모드 전용 4방향 회피 몽타주 세트. BP에서 TargetingDodgeMontages에 등록한다. */
USTRUCT(BlueprintType)
struct FMRDodgeMontageSet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly) TObjectPtr<UAnimMontage> Forward;
	UPROPERTY(EditDefaultsOnly) TObjectPtr<UAnimMontage> Backward;
	UPROPERTY(EditDefaultsOnly) TObjectPtr<UAnimMontage> Left;
	UPROPERTY(EditDefaultsOnly) TObjectPtr<UAnimMontage> Right;
};

/**
 * 회피(Dodge/Roll) 어빌리티.
 *
 * 흐름:
 *   입력 → 스태미나 확인 → 스태미나 즉시 소모
 *       → Dodging + Invincible 태그 부착 → DodgeMontage 재생
 *       → 완료/취소 시 태그 제거 → EndAbility
 *
 * 차단 조건: Character.State.Dead / KnockedDown / Dodging (연속 회피 불가)
 * 취소 어빌리티: Ability.Sprint (회피 시 스프린트 자동 종료)
 */
UCLASS(Blueprintable)
class MR_API UMRAbility_Dodge : public UMRGameplayAbility
{
	GENERATED_BODY()

public:
	UMRAbility_Dodge();

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

	/** BP에서 무기·상황별 회피 몽타주 지정. 루트 모션 사용 권장. */
	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	TObjectPtr<UAnimMontage> DodgeMontage;

	/**
	 * 스태미나 즉시 소모 GE. SetByCaller.StaminaCost 태그를 사용하는 Instant GE를 지정.
	 * (BP_MREffect_AttackStaminaCost 등)
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	TSubclassOf<UGameplayEffect> StaminaCostEffectClass;

	/** 회피 1회당 스태미나 소모량 */
	UPROPERTY(EditDefaultsOnly, Category = "Dodge", meta = (ClampMin = "0"))
	float StaminaCost = 20.f;

	/**
	 * 록온/조준(Character.State.Aiming) 중 사용하는 4방향 회피 몽타주.
	 * 미설정 방향은 DodgeMontage로 폴백.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	FMRDodgeMontageSet TargetingDodgeMontages;

private:
	/** Aiming 상태에서 캐릭터 로컬 입력 방향에 맞는 몽타주 반환. 입력 없으면 Backward. */
	UAnimMontage* SelectTargetingDodgeMontage(ACharacter* Character) const;
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;
};
