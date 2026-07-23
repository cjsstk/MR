// Fill out your copyright notice in the Description page of Project Settings.

#include "MRAbility_Gather.h"
#include "MRPlayerCharacter.h"
#include "MRGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"

UMRAbility_Gather::UMRAbility_Gather()
{
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(MRGameplayTags::Ability_Gather);
	SetAssetTags(AssetTags);

	// 사망 상태이거나 이미 채집 중/공격 중이면 활성화 불가
	ActivationBlockedTags.AddTag(MRGameplayTags::Character_State_Dead);
	ActivationBlockedTags.AddTag(MRGameplayTags::Character_State_Gathering);
	ActivationBlockedTags.AddTag(MRGameplayTags::Character_State_Attacking);
}

void UMRAbility_Gather::SetTargetGatherable(AActor* InTarget)
{
	TargetActor = InTarget;
}

IMRGatherable* UMRAbility_Gather::GetTarget() const
{
	return Cast<IMRGatherable>(TargetActor.Get());
}

void UMRAbility_Gather::ActivateAbility(
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

	// TargetActor가 미설정인 경우(최초 활성화 등) 플레이어 캐릭터에서 직접 조회.
	// InstancingPolicy = InstancedPerActor이므로 첫 활성화 전에 GetPrimaryInstance()가
	// null을 반환해 SetTargetGatherable이 호출되지 않을 수 있다.
	if (!TargetActor.IsValid())
	{
		if (AMRPlayerCharacter* Player = GetPlayerCharacter())
		{
			TargetActor = Player->GetNearestGatherableActor();
		}
	}

	IMRGatherable* Target = GetTarget();
	if (!Target || !Target->CanBeGathered())
	{
		UE_LOG(LogTemp, Warning, TEXT("UMRAbility_Gather::ActivateAbility: 유효한 채집 대상 없음"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 대상 스펙 조회 (이동정책·몽타주·텍스트)
	FMRGatherSpec Spec;
	Target->GetGatherSpec(Spec);

	// 채집 중 상태 태그 부여 (EndAbility에서 제거)
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->AddLooseGameplayTag(MRGameplayTags::Character_State_Gathering);
	}

	// 정지형 채집은 이동 입력을 차단하고 즉시 멈춘다.
	if (Spec.MovementPolicy == EMRGatherMovementPolicy::Stationary)
	{
		if (AMRPlayerCharacter* Player = GetPlayerCharacter())
		{
			if (AController* C = Player->GetController())
			{
				C->SetIgnoreMoveInput(true);
			}
			if (UCharacterMovementComponent* Movement = Player->GetCharacterMovement())
			{
				Movement->StopMovementImmediately();
			}
			bDidBlockMovement = true;
		}
	}

	// 몽타주 선택: 대상 오버라이드 > 정책별 기본 몽타주
	UAnimMontage* Montage = Spec.MontageOverride;
	if (!Montage)
	{
		Montage = (Spec.MovementPolicy == EMRGatherMovementPolicy::Stationary)
			? StationaryMontage
			: UpperBodyMontage;
	}

	// 몽타주가 없으면 즉시 채집 처리
	if (!Montage)
	{
		OnMontageCompleted();
		return;
	}

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, Montage, 1.0f, NAME_None, false);

	MontageTask->OnCompleted.AddDynamic(this, &UMRAbility_Gather::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UMRAbility_Gather::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UMRAbility_Gather::OnMontageCancelled);
	MontageTask->ReadyForActivation();
}

void UMRAbility_Gather::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// 채집 중 상태 태그 제거
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->RemoveLooseGameplayTag(MRGameplayTags::Character_State_Gathering);
	}

	// 정지형 채집으로 막았던 이동 입력 복원 (재진입 안전을 위해 플래그로 판정)
	if (bDidBlockMovement)
	{
		if (AMRPlayerCharacter* Player = GetPlayerCharacter())
		{
			if (AController* C = Player->GetController())
			{
				C->SetIgnoreMoveInput(false);
			}
		}
		bDidBlockMovement = false;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMRAbility_Gather::OnMontageCompleted()
{
	if (IMRGatherable* Target = GetTarget())
	{
		Target->PerformGather(this);
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UMRAbility_Gather::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
