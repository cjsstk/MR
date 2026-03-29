// Fill out your copyright notice in the Description page of Project Settings.

#include "MVVMWidgetBase.h"

UMVVMWidgetBase::UMVVMWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMVVMWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

void UMVVMWidgetBase::NativeDestruct()
{
	UnbindAllStores();
	Super::NativeDestruct();
}

void UMVVMWidgetBase::BindStore(UStoreBase* Store)
{
	if (!ensureMsgf(Store != nullptr, TEXT("UMVVMWidgetBase::BindStore — Store가 nullptr입니다.")))
	{
		return;
	}

	// 중복 바인딩 방지
	const bool bAlreadyBound = BoundStores.ContainsByPredicate(
		[Store](const FStoreBinding& Binding) { return Binding.Store == Store; }
	);
	if (bAlreadyBound)
	{
		return;
	}

	FStoreBinding& Binding = BoundStores.AddDefaulted_GetRef();
	Binding.Store = Store;
	Binding.Handle = Store->Subscribe(
		FOnStoreStateChanged::FDelegate::CreateUObject(this, &UMVVMWidgetBase::HandleStoreChanged)
	);

	// 즉시 전체 리프레시 — 위젯이 항상 최신 상태로 시작하도록 보장
	OnStoreStateChanged(Store, NAME_None);
}

void UMVVMWidgetBase::UnbindStore(UStoreBase* Store)
{
	const int32 Index = BoundStores.IndexOfByPredicate(
		[Store](const FStoreBinding& Binding) { return Binding.Store == Store; }
	);

	if (Index == INDEX_NONE)
	{
		return;
	}

	FStoreBinding& Binding = BoundStores[Index];
	if (IsValid(Binding.Store))
	{
		Binding.Store->Unsubscribe(Binding.Handle);
	}

	BoundStores.RemoveAtSwap(Index);
}

void UMVVMWidgetBase::UnbindAllStores()
{
	// 역순으로 제거하여 RemoveAtSwap의 인덱스 변동을 피한다
	for (int32 i = BoundStores.Num() - 1; i >= 0; --i)
	{
		FStoreBinding& Binding = BoundStores[i];
		if (IsValid(Binding.Store))
		{
			Binding.Store->Unsubscribe(Binding.Handle);
		}
	}
	BoundStores.Empty();
}

void UMVVMWidgetBase::OnStoreStateChanged(UStoreBase* Store, FName FieldName)
{
	// 기본 구현은 비어 있음. 서브클래스에서 오버라이드.
}

void UMVVMWidgetBase::HandleStoreChanged(UStoreBase* Store, FName FieldName)
{
	OnStoreStateChanged(Store, FieldName);
}
