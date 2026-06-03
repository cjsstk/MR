// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRAbility_Attack.h"
#include "MRAbility_BowAimedAttack.generated.h"

/**
 * 활 조준(Aimed) 공격 어빌리티.
 *
 * MRAbility_Attack의 콤보 시스템을 상속한다.
 * 일반 공격보다 높은 데미지와 스태미나 소모, 별도 몽타주 세트를 사용한다.
 *
 * BP 설정 필요 항목:
 *   - ComboMontages: 조준 발사 몽타주 배열 (각 몽타주에 SpawnProjectile Notify 배치)
 *   - DamageEffectClass / StaminaCostEffectClass: GE 서브클래스
 */
UCLASS(Blueprintable)
class MR_API UMRAbility_BowAimedAttack : public UMRAbility_Attack
{
	GENERATED_BODY()

public:
	UMRAbility_BowAimedAttack();
};
