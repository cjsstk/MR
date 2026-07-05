// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "MREffect_ItemHeal.generated.h"

/**
 * 소비 아이템 사용 시 Health를 즉시 회복시키는 GameplayEffect.
 * - Instant, Health Additive, SetByCaller_HealAmount 크기
 * - MRInventoryComponent::UseItem에서 적용한다.
 */
UCLASS(Blueprintable)
class MR_API UMREffect_ItemHeal : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UMREffect_ItemHeal();
};
