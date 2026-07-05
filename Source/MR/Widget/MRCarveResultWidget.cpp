// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/MRCarveResultWidget.h"
#include "Store/InventoryStore.h"
#include "Action/Action.h"
#include "Sugar.h"

void UMRCarveResultWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UInventoryStore* Store = GetInventoryStore(this))
	{
		Subscribe<EActionType::InventoryCarveResultChanged>(Store, &UMRCarveResultWidget::HandleCarveResultChanged);
		HandleCarveResultChanged(FAction_InventoryCarveResultChanged{ Store->GetState().LastAcquiredItems });
	}
}

void UMRCarveResultWidget::HandleCarveResultChanged(const FAction_InventoryCarveResultChanged& Notification)
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
