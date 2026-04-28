// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRGameplayAbility.h"
#include "MREffect_AttackStaminaCost.h"
#include "MREffect_AttackDamage.h"
#include "MRAbility_Attack.generated.h"

class UAbilityTask_PlayMontageAndWait;

/**
 * 4콤보 기본 공격 어빌리티.
 *
 * 콤보 흐름:
 *   ActivateAbility → PlayComboMontage(0) → [AnimNotify: OpenComboWindow]
 *   → 입력 재입력(BufferComboInput) → [AnimNotify: CloseComboWindow] → AdvanceCombo
 *   → PlayComboMontage(1~3) → 마지막 콤보 또는 입력 없으면 OnMontageCompleted → EndAbility
 *
 * BP 설정 필요 항목:
 *   - ComboMontages: 콤보 0~3 몽타주 배열
 *   - StaminaCostEffectClass / DamageEffectClass: GE 서브클래스
 *   - AnimMontage에 UMRAnimNotifyState_ComboWindow 추가
 */
UCLASS(Blueprintable)
class MR_API UMRAbility_Attack : public UMRGameplayAbility
{
	GENERATED_BODY()

public:
	UMRAbility_Attack();

	/** AnimNotifyState_ComboWindow의 Begin에서 호출 — 콤보 입력 수신 가능 구간 시작 */
	void OpenComboWindow();

	/** AnimNotifyState_ComboWindow의 End에서 호출 — 버퍼된 입력이 있으면 즉시 다음 콤보로 진행 */
	void CloseComboWindow();

	/** 공격 입력 재입력 시 PlayerCharacter에서 호출 — 콤보 윈도우 안에서만 버퍼링 */
	void BufferComboInput();

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

	/** 콤보별 몽타주. 인덱스 0~3 = 1~4콤보. BP에서 무기별로 설정. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Combo")
	TArray<TObjectPtr<UAnimMontage>> ComboMontages;

	/** 콤보 단계별 스태미나 소모 배율. 인덱스가 없으면 1.0 사용. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Combo")
	TArray<float> ComboStaminaCostMultipliers;

	/** 공격 시 스태미나 소모 GE. BP에서 서브클래스 지정. */
	UPROPERTY(EditDefaultsOnly, Category = "Attack|Effects")
	TSubclassOf<UMREffect_AttackStaminaCost> StaminaCostEffectClass;

	/** 기본 스태미나 소모량. ComboStaminaCostMultipliers와 곱해서 최종 비용 결정. */
	UPROPERTY(EditDefaultsOnly, Category = "Attack|Effects")
	float BaseStaminaCost = 15.f;

	/** 히트 판정 시스템 연동용 데미지 GE. BP에서 서브클래스 지정. */
	UPROPERTY(EditDefaultsOnly, Category = "Attack|Effects")
	TSubclassOf<UMREffect_AttackDamage> DamageEffectClass;

	/**
	 * 콤보 단계별 모션 값(데미지 배율). FinalDamage = AttackPower * MotionValue.
	 * 인덱스가 없으면 1.0 사용. BP에서 콤보 수에 맞게 설정.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Attack|Effects")
	TArray<float> ComboMotionValues;

	/**
	 * 히트 판정 시스템이 충돌을 감지했을 때 호출.
	 * SourceASC의 AttackPower * 현재 콤보 MotionValue를 SetByCaller로 주입해 데미지 GE 적용.
	 */
	void ApplyDamageToTarget(UAbilitySystemComponent* TargetASC);

	/**
	 * 활성화 시 취소할 어빌리티 태그. 기본값 없음.
	 * 방패 공격 서브클래스에서 Ability.ShieldMode를 추가하면 공격 시 방패 모드가 자동 해제된다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	FGameplayTagContainer CancelOnActivateTags;

private:
	int32 CurrentComboIndex = 0;
	bool bComboWindowOpen = false;
	bool bComboInputBuffered = false;

	void PlayComboMontage();
	void AdvanceCombo();
	void ResetCombo();

	/** 현재 콤보 인덱스에 해당하는 스태미나 비용을 즉시 적용 */
	void ApplyStaminaCost();

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	/** 현재 재생 중인 몽타주 태스크 참조 — 중복 재생 방지 */
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> CurrentMontageTask;
};
