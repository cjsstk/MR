// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRBaseCharacter.h"
#include "MREnum.h"
#include "InputActionValue.h"
#include "MRPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UBlendSpace;

/**
 * 플레이어가 조작하는 캐릭터.
 *
 * 이동 흐름:
 *  1. Enhanced Input (MoveAction) Triggered → OnMoveInputTriggered
 *  2. AddMovementInput 호출 + WalkAbility 활성화 ("Character.State.Moving" 태그 관리)
 *  3. Completed → OnMoveInputCompleted → WalkAbility Cancel
 *
 * 애니메이션 연동:
 *  - BeginPlay / SetWeaponType에서 LoadAndApplyWeaponAnims 호출
 *  - UMRPlayerAnimInstance의 IdleAnimation, LocomotionBlendSpace 변수에 직접 세팅
 */
UCLASS()
class MR_API AMRPlayerCharacter : public AMRBaseCharacter
{
	GENERATED_BODY()

public:
	AMRPlayerCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// ─── 무기 ──────────────────────────────────────────────────────────────

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	EMRWeaponType GetWeaponType() const { return CurrentWeaponType; }

	/** 무기 타입 변경. 이동 중이면 즉시 새 BlendSpace 로드를 트리거한다. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetWeaponType(EMRWeaponType NewWeaponType);

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// ─── 입력 에셋 (Blueprint에서 설정) ────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SprintAction;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

private:
	void OnMoveInputTriggered(const FInputActionValue& Value);
	void OnMoveInputCompleted(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnSprintStarted(const FInputActionValue& Value);
	void OnSprintCompleted(const FInputActionValue& Value);

	/** WeaponType에 맞는 Idle + LocomotionBS를 비동기 로드하여 AnimInstance에 적용 */
	void LoadAndApplyWeaponAnims(EMRWeaponType WeaponType);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	EMRWeaponType CurrentWeaponType = EMRWeaponType::OneHandedSword;

	/** WalkAbility 스펙 핸들 - 활성화/취소에 사용 */
	FGameplayAbilitySpecHandle WalkAbilityHandle;

	/** SprintAbility 스펙 핸들 - 활성화/취소에 사용 */
	FGameplayAbilitySpecHandle SprintAbilityHandle;
};
