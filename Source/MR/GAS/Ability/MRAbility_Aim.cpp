// Fill out your copyright notice in the Description page of Project Settings.

#include "MRAbility_Aim.h"
#include "MRGameplayTags.h"
#include "MRPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UMRAbility_Aim::UMRAbility_Aim()
{
	// 어빌리티 식별 태그
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(MRGameplayTags::Ability_Aim);
	SetAssetTags(AssetTags);

	// 사망 중, 공격 중, 또는 록온 중에는 조준 불가
	ActivationBlockedTags.AddTag(MRGameplayTags::Character_State_Dead);
	ActivationBlockedTags.AddTag(MRGameplayTags::Character_State_Attacking);
	ActivationBlockedTags.AddTag(MRGameplayTags::Character_State_LockOn);

	// 조준 시작 시 스프린트 중단
	CancelAbilitiesWithTag.AddTag(MRGameplayTags::Ability_Sprint);

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UMRAbility_Aim::ActivateAbility(
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

	// 조준 상태 태그 부착 — EndAbility에서 제거
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->AddLooseGameplayTag(MRGameplayTags::Character_State_Aiming);
	}

	// 조준 중에는 캐릭터가 카메라 방향을 바라보도록 회전 설정 변경
	if (AMRPlayerCharacter* Player = GetPlayerCharacter())
	{
		Player->bUseControllerRotationYaw = true;
		Player->GetCharacterMovement()->bOrientRotationToMovement = false;
	}

	// RMB 해제(OnHeavyAttackCompleted)까지 어빌리티를 유지한다.
}

void UMRAbility_Aim::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// 조준 상태 태그 제거
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->RemoveLooseGameplayTag(MRGameplayTags::Character_State_Aiming);
	}

	// 회전 설정 복원
	if (AMRPlayerCharacter* Player = GetPlayerCharacter())
	{
		Player->bUseControllerRotationYaw = false;
		Player->GetCharacterMovement()->bOrientRotationToMovement = true;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
