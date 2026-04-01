// Fill out your copyright notice in the Description page of Project Settings.

#include "StoreBase.h"

FDelegateHandle UStoreBase::Subscribe(FOnStoreStateChanged::FDelegate &&Delegate)
{
	return OnStateChanged.Add(MoveTemp(Delegate));
}

void UStoreBase::Unsubscribe(FDelegateHandle Handle)
{
	OnStateChanged.Remove(Handle);
}

void UStoreBase::UnsubscribeAll(const void *UserObject)
{
	OnStateChanged.RemoveAll(UserObject);
}

void UStoreBase::NotifyStateChanged(FName FieldName)
{
	OnStateChanged.Broadcast(this, FieldName);
}
