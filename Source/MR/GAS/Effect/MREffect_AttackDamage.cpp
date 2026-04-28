// Fill out your copyright notice in the Description page of Project Settings.

#include "MREffect_AttackDamage.h"
#include "MRAttributeSetBase.h"
#include "MRGameplayTags.h"

UMREffect_AttackDamage::UMREffect_AttackDamage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FSetByCallerFloat SBC;
	SBC.DataTag = MRGameplayTags::SetByCaller_Damage;

	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UMRAttributeSetBase::GetHealthAttribute();
	Modifier.ModifierOp = EGameplayModOp::Additive;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SBC);
	Modifiers.Add(Modifier);
}
