// Fill out your copyright notice in the Description page of Project Settings.

#include "MRAbility_Walk.h"
#include "MRGameplayTags.h"
#include "AbilitySystemComponent.h"

UMRAbility_Walk::UMRAbility_Walk()
{
	// 어빌리티 태그 - 이 태그로 외부에서 Cancel 가능
	FGameplayTagContainer Tags;
	Tags.AddTag(MRGameplayTags::Ability_Walk);
	SetAssetTags(Tags);
}

void UMRAbility_Walk::ActivateAbility(
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

	// 이동 상태 태그 부착 - AnimBP 및 다른 어빌리티가 이 태그를 참조한다
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (ASC)
	{
		ASC->AddLooseGameplayTag(MRGameplayTags::Character_State_Moving);
	}
}

void UMRAbility_Walk::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// 이동 상태 태그 제거
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->RemoveLooseGameplayTag(MRGameplayTags::Character_State_Moving);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
