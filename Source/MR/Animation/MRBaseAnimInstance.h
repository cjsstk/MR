// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MRBaseAnimInstance.generated.h"

/**
 * 플레이어/몬스터 공통 AnimInstance 베이스.
 * 이동 속도 등 공통 변수를 관리한다.
 */
UCLASS()
class MR_API UMRBaseAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	/** AnimBP State Machine 전환 조건 및 BlendSpace 입력값으로 사용 */
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float Speed = 0.f;

	/** Speed 보간 속도. 값이 클수록 실제 속도에 빠르게 반응한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	float SpeedInterpSpeed = 8.f;

	/** 공중 체공 여부 - Jump State Machine 전환 조건 */
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsFalling = false;

private:
	TWeakObjectPtr<ACharacter> OwnerCharacter;
};
