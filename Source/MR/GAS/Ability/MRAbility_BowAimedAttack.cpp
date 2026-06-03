// Fill out your copyright notice in the Description page of Project Settings.

#include "MRAbility_BowAimedAttack.h"
#include "MRGameplayTags.h"

UMRAbility_BowAimedAttack::UMRAbility_BowAimedAttack()
{
	// 어빌리티 식별 태그
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(MRGameplayTags::Ability_Attack_BowAimed);
	SetAssetTags(AssetTags);

	// 조준 공격은 데미지가 높고 스태미나 소모도 크다
	ComboMotionValues = { 1.5f, 1.7f, 2.0f, 2.5f };
	BaseStaminaCost = 25.f;
}
