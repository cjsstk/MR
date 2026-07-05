// Fill out your copyright notice in the Description page of Project Settings.

#include "MREffect_ItemHeal.h"
#include "MRAttributeSetBase.h"
#include "MRGameplayTags.h"

UMREffect_ItemHeal::UMREffect_ItemHeal()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FSetByCallerFloat SBC;
	SBC.DataTag = MRGameplayTags::SetByCaller_HealAmount;

	FGameplayModifierInfo Modifier;
	Modifier.Attribute        = UMRAttributeSetBase::GetHealthAttribute();
	Modifier.ModifierOp       = EGameplayModOp::Additive;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SBC);
	Modifiers.Add(Modifier);
}
