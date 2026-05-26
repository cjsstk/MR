// Fill out your copyright notice in the Description page of Project Settings.

#include "MRAttributeSetBase.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemComponent.h"
#include "MREffect_StaminaRegenDelay.h"
#include "MRGameplayTags.h"

UMRAttributeSetBase::UMRAttributeSetBase()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitStamina(100.f);
	InitMaxStamina(100.f);
	InitAttackPower(10.f);
	InitDefensePower(5.f);
}

void UMRAttributeSetBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxStamina());
	}
}

void UMRAttributeSetBase::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));

		// 체력이 0에 도달하면 Dead 태그를 부착해 캐릭터의 HandleDeath()를 트리거한다.
		// 태그 중복 부착 방지를 위해 이미 Dead인 경우 스킵.
		if (GetHealth() <= 0.f)
		{
			UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
			if (ASC && !ASC->HasMatchingGameplayTag(MRGameplayTags::Character_State_Dead))
			{
				ASC->AddLooseGameplayTag(MRGameplayTags::Character_State_Dead);
			}
		}
	}
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.f, GetMaxStamina()));

		// 스태미너 감소 시 회복 딜레이를 자동 적용 (어빌리티가 직접 처리할 필요 없음)
		if (Data.EvaluatedData.Magnitude < 0.f)
		{
			UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
			FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
				UMREffect_StaminaRegenDelay::StaticClass(), 1.f, EffectContext);
			if (SpecHandle.IsValid())
			{
				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
}
