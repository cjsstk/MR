// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "StoreBase.generated.h"

class UStoreBase;

/**
 * 스토어 상태 변경 시 브로드캐스트되는 델리게이트.
 *
 * @param Store       변경된 스토어 (여러 스토어를 구독하는 위젯에서 소스 식별용)
 * @param FieldName   변경된 필드명. NAME_None이면 전체 상태가 바뀐 것으로 처리(전체 리프레시)
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnStoreStateChanged, UStoreBase*, FName);

/**
 * UStoreBase
 *
 * 실제 데이터를 보유하는 개별 콘텐츠 Store의 기반 클래스.
 * UHUDStore 같은 Store 매니저가 이 클래스를 상속한 Store들을 생성하고 소유한다.
 * 위젯은 UMVVMWidgetBase::BindStore()를 통해 이 Store를 구독한다.
 *
 * 사용 방법:
 *   1. 이 클래스를 상속하여 구체적인 Store를 구현한다.
 *   2. 상태를 변경하는 Dispatch* 메서드를 추가하고, 변경 후 NotifyStateChanged()를 호출한다.
 *   3. 필드명은 각 Store 내 namespace 상수로 정의한다 (예: CharacterStoreFields::Health).
 */
UCLASS(Abstract)
class MRSTORE_API UStoreBase : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * 이 스토어의 상태 변경 알림을 구독한다.
	 * 반환된 FDelegateHandle로 이후 Unsubscribe 호출 가능.
	 * 위젯의 경우 NativeDestruct에서 자동으로 모든 구독이 해제된다.
	 */
	FDelegateHandle Subscribe(FOnStoreStateChanged::FDelegate&& Delegate);

	/** 특정 핸들의 구독을 해제한다. */
	void Unsubscribe(FDelegateHandle Handle);

	/**
	 * 특정 오브젝트에 바인딩된 모든 구독을 한 번에 해제한다.
	 * UMVVMWidgetBase::NativeDestruct에서 자동으로 호출되므로 위젯은 수동 정리 불필요.
	 *
	 * @param UserObject  AddUObject로 바인딩할 때 사용한 this 포인터
	 */
	void UnsubscribeAll(const void* UserObject);

protected:
	/**
	 * Dispatch* 메서드에서 상태 변경 후 호출한다.
	 * 구독된 모든 옵저버에게 브로드캐스트한다.
	 *
	 * @param FieldName  변경된 필드명. 생략 시 NAME_None(전체 리프레시)
	 */
	void NotifyStateChanged(FName FieldName = NAME_None);

private:
	FOnStoreStateChanged OnStateChanged;
};
