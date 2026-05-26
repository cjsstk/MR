// Fill out your copyright notice in the Description page of Project Settings.

#include "MRBTService_DetectPlayer.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MRAIController.h"
#include "MRMonster.h"
#include "MRBaseCharacter.h"

UMRBTService_DetectPlayer::UMRBTService_DetectPlayer()
{
	NodeName = TEXT("Detect Player");
	Interval = 0.5f;
	RandomDeviation = 0.1f;
}

void UMRBTService_DetectPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AMRAIController* AIController = Cast<AMRAIController>(OwnerComp.GetAIOwner());
	AMRMonster* Monster = AIController ? AIController->GetMonster() : nullptr;
	if (!BB || !Monster || Monster->IsDead())
	{
		return;
	}

	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(Monster->GetWorld(), 0);
	if (!PlayerCharacter)
	{
		BB->ClearValue(AMRAIController::BBKey_TargetActor);
		return;
	}

	// 플레이어 사망 시 타겟 해제
	if (AMRBaseCharacter* BaseChar = Cast<AMRBaseCharacter>(PlayerCharacter))
	{
		if (BaseChar->IsDead())
		{
			BB->ClearValue(AMRAIController::BBKey_TargetActor);
			return;
		}
	}

	const float Dist = FVector::Dist(Monster->GetActorLocation(), PlayerCharacter->GetActorLocation());
	const AActor* CurrentTarget = Cast<AActor>(BB->GetValueAsObject(AMRAIController::BBKey_TargetActor));

	if (!CurrentTarget)
	{
		// 타겟 없음 → 감지 반경 내 진입 시 타겟 설정
		if (Dist <= DetectRadius)
		{
			BB->SetValueAsObject(AMRAIController::BBKey_TargetActor, PlayerCharacter);
			UE_LOG(LogTemp, Log, TEXT("[DetectPlayer] Monster(%s) detected player. Dist=%.0f"), *Monster->GetName(), Dist);
		}
	}
	else
	{
		// 타겟 있음 → 이탈 반경 밖으로 나가면 타겟 해제
		if (Dist > LoseTargetRadius)
		{
			BB->ClearValue(AMRAIController::BBKey_TargetActor);
			UE_LOG(LogTemp, Log, TEXT("[DetectPlayer] Monster(%s) lost player. Dist=%.0f"), *Monster->GetName(), Dist);
		}
	}
}
