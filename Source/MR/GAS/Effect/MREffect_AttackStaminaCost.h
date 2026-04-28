// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "MREffect_AttackStaminaCost.generated.h"

/**
 * 공격 1회당 스태미나를 즉시 소모하는 GameplayEffect.
 * - C++: Instant, Stamina Additive -15 기본값
 * - BP 서브클래스에서 수치 조정 가능
 */
UCLASS(Blueprintable)
class MR_API UMREffect_AttackStaminaCost : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UMREffect_AttackStaminaCost();
};
