// Fill out your copyright notice in the Description page of Project Settings.

#include "MRBTTask_ActivateAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "AIController.h"

// BT 노드 인스턴스 메모리. UE가 0초기화하므로 default initializer 불필요.
struct FMRBTTaskMemory_ActivateAbility
{
	// ExecuteTask에서 어빌리티가 한 번이라도 활성화됐는지 추적.
	// TickTask가 활성화 직후 IsActive 확인 전에 Succeeded를 반환하는 경우를 방지.
	bool bAbilityActivated;
};

UMRBTTask_ActivateAbility::UMRBTTask_ActivateAbility()
{
	NodeName = TEXT("Activate Ability");
	bNotifyTick = true;
}

EBTNodeResult::Type UMRBTTask_ActivateAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	APawn* Pawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Pawn);
	UAbilitySystemComponent* ASC = ASCInterface ? ASCInterface->GetAbilitySystemComponent() : nullptr;

	if (!ASC || !AbilityTag.IsValid())
	{
		return EBTNodeResult::Failed;
	}

	// AbilityTag와 일치하는 Spec을 찾아 직접 활성화
	FGameplayAbilitySpec* FoundSpec = nullptr;
	for (FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.Ability && Spec.Ability->GetAssetTags().HasTag(AbilityTag))
		{
			FoundSpec = &Spec;
			break;
		}
	}

	if (!FoundSpec || !ASC->TryActivateAbility(FoundSpec->Handle))
	{
		return EBTNodeResult::Failed;
	}

	if (!bWaitForCompletion)
	{
		return EBTNodeResult::Succeeded;
	}

	auto* Memory = reinterpret_cast<FMRBTTaskMemory_ActivateAbility*>(NodeMemory);
	Memory->bAbilityActivated = false;

	return EBTNodeResult::InProgress;
}

void UMRBTTask_ActivateAbility::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	APawn* Pawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Pawn);
	UAbilitySystemComponent* ASC = ASCInterface ? ASCInterface->GetAbilitySystemComponent() : nullptr;

	if (!ASC)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	auto* Memory = reinterpret_cast<FMRBTTaskMemory_ActivateAbility*>(NodeMemory);
	const bool bCurrentlyActive = IsAbilityActive(ASC);

	if (bCurrentlyActive)
	{
		Memory->bAbilityActivated = true;
	}
	else if (Memory->bAbilityActivated)
	{
		// 한 번 활성화됐다가 종료됨 → 태스크 완료
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UMRBTTask_ActivateAbility::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	APawn* Pawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Pawn);
	UAbilitySystemComponent* ASC = ASCInterface ? ASCInterface->GetAbilitySystemComponent() : nullptr;

	if (ASC && AbilityTag.IsValid())
	{
		FGameplayTagContainer TagContainer(AbilityTag);
		ASC->CancelAbilities(&TagContainer);
	}

	return EBTNodeResult::Aborted;
}

uint16 UMRBTTask_ActivateAbility::GetInstanceMemorySize() const
{
	return sizeof(FMRBTTaskMemory_ActivateAbility);
}

bool UMRBTTask_ActivateAbility::IsAbilityActive(UAbilitySystemComponent* ASC) const
{
	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.IsActive() && Spec.Ability && Spec.Ability->GetAssetTags().HasTag(AbilityTag))
		{
			return true;
		}
	}
	return false;
}
