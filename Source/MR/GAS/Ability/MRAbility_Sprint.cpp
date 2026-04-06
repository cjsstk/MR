// Fill out your copyright notice in the Description page of Project Settings.

#include "MRAbility_Sprint.h"
#include "MRGameplayTags.h"
#include "MRPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UMRAbility_Sprint::UMRAbility_Sprint()
{
	FGameplayTagContainer Tags;
	Tags.AddTag(MRGameplayTags::Ability_Sprint);
	SetAssetTags(Tags);
}

void UMRAbility_Sprint::ActivateAbility(
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

	AMRPlayerCharacter* Player = GetPlayerCharacter();
	if (!Player)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UCharacterMovementComponent* Movement = Player->GetCharacterMovement();
	OriginalMaxWalkSpeed = Movement->MaxWalkSpeed;
	Movement->MaxWalkSpeed = SprintSpeed;

	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->AddLooseGameplayTag(MRGameplayTags::Character_State_Sprinting);
	}
}

void UMRAbility_Sprint::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (AMRPlayerCharacter* Player = GetPlayerCharacter())
	{
		Player->GetCharacterMovement()->MaxWalkSpeed = OriginalMaxWalkSpeed;
	}

	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->RemoveLooseGameplayTag(MRGameplayTags::Character_State_Sprinting);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
