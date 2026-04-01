// Fill out your copyright notice in the Description page of Project Settings.

#include "MRGameplayAbility.h"
#include "MRBaseCharacter.h"
#include "MRPlayerCharacter.h"

UMRGameplayAbility::UMRGameplayAbility()
{
	// 싱글 플레이어 기본 설정
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
}

AMRBaseCharacter* UMRGameplayAbility::GetBaseCharacter() const
{
	return Cast<AMRBaseCharacter>(GetAvatarActorFromActorInfo());
}

AMRPlayerCharacter* UMRGameplayAbility::GetPlayerCharacter() const
{
	return Cast<AMRPlayerCharacter>(GetAvatarActorFromActorInfo());
}
