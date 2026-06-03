// Fill out your copyright notice in the Description page of Project Settings.

#include "MRBTDecorator_IsInRange.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "MRAIController.h"

UMRBTDecorator_IsInRange::UMRBTDecorator_IsInRange()
{
	NodeName = TEXT("Is In Range");
}

bool UMRBTDecorator_IsInRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
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

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return false;
	}

	// TargetActor 키에서 타겟 액터 획득
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(AMRAIController::BBKey_TargetActor));
	if (!Target)
	{
		return false;
	}

	const float Dist = FVector::Dist(Pawn->GetActorLocation(), Target->GetActorLocation());
	const bool bInRange = Dist <= AcceptableRadius;

	UE_LOG(LogTemp, Verbose, TEXT("[IsInRange] Pawn=%s Target=%s Dist=%.0f Radius=%.0f Result=%d"),
		*Pawn->GetName(), *Target->GetName(), Dist, AcceptableRadius, bInRange ? 1 : 0);

	return bInRange;
}
