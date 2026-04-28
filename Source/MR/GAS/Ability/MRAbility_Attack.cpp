// Fill out your copyright notice in the Description page of Project Settings.

#include "MRAbility_Attack.h"
#include "MRGameplayTags.h"
#include "MRAttributeSetBase.h"
#include "MRPlayerCharacter.h"
#include "MREffect_AttackStaminaCost.h"
#include "MREffect_AttackDamage.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UMRAbility_Attack::UMRAbility_Attack()
{
	// 어빌리티 식별 태그 — 외부에서 이 태그로 Cancel 가능
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(MRGameplayTags::Ability_Attack);
	SetAssetTags(AssetTags);

	// 사망 상태에서는 활성화 불가
	ActivationBlockedTags.AddTag(MRGameplayTags::Character_State_Dead);

	// 공격 시 스프린트 어빌리티를 태그로 취소
	CancelAbilitiesWithTag.AddTag(MRGameplayTags::Ability_Sprint);

	// 콤보 단계별 스태미나 배율 기본값 (1~3콤보: 100%, 4콤보: 150%)
	ComboStaminaCostMultipliers = { 1.0f, 1.0f, 1.2f, 1.5f };
}

void UMRAbility_Attack::ActivateAbility(
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

	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		// 공격 상태 태그 부착 (EndAbility에서 제거)
		ASC->AddLooseGameplayTag(MRGameplayTags::Character_State_Attacking);

		// CancelOnActivateTags에 지정된 어빌리티 취소 (방패 공격 시 방패 모드 해제 등)
		if (CancelOnActivateTags.IsValid())
		{
			ASC->CancelAbilities(&CancelOnActivateTags);
		}
	}

	CurrentComboIndex = 0;
	PlayComboMontage();
}

void UMRAbility_Attack::EndAbility(
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

	ResetCombo();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMRAbility_Attack::PlayComboMontage()
{
	if (!ComboMontages.IsValidIndex(CurrentComboIndex) || !ComboMontages[CurrentComboIndex])
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	bComboWindowOpen = false;
	bComboInputBuffered = false;

	// 이전 태스크의 콜백을 먼저 해제한다.
	// Montage_Stop은 비동기적으로 처리되어 다음 프레임에 OnMontageEnded가 발생할 수 있다.
	// 델리게이트를 미리 해제하지 않으면 이전 태스크의 OnInterrupted가 뒤늦게 발화해
	// OnMontageCancelled → EndAbility를 호출해버린다.
	if (CurrentMontageTask)
	{
		CurrentMontageTask->OnCompleted.RemoveAll(this);
		CurrentMontageTask->OnCancelled.RemoveAll(this);
		CurrentMontageTask->OnInterrupted.RemoveAll(this);
		CurrentMontageTask = nullptr;
	}

	ApplyStaminaCost();

	CurrentMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		ComboMontages[CurrentComboIndex],
		1.0f,
		NAME_None,
		false
	);

	CurrentMontageTask->OnCompleted.AddDynamic(this, &UMRAbility_Attack::OnMontageCompleted);
	CurrentMontageTask->OnCancelled.AddDynamic(this, &UMRAbility_Attack::OnMontageCancelled);
	CurrentMontageTask->OnInterrupted.AddDynamic(this, &UMRAbility_Attack::OnMontageCancelled);
	CurrentMontageTask->ReadyForActivation();
}

void UMRAbility_Attack::OpenComboWindow()
{
	bComboWindowOpen = true;
}

void UMRAbility_Attack::CloseComboWindow()
{
	bComboWindowOpen = false;

	if (bComboInputBuffered)
	{
		AdvanceCombo();
	}
}

void UMRAbility_Attack::BufferComboInput()
{
	// 마지막 콤보가 아니면 윈도우 바깥 입력도 버퍼링한다.
	// 윈도우가 열렸을 때 즉시 AdvanceCombo가 되므로 타이밍 제어는 CloseComboWindow에서 담당.
	if (CurrentComboIndex < ComboMontages.Num() - 1)
	{
		bComboInputBuffered = true;
	}
}

void UMRAbility_Attack::AdvanceCombo()
{
	CurrentComboIndex++;
	PlayComboMontage();
}

void UMRAbility_Attack::ResetCombo()
{
	CurrentComboIndex = 0;
	bComboWindowOpen = false;
	bComboInputBuffered = false;
}

void UMRAbility_Attack::ApplyStaminaCost()
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

	const float Multiplier = ComboStaminaCostMultipliers.IsValidIndex(CurrentComboIndex)
		? ComboStaminaCostMultipliers[CurrentComboIndex]
		: 1.0f;
	const float Cost = -(BaseStaminaCost * Multiplier);

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(this);

	FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(StaminaCostEffectClass, 1.f, Context);
	if (Spec.IsValid())
	{
		Spec.Data->SetSetByCallerMagnitude(MRGameplayTags::SetByCaller_StaminaCost, Cost);
		ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
}

void UMRAbility_Attack::ApplyDamageToTarget(UAbilitySystemComponent* TargetASC)
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

	const float MotionValue = ComboMotionValues.IsValidIndex(CurrentComboIndex)
		? ComboMotionValues[CurrentComboIndex]
		: 1.0f;
	const float FinalDamage = -(AttackPower * MotionValue);

	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(this);

	FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);
	if (Spec.IsValid())
	{
		Spec.Data->SetSetByCallerMagnitude(MRGameplayTags::SetByCaller_Damage, FinalDamage);
		SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
	}
}

void UMRAbility_Attack::OnMontageCompleted()
{
	UE_LOG(LogTemp, Log, TEXT("Completed"));
	
	ResetCombo();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UMRAbility_Attack::OnMontageCancelled()
{
	ResetCombo();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
