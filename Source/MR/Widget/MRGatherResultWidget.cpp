// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/MRGatherResultWidget.h"
#include "Store/InventoryStore.h"
#include "Action/Action.h"
#include "Sugar.h"

void UMRGatherResultWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UInventoryStore* Store = GetInventoryStore(this))
	{
		Subscribe<EActionType::InventoryGatherResultChanged>(Store, &UMRGatherResultWidget::HandleGatherResultChanged);
		HandleGatherResultChanged(FAction_InventoryGatherResultChanged{ Store->GetState().LastAcquiredItems });
	}
}

void UMRGatherResultWidget::HandleGatherResultChanged(const FAction_InventoryGatherResultChanged& Notification)
{
	if (Notification.Items.Num() > 0)
	{
		ShowAcquiredItems(Notification.Items);
	}
	else
	{
		HideResult();
	}
}
