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
 *  - VelocityForward / VelocityRight: 캐릭터 로컬 좌표 속도 (-100~100). 2D BlendSpace 입력.
 *  - AimPitch / AimYaw: 에임 오프셋 입력 (-90~90). 조준 시 상하좌우 자세 블렌딩.
 */
UCLASS()
class MR_API UMRPlayerAnimInstance : public UMRBaseAnimInstance
{
	GENERATED_BODY()

protected:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	/** 록온 또는 조준 모드 여부. ABP에서 1D(Speed) vs 2D(Forward/Right) BlendSpace 전환 조건. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Targeting")
	bool bIsTargeting = false;

	/** 캐릭터 로컬 전후 속도. MaxWalkSpeed 기준 -100(후진)~100(전진). 조준 모드 2D BlendSpace 입력. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Targeting")
	float VelocityForward = 0.f;

	/** 캐릭터 로컬 좌우 속도. MaxWalkSpeed 기준 -100(좌)~100(우). 조준 모드 2D BlendSpace 입력. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Targeting")
	float VelocityRight = 0.f;

	/** 컨트롤러 Pitch. 위(-90)~아래(90). AimOffset Y축 입력. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
	float AimPitch = 0.f;

	/** 컨트롤러 Yaw - 캐릭터 Yaw. 좌(-90)~우(90). AimOffset X축 입력. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
	float AimYaw = 0.f;
};
