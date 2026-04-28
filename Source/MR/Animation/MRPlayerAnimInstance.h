// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRBaseAnimInstance.h"
#include "MRPlayerAnimInstance.generated.h"

/**
 * 플레이어 전용 AnimInstance.
 * Ground/Jump 애니메이션은 무기별 Linked Anim Layer(ABP_OneHandedSword 등)가 담당한다.
 *
 * 록온/조준 모드 전용 파라미터:
 *  - VelocityForward / VelocityRight: 캐릭터 로컬 좌표 속도 (MaxWalkSpeed 기준 -1~1 정규화)
 *  - 2D BlendSpace의 X(좌우 stafe), Y(전후) 입력으로 직접 사용한다.
 */
UCLASS()
class MR_API UMRPlayerAnimInstance : public UMRBaseAnimInstance
{
	GENERATED_BODY()

protected:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	/** 캐릭터 로컬 전후 속도. MaxWalkSpeed 기준 -1(후진)~1(전진). 조준 모드 2D BlendSpace 입력. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Targeting")
	float VelocityForward = 0.f;

	/** 캐릭터 로컬 좌우 속도. MaxWalkSpeed 기준 -1(좌)~1(우). 조준 모드 2D BlendSpace 입력. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Targeting")
	float VelocityRight = 0.f;
};
