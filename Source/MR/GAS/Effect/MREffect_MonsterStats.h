// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "MREffect_MonsterStats.generated.h"

/**
 * 몬스터 스폰 시 DataTable 기반 스탯을 AttributeSet에 덮어쓰는 GameplayEffect.
 * - Instant / Override 방식으로 MaxHealth, Health, AttackPower, DefensePower를 설정
 * - 각 수치는 SetByCaller로 주입 (AMRMonster::InitializeMonsterStats 참조)
 */
UCLASS()
class MR_API UMREffect_MonsterStats : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UMREffect_MonsterStats();
};
