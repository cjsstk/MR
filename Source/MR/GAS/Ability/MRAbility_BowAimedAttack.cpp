// Fill out your copyright notice in the Description page of Project Settings.

#include "MRAbility_BowAimedAttack.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "MRGameplayTags.h"
#include "MRAttributeSetBase.h"

UMRAbility_BowAimedAttack::UMRAbility_BowAimedAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 어빌리티 식별 태그
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(MRGameplayTags::Ability_Attack_BowAimed);
	SetAssetTags(AssetTags);

	// 공격 중 스프린트 취소
	CancelAbilitiesWithTag.AddTag(MRGameplayTags::Ability_Sprint);

	// 사망 상태에서는 활성화 불가
	ActivationBlockedTags.AddTag(MRGameplayTags::Character_State_Dead);
}

void UMRAbility_BowAimedAttack::ActivateAbility(
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

	if (!AimedFireMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BowAimedAttack] AimedFireMontage is not set"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->AddLooseGameplayTag(MRGameplayTags::Character_State_Attacking);
	}

	// 스태미나 즉시 소모
	ApplyStaminaCost();

	// 발사체가 Event.Attack.Hit을 전송할 때까지 대기
	HitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, MRGameplayTags::Event_Attack_Hit, nullptr, false, true);
	HitEventTask->EventReceived.AddDynamic(this, &UMRAbility_BowAimedAttack::OnHitEventReceived);
	HitEventTask->ReadyForActivation();

	// 조준 발사 몽타주 재생
	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, AimedFireMontage, 1.0f, NAME_None, false);
	MontageTask->OnCompleted.AddDynamic(this, &UMRAbility_BowAimedAttack::OnMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UMRAbility_BowAimedAttack::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UMRAbility_BowAimedAttack::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UMRAbility_BowAimedAttack::OnMontageCancelled);
	MontageTask->ReadyForActivation();

	UE_LOG(LogTemp, Log, TEXT("[BowAimedAttack] ActivateAbility - AimedFireMontage: %s"), *AimedFireMontage->GetName());
}

void UMRAbility_BowAimedAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// 공격 상태 태그 제거
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->RemoveLooseGameplayTag(MRGameplayTags::Character_State_Attacking);
	}

	if (HitEventTask)
	{
		HitEventTask->EndTask();
		HitEventTask = nullptr;
	}

	if (MontageTask)
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMRAbility_BowAimedAttack::ApplyStaminaCost()
{
	if (!StaminaCostEffectClass)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	const float Cost = -BaseStaminaCost;

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(this);

	FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(StaminaCostEffectClass, 1.f, Context);
	if (Spec.IsValid())
	{
		Spec.Data->SetSetByCallerMagnitude(MRGameplayTags::SetByCaller_StaminaCost, Cost);
		ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
}

void UMRAbility_BowAimedAttack::ApplyDamageToTarget(UAbilitySystemComponent* TargetASC)
{
	if (!DamageEffectClass || !TargetASC)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC)
	{
		return;
	}

	const UMRAttributeSetBase* AttrSet = SourceASC->GetSet<UMRAttributeSetBase>();
	const float AttackPower = AttrSet ? AttrSet->GetAttackPower() : 1.f;
	const float FinalDamage = -(AttackPower * MotionValue);

	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(this);

	FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);
	if (Spec.IsValid())
	{
		Spec.Data->SetSetByCallerMagnitude(MRGameplayTags::SetByCaller_Damage, FinalDamage);
		SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);

		UE_LOG(LogTemp, Log, TEXT("[BowAimedAttack] ApplyDamage - AttackPower: %.1f, MotionValue: %.2f, Damage: %.1f"),
			AttackPower, MotionValue, FinalDamage);
	}
}

void UMRAbility_BowAimedAttack::OnHitEventReceived(FGameplayEventData Payload)
{
	AActor* HitActor = const_cast<AActor*>(Payload.Target.Get());
	if (!HitActor)
	{
		return;
	}

	IAbilitySystemInterface* TargetASCOwner = Cast<IAbilitySystemInterface>(HitActor);
	if (!TargetASCOwner)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[BowAimedAttack] OnHitEventReceived - Target: %s"), *HitActor->GetName());
	ApplyDamageToTarget(TargetASCOwner->GetAbilitySystemComponent());
}

void UMRAbility_BowAimedAttack::OnMontageCompleted()
{
	UE_LOG(LogTemp, Log, TEXT("[BowAimedAttack] Montage Completed"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UMRAbility_BowAimedAttack::OnMontageCancelled()
{
	UE_LOG(LogTemp, Log, TEXT("[BowAimedAttack] Montage Cancelled"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
