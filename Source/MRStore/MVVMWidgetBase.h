// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StoreBase.h"
#include "MVVMWidgetBase.generated.h"

/**
 * UMVVMWidgetBase
 *
 * MVVM 패턴에서 View 역할을 하는 위젯 기반 클래스.
 * 하나 이상의 UStoreBase를 구독하고, 상태 변경 시 OnStoreStateChanged를 통해 UI를 갱신한다.
 *
 * 사용 방법 (서브클래스에서):
 *   1. NativeConstruct에서 Super 호출 후 BindStore(GetMyStore(this)) 호출
 *   2. OnStoreStateChanged를 오버라이드하여 Store 타입에 맞게 UI 갱신
 *   3. NativeDestruct는 오버라이드 불필요 — 모든 구독이 자동 해제됨
 *
 * 여러 Store를 바인딩하는 경우:
 *   BindStore을 여러 번 호출하고, OnStoreStateChanged의 Store 파라미터로 소스를 구분한다.
 */
UCLASS(Abstract, Blueprintable)
class MRSTORE_API UMVVMWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UMVVMWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	// --- UUserWidget 수명 주기 ---

	/** 서브클래스는 여기서 Super 호출 후 BindStore를 호출한다. */
	virtual void NativeConstruct() override;

	/** 모든 Store 구독을 자동 해제한다. 서브클래스에서 별도 해제 불필요. */
	virtual void NativeDestruct() override;

	// --- Store 바인딩 ---

	/**
	 * 특정 Store에 이 위젯을 바인딩한다.
	 * 바인딩 즉시 OnStoreStateChanged(Store, NAME_None)을 호출하여 초기 UI를 동기화한다.
	 * 동일 Store에 중복 바인딩 시 무시된다(멱등).
	 *
	 * @param Store  바인딩할 Store. nullptr 또는 이미 바인딩된 경우 무시.
	 */
	void BindStore(UStoreBase* Store);

	/**
	 * 특정 Store의 바인딩을 해제한다.
	 * NativeDestruct에서 모든 Store에 대해 자동 호출되므로 수동 호출은 선택사항.
	 */
	void UnbindStore(UStoreBase* Store);

	/** 현재 바인딩된 모든 Store를 해제한다. NativeDestruct에서 자동 호출. */
	void UnbindAllStores();

	/**
	 * Store 상태 변경 시 호출되는 콜백. 서브클래스에서 오버라이드하여 UI를 갱신한다.
	 * BindStore 호출 시 NAME_None으로 즉시 한 번 호출되어 초기 상태를 설정한다.
	 *
	 * @param Store      변경된 Store (Cast하여 구체적인 상태 접근)
	 * @param FieldName  변경된 필드명, NAME_None이면 전체 리프레시
	 */
	virtual void OnStoreStateChanged(UStoreBase* Store, FName FieldName);

private:
	void HandleStoreChanged(UStoreBase* Store, FName FieldName);

	struct FStoreBinding
	{
		TObjectPtr<UStoreBase> Store;
		FDelegateHandle Handle;
	};

	TArray<FStoreBinding> BoundStores;
};
