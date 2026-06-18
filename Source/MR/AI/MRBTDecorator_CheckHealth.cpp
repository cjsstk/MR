// Fill out your copyright notice in the Description page of Project Settings.

#include "MRBTDecorator_CheckHealth.h"
#include "AIController.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Pawn.h"
#include "MRAttributeSetBase.h"

UMRBTDecorator_CheckHealth::UMRBTDecorator_CheckHealth()
{
	NodeName = TEXT("Check Health Ratio");
}

bool UMRBTDecorator_CheckHealth::CalculateRawConditionValue(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory) const
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return false;
	}

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn)
	{
		return false;
	}

	IAbilitySystemInterface* ASCOwner = Cast<IAbilitySystemInterface>(Pawn);
	if (!ASCOwner)
	{
		return false;
	}

	UAbilitySystemComponent* ASC = ASCOwner->GetAbilitySystemComponent();
	if (!ASC)
	{
		return false;
	}

	const UMRAttributeSetBase* AttrSet = ASC->GetSet<UMRAttributeSetBase>();
	if (!AttrSet)
	{
		return false;
	}

	const float MaxHealth = AttrSet->GetMaxHealth();
	if (MaxHealth <= 0.f)
	{
		return false;
	}

	const float Health = AttrSet->GetHealth();
	const float HealthRatio = Health / MaxHealth;

	const bool bResult = bCheckBelow
		? (HealthRatio <= HealthThreshold)
		: (HealthRatio > HealthThreshold);

	UE_LOG(LogTemp, Verbose, TEXT("[CheckHealth] Pawn=%s Health=%.1f/%.1f (%.2f%%) Threshold=%.2f bCheckBelow=%d Result=%d"),
		*Pawn->GetName(), Health, MaxHealth, HealthRatio * 100.f, HealthThreshold, bCheckBelow ? 1 : 0, bResult ? 1 : 0);

	return bResult;
}
