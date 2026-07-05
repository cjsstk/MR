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
 * Store를 구독(Subscribe)하고, 관심 있는 페이로드 타입에 맞는 자신의 핸들러 함수를 바인딩한다.
 *
 * 사용 방법 (서브클래스에서):
 *   1. NativeConstruct에서 Super 호출 후 Subscribe(Store, &UMyWidget::HandleHealthChanged) 호출
 *   2. HandleHealthChanged(const FAction_SetHealth& Action) 처럼 페이로드 타입에 맞는 핸들러를 구현
 *   3. 바인딩 직후 필요하면 핸들러를 한 번 직접 호출해 초기 상태를 동기화한다
 *   4. NativeDestruct는 오버라이드 불필요 — Subscribe한 모든 Store 구독이 자동 해제됨
 */
UCLASS(Abstract, Blueprintable)
class MRSTORE_API UMVVMWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UMVVMWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	/** Subscribe한 모든 Store 구독을 자동 해제한다. 서브클래스에서 별도 해제 불필요. */
	virtual void NativeDestruct() override;

	/**
	 * Store를 구독하고 이 위젯의 핸들러를 바인딩한다. Type은 Store가 정의한 액션 타입
	 * 값(예: EActionType::SetHealth)을 그대로 넘기면 된다 — 이 모듈은 재사용 가능해야
	 * 해서 Type의 실제 타입을 모르는 채로(auto) Store 쪽 Subscribe<Type>에 그대로 전달한다.
	 * 위젯이 파괴될 때(NativeDestruct) 자동으로 해제된다.
	 *
	 * 사용 예: Subscribe<EActionType::SetHealth>(Store, &UMyWidget::HandleHealthChanged);
	 */
	template<auto Type, typename TOwner, typename TStore, typename TPayload>
	void Subscribe(TStore* Store, void (TOwner::* Handler)(const TPayload&))
	{
		if (!Store)
		{
			return;
		}
		Store->template Subscribe<Type>(static_cast<TOwner*>(this), Handler);
		SubscribedStores.AddUnique(Store);
	}

private:
	UPROPERTY()
	TArray<TObjectPtr<UStoreBase>> SubscribedStores;
};
