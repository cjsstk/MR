// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRAbility_Attack.h"
#include "MRAbility_BowAttack.generated.h"

/**
 * 활 일반(비조준) 공격 어빌리티.
 *
 * MRAbility_Attack의 콤보 시스템을 그대로 상속한다.
 * 몽타주에는 AnimNotifyState_MeleeHitbox 대신 AnimNotify_SpawnProjectile을 배치한다.
 * 프로젝타일이 충돌하면 Event.Attack.Hit을 발사자 ASC로 전송 →
 * 부모 OnHitEventReceived에서 데미지 적용.
 *
 * BP 설정 필요 항목:
 *   - ComboMontages: 활 콤보 몽타주 배열 (각 몽타주에 SpawnProjectile Notify 배치)
 *   - DamageEffectClass / StaminaCostEffectClass: GE 서브클래스
 */
UCLASS(Blueprintable)
class MR_API UMRAbility_BowAttack : public UMRAbility_Attack
{
	GENERATED_BODY()

public:
	UMRAbility_BowAttack();
};
