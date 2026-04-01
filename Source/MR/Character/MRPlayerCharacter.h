// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRBaseCharacter.h"
#include "MREnum.h"
#include "InputActionValue.h"
#include "MRPlayerCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class UBlendSpace;

/**
 * 플레이어가 조작하는 캐릭터.
 *
 * 이동 흐름:
 *  1. Enhanced Input (MoveAction) Triggered → OnMoveInputTriggered
 *  2. AddMovementInput 호출 + WalkAbility 활성화
 *  3. WalkAbility가 "Character.State.Moving" 태그 관리 + 무기별 BlendSpace 로드/적용
 *  4. Completed → OnMoveInputCompleted → WalkAbility Cancel
 *
 * 애니메이션 연동:
 *  - AnimBP에서 GetLocomotionBlendSpace()로 현재 BlendSpace를 읽는다.
 *  - AnimBP에서 ASC의 "Character.State.Moving" 태그로 이동/대기 전환을 한다.
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

	// ─── 애니메이션 ────────────────────────────────────────────────────────

	/** Walk 어빌리티가 로드 완료 후 설정하는 LocomotionBlendSpace. AnimBP에서 읽는다. */
	UFUNCTION(BlueprintPure, Category = "Animation")
	UBlendSpace* GetLocomotionBlendSpace() const { return CurrentLocomotionBS; }

	/** WalkAbility 전용 - 로드 완료된 BlendSpace를 캐싱한다. */
	void SetLocomotionBlendSpace(UBlendSpace* BlendSpace);

protected:
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// ─── 입력 에셋 (Blueprint에서 설정) ────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

private:
	void OnMoveInputTriggered(const FInputActionValue& Value);
	void OnMoveInputCompleted(const FInputActionValue& Value);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	EMRWeaponType CurrentWeaponType = EMRWeaponType::OneHandedSword;

	UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBlendSpace> CurrentLocomotionBS;

	/** WalkAbility 스펙 핸들 - 활성화/취소에 사용 */
	FGameplayAbilitySpecHandle WalkAbilityHandle;
};
