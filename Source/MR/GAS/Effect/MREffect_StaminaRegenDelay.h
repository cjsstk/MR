// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "MREffect_StaminaRegenDelay.generated.h"

/**
 * 스태미너 소모 종료 후 회복을 일정 시간 차단하는 GameplayEffect.
 * - 구조 (C++): HasDuration GE, State.Stamina.RegenBlocked 태그 부여
 * - 수치 (BP 서브클래스): Duration 편집
 */
UCLASS(Blueprintable)
class MR_API UMREffect_StaminaRegenDelay : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UMREffect_StaminaRegenDelay();
};
