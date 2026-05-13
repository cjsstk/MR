// Fill out your copyright notice in the Description page of Project Settings.

#include "MRAbility_Sprint.h"
#include "MRGameplayTags.h"
#include "MRPlayerCharacter.h"
#include "MRAttributeSetBase.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UMRAbility_Sprint::UMRAbility_Sprint()
{
	FGameplayTagContainer Tags;
	Tags.AddTag(MRGameplayTags::Ability_Sprint);
	SetAssetTags(Tags);

	// 공격 중, 록온 중에는 스프린트 활성화 불가
	ActivationBlockedTags.AddTag(MRGameplayTags::Character_State_Attacking);
	ActivationBlockedTags.AddTag(MRGameplayTags::Character_State_LockOn);
}

void UMRAbility_Sprint::ActivateAbility(
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

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	AMRPlayerCharacter* Player = GetPlayerCharacter();
	if (!ASC || !Player)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 스태미너 없으면 스프린트 불가
	const float CurrentStamina = ASC->GetNumericAttribute(UMRAttributeSetBase::GetStaminaAttribute());
	if (CurrentStamina <= 0.f)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UCharacterMovementComponent* Movement = Player->GetCharacterMovement();
	OriginalMaxWalkSpeed = Movement->MaxWalkSpeed;
	Movement->MaxWalkSpeed = SprintSpeed;

	ASC->AddLooseGameplayTag(MRGameplayTags::Character_State_Sprinting);

	// 드레인 GE 적용 (Infinite, 스태미너 소모 + RegenBlocked 태그 부여)
	if (StaminaDrainEffectClass)
	{
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);
		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(StaminaDrainEffectClass, 1.f, EffectContext);
		if (SpecHandle.IsValid())
		{
			DrainEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	// 스태미너 고갈 시 EndAbility 호출을 위한 델리게이트 등록
	ASC->GetGameplayAttributeValueChangeDelegate(UMRAttributeSetBase::GetStaminaAttribute())
		.AddUObject(this, &UMRAbility_Sprint::OnStaminaChanged);
}

void UMRAbility_Sprint::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (AMRPlayerCharacter* Player = GetPlayerCharacter())
	{
		Player->GetCharacterMovement()->MaxWalkSpeed = OriginalMaxWalkSpeed;
	}

	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		// 델리게이트 해제
		ASC->GetGameplayAttributeValueChangeDelegate(UMRAttributeSetBase::GetStaminaAttribute())
			.RemoveAll(this);

		// 드레인 GE 제거 (RegenBlocked 태그도 함께 제거됨)
		ASC->RemoveActiveGameplayEffect(DrainEffectHandle);

		ASC->RemoveLooseGameplayTag(MRGameplayTags::Character_State_Sprinting);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMRAbility_Sprint::OnStaminaChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue <= 0.f)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}
