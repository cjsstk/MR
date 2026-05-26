// Fill out your copyright notice in the Description page of Project Settings.

#include "MREffect_MonsterStats.h"
#include "MRAttributeSetBase.h"
#include "MRGameplayTags.h"

UMREffect_MonsterStats::UMREffect_MonsterStats()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	auto AddOverrideModifier = [this](FGameplayAttribute Attribute, FGameplayTag CallerTag)
	{
		FSetByCallerFloat SBC;
		SBC.DataTag = CallerTag;

		FGameplayModifierInfo Modifier;
		Modifier.Attribute = Attribute;
		Modifier.ModifierOp = EGameplayModOp::Override;
		Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SBC);
		Modifiers.Add(Modifier);
	};

	// MaxHealth와 Health를 동일한 CallerTag로 설정해 스폰 시 체력이 가득 찬 상태로 시작
	AddOverrideModifier(UMRAttributeSetBase::GetMaxHealthAttribute(),   MRGameplayTags::SetByCaller_MaxHealth);
	AddOverrideModifier(UMRAttributeSetBase::GetHealthAttribute(),      MRGameplayTags::SetByCaller_MaxHealth);
	AddOverrideModifier(UMRAttributeSetBase::GetAttackPowerAttribute(), MRGameplayTags::SetByCaller_AttackPower);
	AddOverrideModifier(UMRAttributeSetBase::GetDefensePowerAttribute(),MRGameplayTags::SetByCaller_DefensePower);
}
