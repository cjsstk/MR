// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRBaseAnimInstance.h"
#include "MRMonsterAnimInstance.generated.h"

/**
 * 몬스터 전용 AnimInstance.
 * 공통 Speed는 베이스에서 처리되며, 모든 몬스터가 공유하는 상태 변수를 여기에 추가한다.
 */
UCLASS()
class MR_API UMRMonsterAnimInstance : public UMRBaseAnimInstance
{
	GENERATED_BODY()

public:
	/** 이동 방향 (-180~180). 스트레이프 BlendSpace에 사용 */
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float Direction = 0.f;

	/** Character.State.Flying 태그 보유 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsFlying = false;

	/** Character.State.Dead 태그 보유 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsDead = false;

	/** Character.State.Attacking 태그 보유 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsAttacking = false;

protected:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
