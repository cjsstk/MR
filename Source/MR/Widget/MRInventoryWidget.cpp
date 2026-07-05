// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/MRInventoryWidget.h"
#include "Store/InventoryStore.h"
#include "Action/Action.h"
#include "Sugar.h"

void UMRInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UInventoryStore* Store = GetInventoryStore(this))
	{
		Subscribe<EActionType::InventorySlotsChanged>(Store, &UMRInventoryWidget::HandleSlotsChanged);
		HandleSlotsChanged(FAction_InventorySlotsChanged{ Store->GetState().Slots });
	}
}

void UMRInventoryWidget::HandleSlotsChanged(const FAction_InventorySlotsChanged& Notification)
{
	RefreshSlots(Notification.Slots);
}
