// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "MREffect_AttackDamage.generated.h"

/**
 * 공격 히트 시 대상 Health를 즉시 감소시키는 GameplayEffect.
 * - C++: Instant, Health Additive -10 기본값
 * - BP 서브클래스에서 수치 조정 가능
 * - 히트 판정 시스템에서 대상 ASC에 직접 적용
 */
UCLASS(Blueprintable)
class MR_API UMREffect_AttackDamage : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UMREffect_AttackDamage();
};
