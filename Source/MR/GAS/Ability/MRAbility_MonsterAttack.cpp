// Fill out your copyright notice in the Description page of Project Settings.

#include "MRAbility_MonsterAttack.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "MRGameplayTags.h"
#include "MRAttributeSetBase.h"
#include "MREffect_AttackDamage.h"

UMRAbility_MonsterAttack::UMRAbility_MonsterAttack()
{
	// 어빌리티 활성 중 Character.State.Attacking 태그 자동 부여/해제
	ActivationOwnedTags.AddTag(MRGameplayTags::Character_State_Attacking);

	DamageEffectClass = UMREffect_AttackDamage::StaticClass();
}

void UMRAbility_MonsterAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!AttackMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	HitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, MRGameplayTags::Event_Attack_Hit);
	HitEventTask->EventReceived.AddDynamic(this, &UMRAbility_MonsterAttack::OnHitEventReceived);
	HitEventTask->ReadyForActivation();

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, AttackMontage, PlayRate);

	MontageTask->OnCompleted.AddDynamic(this, &UMRAbility_MonsterAttack::OnMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UMRAbility_MonsterAttack::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UMRAbility_MonsterAttack::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UMRAbility_MonsterAttack::OnMontageCancelled);
	MontageTask->ReadyForActivation();
}

void UMRAbility_MonsterAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
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

void UMRAbility_MonsterAttack::OnHitEventReceived(FGameplayEventData Payload)
{
	AActor* HitActor = const_cast<AActor*>(Payload.Target.Get());
	if (!HitActor)
	{
		return;
	}

	IAbilitySystemInterface* TargetASCOwner = Cast<IAbilitySystemInterface>(HitActor);
	if (!TargetASCOwner || !DamageEffectClass)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC = TargetASCOwner->GetAbilitySystemComponent();
	if (!SourceASC || !TargetASC)
	{
		return;
	}

	const UMRAttributeSetBase* AttrSet = SourceASC->GetSet<UMRAttributeSetBase>();
	const float AttackPower = AttrSet ? AttrSet->GetAttackPower() : 1.f;
	const float FinalDamage = -(AttackPower * DamageMultiplier);

	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(this);

	FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);
	if (Spec.IsValid())
	{
		Spec.Data->SetSetByCallerMagnitude(MRGameplayTags::SetByCaller_Damage, FinalDamage);
		SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
	}
}

void UMRAbility_MonsterAttack::OnMontageCompleted()
{
	const FGameplayAbilitySpecHandle Handle = GetCurrentAbilitySpecHandle();
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	const FGameplayAbilityActivationInfo ActivationInfo = GetCurrentActivationInfo();
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UMRAbility_MonsterAttack::OnMontageCancelled()
{
	const FGameplayAbilitySpecHandle Handle = GetCurrentAbilitySpecHandle();
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	const FGameplayAbilityActivationInfo ActivationInfo = GetCurrentActivationInfo();
	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}
