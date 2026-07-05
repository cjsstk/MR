// Fill out your copyright notice in the Description page of Project Settings.

#include "MRTravelGate.h"
#include "MRTravelSubsystem.h"
#include "Character/MRPlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"

AMRTravelGate::AMRTravelGate(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	GateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GateMesh"));
	SetRootComponent(GateMesh);

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetupAttachment(RootComponent);
	TriggerVolume->SetBoxExtent(FVector(100.f, 100.f, 200.f));
	TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));
}

void AMRTravelGate::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (bTravelTriggered)
	{
		return;
	}

	if (!Cast<AMRPlayerCharacter>(OtherActor))
	{
		return;
	}

	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	UMRTravelSubsystem* TravelSubsystem = GI->GetSubsystem<UMRTravelSubsystem>();
	if (!TravelSubsystem)
	{
		return;
	}

	bTravelTriggered = true;

	if (DestinationFieldId == 0)
	{
		TravelSubsystem->RequestReturnToVillage(this);
	}
	else
	{
		TravelSubsystem->RequestTravel(this, FFieldId(DestinationFieldId));
	}
}
