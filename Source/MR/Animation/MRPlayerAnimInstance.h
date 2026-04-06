// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRBaseAnimInstance.h"
#include "MRPlayerAnimInstance.generated.h"

class UAnimSequence;
class UBlendSpace;

/**
 * 플레이어 전용 AnimInstance.
 * 무기 교체 시 LoadAndApplyWeaponAnims(Character)에서 각 변수를 직접 세팅한다.
 *
 * AnimBP State Machine 구조:
 *   Ground ┬─ Idle State       : Sequence Player  ← IdleAnimation
 *          └─ Locomotion State : BlendSpace Player ← LocomotionBlendSpace (Speed 기반)
 *   Jump   ┬─ JumpStart        : Sequence Player  ← JumpStartAnimation  (재생 완료 → JumpLoop)
 *          ├─ JumpLoop         : Sequence Player  ← JumpLoopAnimation   (bIsFalling=false → JumpEnd)
 *          └─ JumpEnd          : Sequence Player  ← JumpEndAnimation    (재생 완료 → Ground)
 *   전환 조건: bIsFalling(true) → Jump, bIsFalling(false) → Ground
 */
UCLASS()
class MR_API UMRPlayerAnimInstance : public UMRBaseAnimInstance
{
	GENERATED_BODY()

public:
	// ─── Ground 애니메이션 ───────────────────────────────────────────────────

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Weapon")
	TObjectPtr<UAnimSequence> IdleAnimation;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Weapon")
	TObjectPtr<UBlendSpace> LocomotionBlendSpace;

	// ─── Jump 애니메이션 ─────────────────────────────────────────────────────

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Weapon")
	TObjectPtr<UAnimSequence> JumpStartAnimation;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Weapon")
	TObjectPtr<UAnimSequence> JumpLoopAnimation;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Weapon")
	TObjectPtr<UAnimSequence> JumpEndAnimation;
};
