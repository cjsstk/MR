// Fill out your copyright notice in the Description page of Project Settings.

#include "MRAbility_MonsterLand.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MRGameplayTags.h"
#include "MRAIController.h"

UMRAbility_MonsterLand::UMRAbility_MonsterLand()
{
	// 어빌리티 식별 태그
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(MRGameplayTags::Ability_Monster_Land);
	SetAssetTags(AssetTags);

	// 사망 상태면 발동 차단
	ActivationBlockedTags.AddTag(MRGameplayTags::Character_State_Dead);

	// 비행 상태에서만 착지 가능
	ActivationRequiredTags.AddTag(MRGameplayTags::Character_State_Flying);
}

void UMRAbility_MonsterLand::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!LandMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MonsterLand] LandMontage is not set. Cancelling."));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, LandMontage, PlayRate);

	MontageTask->OnCompleted.AddDynamic(this, &UMRAbility_MonsterLand::OnMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UMRAbility_MonsterLand::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UMRAbility_MonsterLand::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UMRAbility_MonsterLand::OnMontageCancelled);
	MontageTask->ReadyForActivation();
}

void UMRAbility_MonsterLand::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (MontageTask)
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMRAbility_MonsterLand::OnMontageCompleted()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		// 비행 상태 태그 제거
		ASC->RemoveLooseGameplayTag(MRGameplayTags::Character_State_Flying);
	}

	// CharacterMovement를 보행 모드로 복귀
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character)
	{
		UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
		if (Movement)
		{
			Movement->SetMovementMode(MOVE_Walking);
		}

		// Blackboard IsFlying 키 동기화
		if (AAIController* AIC = Cast<AAIController>(Character->GetController()))
		{
			if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
			{
				BB->SetValueAsBool(AMRAIController::BBKey_IsFlying, false);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[MonsterLand] Landing completed. Flying state deactivated."));

	const FGameplayAbilitySpecHandle Handle = GetCurrentAbilitySpecHandle();
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	const FGameplayAbilityActivationInfo ActivationInfo = GetCurrentActivationInfo();
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UMRAbility_MonsterLand::OnMontageCancelled()
{
	const FGameplayAbilitySpecHandle Handle = GetCurrentAbilitySpecHandle();
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	const FGameplayAbilityActivationInfo ActivationInfo = GetCurrentActivationInfo();
	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}
