// Fill out your copyright notice in the Description page of Project Settings.

#include "MRBTService_FaceTarget.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "MRAIController.h"

UMRBTService_FaceTarget::UMRBTService_FaceTarget()
{
	NodeName = TEXT("Face Target");
	Interval = 0.05f;
	RandomDeviation = 0.01f;
}

void UMRBTService_FaceTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return;
	}

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn)
	{
		return;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return;
	}

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(AMRAIController::BBKey_TargetActor));
	if (!Target)
	{
		return;
	}

	// 타겟 방향의 Yaw만 보간. bUseControllerRotationYaw=true(ACharacter 기본값)이면
	// SetActorRotation은 매 틱 컨트롤러 회전으로 덮어써지므로 SetControlRotation을 사용한다.
	const FVector ToTarget = Target->GetActorLocation() - Pawn->GetActorLocation();
	const FRotator ControlRot = AIController->GetControlRotation();
	const FRotator CurrentYaw = FRotator(0.f, ControlRot.Yaw, 0.f);
	const FRotator TargetYaw = FRotator(0.f, ToTarget.Rotation().Yaw, 0.f);
	const FRotator Interped = FMath::RInterpTo(CurrentYaw, TargetYaw, DeltaSeconds, InterpSpeed);

	// Pitch/Roll은 컨트롤러 현재 값을 유지
	AIController->SetControlRotation(FRotator(ControlRot.Pitch, Interped.Yaw, ControlRot.Roll));
}
