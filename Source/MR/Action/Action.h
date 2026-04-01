// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ActionTypes.h"
#include "Action.generated.h"

/**
 * FAction
 *
 * 액션의 타입과 페이로드를 담는 구조체.
 * 직접 생성하지 말고 Actions 네임스페이스의 팩토리 함수를 사용한다.
 *
 * 예: Dispatcher->Dispatch(Actions::SetHealth(80.f, 100.f));
 */
USTRUCT(BlueprintType)
struct FAction
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EActionType Type = EActionType::SetHealth;

	/** 주 값 (현재 HP, 현재 스태미나 등) */
	UPROPERTY(BlueprintReadOnly)
	float Value1 = 0.f;

	/** 보조 값 (최대 HP, 최대 스태미나 등) */
	UPROPERTY(BlueprintReadOnly)
	float Value2 = 0.f;
};

/** 타입 안전한 액션 생성 헬퍼. 새 액션 추가 시 여기에 팩토리 함수를 추가한다. */
namespace Actions
{
	inline FAction SetHealth(float Current, float Max)  { return { EActionType::SetHealth,  Current, Max }; }
	inline FAction SetStamina(float Current, float Max) { return { EActionType::SetStamina, Current, Max }; }
}

/** Store가 특정 액션 타입에 바인딩할 핸들러 델리게이트 */
DECLARE_DELEGATE_OneParam(FActionHandlerDelegate, const FAction&);

/**
 * UActionDispatcher
 *
 * 액션을 받아 해당 타입에 등록된 Store 핸들러들을 호출하는 중앙 디스패처.
 * GameInstanceSubsystem으로 동작하며 게임 전체에서 접근 가능하다.
 *
 * 흐름:
 *   Caller → Dispatch(Action) → Store::Handle*() → NotifyStateChanged → Widget
 *
 * 접근 방법:
 *   GetActionDispatcher(WorldContextObject)   // Sugar.h
 *   GetGameInstance()->GetSubsystem<UActionDispatcher>()
 */
UCLASS()
class MR_API UActionDispatcher : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 액션을 디스패치한다. 해당 타입에 바인딩된 모든 핸들러가 호출된다. */
	UFUNCTION(BlueprintCallable, Category = "Action")
	void Dispatch(const FAction& Action);

	/**
	 * 특정 액션 타입에 핸들러를 바인딩한다.
	 * 반환된 FDelegateHandle로 UnbindAction 호출 시 해제한다.
	 */
	FDelegateHandle BindAction(EActionType Type, FActionHandlerDelegate&& Delegate);

	/** 특정 액션 타입에서 핸들러를 해제한다. */
	void UnbindAction(EActionType Type, FDelegateHandle Handle);

private:
	struct FActionBinding
	{
		FDelegateHandle Handle;
		FActionHandlerDelegate Delegate;
	};

	TMap<EActionType, TArray<FActionBinding>> ActionBindings;
};
