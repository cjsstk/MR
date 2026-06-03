// Fill out your copyright notice in the Description page of Project Settings.

#include "MRAbility_BowAttack.h"
#include "MRGameplayTags.h"

UMRAbility_BowAttack::UMRAbility_BowAttack()
{
	// 어빌리티 식별 태그
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(MRGameplayTags::Ability_Attack_BowNormal);
	SetAssetTags(AssetTags);

	// 활은 근접보다 개별 화살 데미지가 낮고 스태미나 소모도 적다
	ComboMotionValues = { 0.5f, 0.6f, 0.7f, 0.9f };
	BaseStaminaCost = 10.f;
}
