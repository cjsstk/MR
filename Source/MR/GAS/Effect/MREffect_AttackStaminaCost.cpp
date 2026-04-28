// Fill out your copyright notice in the Description page of Project Settings.

#include "MREffect_AttackStaminaCost.h"
#include "MRAttributeSetBase.h"
#include "MRGameplayTags.h"

UMREffect_AttackStaminaCost::UMREffect_AttackStaminaCost()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FSetByCallerFloat SBC;
	SBC.DataTag = MRGameplayTags::SetByCaller_StaminaCost;

	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UMRAttributeSetBase::GetStaminaAttribute();
	Modifier.ModifierOp = EGameplayModOp::Additive;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SBC);
	Modifiers.Add(Modifier);
}
