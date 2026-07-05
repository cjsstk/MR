// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MRTravelGate.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

/**
 * 레벨에 배치해 마을↔필드 이동을 트리거하는 액터.
 *
 * - DestinationFieldId가 비어있으면 마을으로 귀환 (필드 레벨에 배치)
 * - DestinationFieldId가 설정되어 있으면 해당 필드로 이동 (마을 레벨에 배치)
 *
 * 플레이어가 TriggerVolume에 겹치면 UMRTravelSubsystem을 통해 이동을 요청한다.
 */
UCLASS()
class MR_API AMRTravelGate : public AActor
{
	GENERATED_BODY()

public:
	AMRTravelGate(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	/**
	 * 이동할 필드 ID (FFieldTableRow의 RowName).
	 * 0으로 두면 마을 귀환 게이트로 동작한다.
	 */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Travel")
	int32 DestinationFieldId = 0;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> TriggerVolume;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> GateMesh;

	/** 중복 트리거 방지 플래그 */
	bool bTravelTriggered = false;
};
