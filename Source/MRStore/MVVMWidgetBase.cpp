// Fill out your copyright notice in the Description page of Project Settings.

#include "MVVMWidgetBase.h"

UMVVMWidgetBase::UMVVMWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMVVMWidgetBase::NativeDestruct()
{
	for (UStoreBase* Store : SubscribedStores)
	{
		if (IsValid(Store))
		{
			Store->UnsubscribeAll(this);
		}
	}
	SubscribedStores.Empty();

	Super::NativeDestruct();
}
