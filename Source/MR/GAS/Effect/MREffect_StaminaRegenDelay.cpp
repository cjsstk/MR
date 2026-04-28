// Fill out your copyright notice in the Description page of Project Settings.

#include "MREffect_StaminaRegenDelay.h"
#include "MRGameplayTags.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

UMREffect_StaminaRegenDelay::UMREffect_StaminaRegenDelay()
{
	// 기본값: 3초. BP 서브클래스에서 Duration 변경 가능
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FScalableFloat(3.f);

	// 스태미너 감소마다 재적용되므로 중복 인스턴스 방지 및 Duration 갱신
	StackingType = EGameplayEffectStackingType::AggregateByTarget;
	StackLimitCount = 1;
	StackDurationRefreshPolicy = EGameplayEffectStackingDurationPolicy::RefreshOnSuccessfulApplication;
	StackExpirationPolicy = EGameplayEffectStackingExpirationPolicy::ClearEntireStack;

	// 지속 중 RegenBlocked 태그 부여 → GE_StaminaRegen 억제
	UTargetTagsGameplayEffectComponent* TagsComp = CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>(TEXT("GrantBlockTag"));
	GEComponents.Add(TagsComp);
	FInheritedTagContainer GrantedTags;
	GrantedTags.Added.AddTag(MRGameplayTags::State_Stamina_RegenBlocked);
	TagsComp->SetAndApplyTargetTagChanges(GrantedTags);
}
