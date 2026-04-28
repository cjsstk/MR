// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "MREffect_StaminaDrain.generated.h"

/**
 * 스태미너 드레인 GameplayEffect.
 * - 구조 (C++): Infinite 주기 GE, Stamina Additive 모디파이어, State.Stamina.RegenBlocked 태그 부여
 * - 수치 (BP 서브클래스): Period, 모디파이어 크기 등 편집
 */
UCLASS(Blueprintable)
class MR_API UMREffect_StaminaDrain : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UMREffect_StaminaDrain();
};
