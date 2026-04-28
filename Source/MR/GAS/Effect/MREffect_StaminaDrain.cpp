// Fill out your copyright notice in the Description page of Project Settings.

#include "MREffect_StaminaDrain.h"
#include "MRAttributeSetBase.h"
#include "MRGameplayTags.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

UMREffect_StaminaDrain::UMREffect_StaminaDrain()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	Period = 0.1f;

	// 드레인 중 RegenBlocked 태그 부여 → GE_StaminaRegen 억제
	UTargetTagsGameplayEffectComponent* TagsComp = CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>(TEXT("GrantBlockTag"));
	GEComponents.Add(TagsComp);
	FInheritedTagContainer GrantedTags;
	GrantedTags.Added.AddTag(MRGameplayTags::State_Stamina_RegenBlocked);
	TagsComp->SetAndApplyTargetTagChanges(GrantedTags);

	// 기본값: 0.1초마다 -2 = 초당 20 소모. BP 서브클래스에서 수치 변경 가능
	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UMRAttributeSetBase::GetStaminaAttribute();
	Modifier.ModifierOp = EGameplayModOp::Additive;
	Modifier.ModifierMagnitude = FScalableFloat(-2.f);
	Modifiers.Add(Modifier);
}
