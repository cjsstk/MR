// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "StoreBase.generated.h"

class UActionDispatcher;

/**
 * UStoreBase
 *
 * 실제 데이터를 보유하는 개별 콘텐츠 Store의 기반 클래스.
 * UHUDStore 같은 Store 매니저가 이 클래스를 상속한 Store들을 생성하고 소유한다.
 *
 * 이 클래스는 프로젝트에 상관없이 재사용 가능한 최소한의 계약만 정의한다.
 * 실제 알림 구독(Subscribe/Notify)은 프로젝트 전용 라우팅 키(예: EActionType)가
 * 필요하므로 이 모듈이 아니라 프로젝트 쪽 중간 베이스(예: MR 모듈의 UMRStoreBase)에서
 * 구현한다 — UnsubscribeAll만 여기서 virtual로 열어둬서, 위젯 베이스가 구체 타입을
 * 몰라도 다형적으로 정리를 호출할 수 있게 한다.
 *
 * 사용 방법:
 *   1. 이 클래스를 (직접, 또는 프로젝트 전용 중간 베이스를 통해) 상속해 Store를 구현한다.
 *   2. 상태 변경 알림 방식은 프로젝트 쪽 중간 베이스의 계약을 따른다.
 */
UCLASS(Abstract)
class MRSTORE_API UStoreBase : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * 이 Store가 처리할 액션 핸들러를 Dispatcher에 등록한다.
	 * Action으로 상태를 변경하는 Store는 이를 오버라이드해서 구현한다.
	 */
	virtual void RegisterActionHandlers(UActionDispatcher* Dispatcher) {}

	/** RegisterActionHandlers에서 등록한 핸들러를 모두 해제한다. */
	virtual void UnregisterActionHandlers(UActionDispatcher* Dispatcher) {}

	/**
	 * 특정 오너가 이 Store를 구독해 등록한 모든 알림을 한 번에 해제한다.
	 * 알림 구독 기능을 제공하는 서브클래스(예: UMRStoreBase)가 오버라이드해서 구현한다.
	 */
	virtual void UnsubscribeAll(const void* Owner) {}
};
