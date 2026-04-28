// Fill out your copyright notice in the Description page of Project Settings.

#include "MREffect_StaminaRegen.h"
#include "MRAttributeSetBase.h"
#include "MRGameplayTags.h"
#include "GameplayEffectComponents/TargetTagRequirementsGameplayEffectComponent.h"

UMREffect_StaminaRegen::UMREffect_StaminaRegen()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	Period = 0.1f;

	// RegenBlocked 태그가 있으면 이 GE 주기 실행 억제
	UTargetTagRequirementsGameplayEffectComponent* ReqComp = CreateDefaultSubobject<UTargetTagRequirementsGameplayEffectComponent>(TEXT("OngoingTagReqs"));
	GEComponents.Add(ReqComp);
	ReqComp->OngoingTagRequirements.IgnoreTags.AddTag(MRGameplayTags::State_Stamina_RegenBlocked);

	// 기본값: 0.1초마다 +1 = 초당 10 회복. BP 서브클래스에서 수치 변경 가능
	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UMRAttributeSetBase::GetStaminaAttribute();
	Modifier.ModifierOp = EGameplayModOp::Additive;
	Modifier.ModifierMagnitude = FScalableFloat(1.f);
	Modifiers.Add(Modifier);
}
