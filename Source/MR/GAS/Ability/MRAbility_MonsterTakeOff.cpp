// Fill out your copyright notice in the Description page of Project Settings.

#include "MRAbility_MonsterTakeOff.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MRGameplayTags.h"
#include "MRAIController.h"

UMRAbility_MonsterTakeOff::UMRAbility_MonsterTakeOff()
{
	// 어빌리티 식별 태그
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(MRGameplayTags::Ability_Monster_TakeOff);
	SetAssetTags(AssetTags);

	// 사망 상태 또는 이미 비행 중이면 발동 차단
	ActivationBlockedTags.AddTag(MRGameplayTags::Character_State_Dead);
	ActivationBlockedTags.AddTag(MRGameplayTags::Character_State_Flying);
}

void UMRAbility_MonsterTakeOff::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!TakeOffMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MonsterTakeOff] TakeOffMontage is not set. Cancelling."));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, TakeOffMontage, PlayRate);

	MontageTask->OnCompleted.AddDynamic(this, &UMRAbility_MonsterTakeOff::OnMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UMRAbility_MonsterTakeOff::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UMRAbility_MonsterTakeOff::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UMRAbility_MonsterTakeOff::OnMontageCancelled);
	MontageTask->ReadyForActivation();
}

void UMRAbility_MonsterTakeOff::EndAbility(
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

void UMRAbility_MonsterTakeOff::OnMontageCompleted()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		// 비행 상태 태그 추가
		ASC->AddLooseGameplayTag(MRGameplayTags::Character_State_Flying);
	}

	// CharacterMovement를 비행 모드로 전환
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character)
	{
		UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
		if (Movement)
		{
			Movement->SetMovementMode(MOVE_Flying);
		}

		// Blackboard IsFlying 키 동기화
		if (AAIController* AIC = Cast<AAIController>(Character->GetController()))
		{
			if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
			{
				BB->SetValueAsBool(AMRAIController::BBKey_IsFlying, true);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[MonsterTakeOff] TakeOff completed. Flying state activated."));

	const FGameplayAbilitySpecHandle Handle = GetCurrentAbilitySpecHandle();
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	const FGameplayAbilityActivationInfo ActivationInfo = GetCurrentActivationInfo();
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UMRAbility_MonsterTakeOff::OnMontageCancelled()
{
	const FGameplayAbilitySpecHandle Handle = GetCurrentAbilitySpecHandle();
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	const FGameplayAbilityActivationInfo ActivationInfo = GetCurrentActivationInfo();
	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}
