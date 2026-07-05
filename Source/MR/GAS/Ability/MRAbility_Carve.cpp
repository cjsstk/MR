// Fill out your copyright notice in the Description page of Project Settings.

#include "MRAbility_Carve.h"
#include "MRMonster.h"
#include "MRPlayerCharacter.h"
#include "MRGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UMRAbility_Carve::UMRAbility_Carve()
{
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(MRGameplayTags::Ability_Carve);
	SetAssetTags(AssetTags);

	// 사망 상태이거나 이미 박리 중이면 활성화 불가
	ActivationBlockedTags.AddTag(MRGameplayTags::Character_State_Dead);
	ActivationBlockedTags.AddTag(MRGameplayTags::Character_State_Carving);
	ActivationBlockedTags.AddTag(MRGameplayTags::Character_State_Attacking);
}

void UMRAbility_Carve::SetTargetMonster(AMRMonster* Monster)
{
	TargetMonster = Monster;
}

void UMRAbility_Carve::ActivateAbility(
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

	// TargetMonster가 미설정인 경우(최초 활성화 등) 플레이어 캐릭터에서 직접 조회.
	// UMRGameplayAbility::InstancingPolicy = InstancedPerActor이므로 첫 활성화 전에
	// GetPrimaryInstance()가 null을 반환해 SetTargetMonster가 호출되지 않을 수 있다.
	if (!TargetMonster.IsValid())
	{
		if (AMRPlayerCharacter* Player = GetPlayerCharacter())
		{
			TargetMonster = Player->GetNearestCarvableMonster();
		}
	}

	if (!TargetMonster.IsValid() || !TargetMonster->CanBeCarved())
	{
		UE_LOG(LogTemp, Warning, TEXT("UMRAbility_Carve::ActivateAbility: 유효한 박리 대상 없음"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 박리 중 상태 태그 부여 (EndAbility에서 제거)
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->AddLooseGameplayTag(MRGameplayTags::Character_State_Carving);
	}

	// 몽타주가 없으면 즉시 박리 처리
	if (!CarveMontage)
	{
		OnMontageCompleted();
		return;
	}

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, CarveMontage, 1.0f, NAME_None, false);

	MontageTask->OnCompleted.AddDynamic(this, &UMRAbility_Carve::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UMRAbility_Carve::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UMRAbility_Carve::OnMontageCancelled);
	MontageTask->ReadyForActivation();
}

void UMRAbility_Carve::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// 박리 중 상태 태그 제거
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->RemoveLooseGameplayTag(MRGameplayTags::Character_State_Carving);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMRAbility_Carve::OnMontageCompleted()
{
	if (TargetMonster.IsValid())
	{
		TargetMonster->PerformCarve(this);
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UMRAbility_Carve::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
