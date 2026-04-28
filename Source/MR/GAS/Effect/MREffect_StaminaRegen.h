// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "MREffect_StaminaRegen.generated.h"

/**
 * 스태미너 회복 GameplayEffect. 캐릭터 초기화 시 한 번 적용, 이후 계속 유지.
 * - 구조 (C++): Infinite 주기 GE, Stamina Additive 모디파이어, State.Stamina.RegenBlocked 태그 있으면 억제
 * - 수치 (BP 서브클래스): Period, 모디파이어 크기 등 편집
 */
UCLASS(Blueprintable)
class MR_API UMREffect_StaminaRegen : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UMREffect_StaminaRegen();
};
