// Fill out your copyright notice in the Description page of Project Settings.

#include "MRAbility_ShieldMode.h"
#include "MRGameplayTags.h"
#include "AbilitySystemComponent.h"

UMRAbility_ShieldMode::UMRAbility_ShieldMode()
{
	// 어빌리티 식별 태그
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(MRGameplayTags::Ability_ShieldMode);
	SetAssetTags(AssetTags);

	// 사망 중 또는 공격 중에는 방패 모드 진입 불가
	ActivationBlockedTags.AddTag(MRGameplayTags::Character_State_Dead);
	ActivationBlockedTags.AddTag(MRGameplayTags::Character_State_Attacking);

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UMRAbility_ShieldMode::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 방패 모드 태그 부착 — EndAbility에서 제거
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->AddLooseGameplayTag(MRGameplayTags::Character_State_ShieldMode);
	}

	// 어빌리티를 종료하지 않고 유지한다.
	// 재입력 시 MRPlayerCharacter에서 IsActive를 확인하고 CancelAbility를 호출한다.
}

void UMRAbility_ShieldMode::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// 방패 모드 태그 제거
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->RemoveLooseGameplayTag(MRGameplayTags::Character_State_ShieldMode);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
