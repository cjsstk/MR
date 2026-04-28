// Fill out your copyright notice in the Description page of Project Settings.

#include "MRAnimNotifyState_ComboWindow.h"
#include "MRAbility_Attack.h"
#include "MRGameplayTags.h"
#include "AbilitySystemComponent.h"

void UMRAnimNotifyState_ComboWindow::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	if (UMRAbility_Attack* Attack = FindAttackAbility(MeshComp))
	{
		Attack->OpenComboWindow();
	}
}

void UMRAnimNotifyState_ComboWindow::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (UMRAbility_Attack* Attack = FindAttackAbility(MeshComp))
	{
		Attack->CloseComboWindow();
	}
}

FString UMRAnimNotifyState_ComboWindow::GetNotifyName_Implementation() const
{
	return TEXT("ComboWindow");
}

UMRAbility_Attack* UMRAnimNotifyState_ComboWindow::FindAttackAbility(USkeletalMeshComponent* MeshComp) const
{
	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!Owner)
	{
		return nullptr;
	}

	UAbilitySystemComponent* ASC = Owner->FindComponentByClass<UAbilitySystemComponent>();
	if (!ASC)
	{
		return nullptr;
	}

	// Ability_Attack 태그를 가진 활성 어빌리티 스펙에서 인스턴스 탐색
	FGameplayTagContainer AttackTag;
	AttackTag.AddTag(MRGameplayTags::Ability_Attack);

	TArray<FGameplayAbilitySpec*> Specs;
	ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(AttackTag, Specs);

	for (FGameplayAbilitySpec* Spec : Specs)
	{
		if (!Spec || !Spec->IsActive())
		{
			continue;
		}

		// InstancedPerActor 정책이므로 GetPrimaryInstance로 단일 인스턴스를 가져옴
		if (UMRAbility_Attack* Attack = Cast<UMRAbility_Attack>(Spec->GetPrimaryInstance()))
		{
			return Attack;
		}
	}

	return nullptr;
}
