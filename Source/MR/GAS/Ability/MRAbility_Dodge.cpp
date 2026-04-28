// Fill out your copyright notice in the Description page of Project Settings.

#include "MRAbility_Dodge.h"
#include "MRGameplayTags.h"
#include "MRAttributeSetBase.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UMRAbility_Dodge::UMRAbility_Dodge()
{
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(MRGameplayTags::Ability_Dodge);
	SetAssetTags(AssetTags);

	ActivationBlockedTags.AddTag(MRGameplayTags::Character_State_Dead);
	ActivationBlockedTags.AddTag(MRGameplayTags::Character_State_KnockedDown);
	ActivationBlockedTags.AddTag(MRGameplayTags::Character_State_Dodging);

	CancelAbilitiesWithTag.AddTag(MRGameplayTags::Ability_Sprint);
}

void UMRAbility_Dodge::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 스태미나 부족 시 회피 불가
	const UMRAttributeSetBase* AttrSet = ASC->GetSet<UMRAttributeSetBase>();
	if (!AttrSet || AttrSet->GetStamina() < StaminaCost)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 스태미나 즉시 소모
	if (StaminaCostEffectClass)
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		Context.AddSourceObject(this);
		FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(StaminaCostEffectClass, 1.f, Context);
		if (Spec.IsValid())
		{
			Spec.Data->SetSetByCallerMagnitude(MRGameplayTags::SetByCaller_StaminaCost, -StaminaCost);
			ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
	}

	// 조준(Aiming) 또는 록온(LockOn) 중이면 입력 방향 기준 4방향 몽타주, 아니면 단일 몽타주
	UAnimMontage* MontageToPlay = DodgeMontage;
	if (ASC->HasMatchingGameplayTag(MRGameplayTags::Character_State_Aiming)
		|| ASC->HasMatchingGameplayTag(MRGameplayTags::Character_State_LockOn))
	{
		ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
		MontageToPlay = SelectTargetingDodgeMontage(Character);
	}

	if (!MontageToPlay)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	ASC->AddLooseGameplayTag(MRGameplayTags::Character_State_Dodging);
	ASC->AddLooseGameplayTag(MRGameplayTags::Character_State_Invincible);

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, MontageToPlay, 1.0f, NAME_None, false);

	MontageTask->OnCompleted.AddDynamic(this, &UMRAbility_Dodge::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UMRAbility_Dodge::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UMRAbility_Dodge::OnMontageCancelled);
	MontageTask->ReadyForActivation();
}

void UMRAbility_Dodge::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->RemoveLooseGameplayTag(MRGameplayTags::Character_State_Dodging);
		ASC->RemoveLooseGameplayTag(MRGameplayTags::Character_State_Invincible);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

UAnimMontage* UMRAbility_Dodge::SelectTargetingDodgeMontage(ACharacter* Character) const
{
	UAnimMontage* Fallback  = DodgeMontage.Get();
	UAnimMontage* Forward  = TargetingDodgeMontages.Forward.Get();
	UAnimMontage* Backward = TargetingDodgeMontages.Backward.Get();
	UAnimMontage* Left     = TargetingDodgeMontages.Left.Get();
	UAnimMontage* Right    = TargetingDodgeMontages.Right.Get();

	if (!Character)
	{
		return Backward ? Backward : Fallback;
	}

	const FVector WorldInput = Character->GetCharacterMovement()->GetLastInputVector();

	// 입력 없으면 후방 구르기 (백스텝)
	if (WorldInput.IsNearlyZero())
	{
		return Backward ? Backward : Fallback;
	}

	// 월드 입력을 캐릭터 로컬 좌표로 변환 (X: 전후, Y: 좌우)
	const FVector LocalInput = Character->GetActorTransform().InverseTransformVectorNoScale(WorldInput);

	UAnimMontage* Selected = nullptr;
	if (FMath::Abs(LocalInput.X) >= FMath::Abs(LocalInput.Y))
	{
		Selected = LocalInput.X >= 0.f ? Forward : Backward;
	}
	else
	{
		Selected = LocalInput.Y >= 0.f ? Right : Left;
	}

	// 해당 방향 몽타주가 미설정이면 기본 몽타주로 폴백
	return Selected ? Selected : Fallback;
}

void UMRAbility_Dodge::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UMRAbility_Dodge::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
