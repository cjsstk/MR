// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 게임에서 사용되는 액션 타입.
 *
 * 열거값은 ActionList.inl 하나에서 자동 생성된다 — 새 액션 추가 시 여기가 아니라
 * ActionList.inl에 ACTION(...) 항목 하나만 추가하면 enum·페이로드·매핑이 함께 생긴다.
 * (enum과 페이로드 목록이 어긋날 수 없게 하기 위한 단일 진실 소스 구조)
 *
 * C++ 라우팅 키(템플릿 논타입 인자·TMap 키)로만 쓰이고 UPROPERTY/블루프린트에는
 * 노출되지 않으므로 UENUM 리플렉션이 필요 없다. 일반 enum class로 두어야
 * enum 정의 안에서 ActionList.inl을 #include 할 수 있다 (UHT는 .generated.h를
 * 마지막 include로 강제하므로 UENUM이면 이 X-Macro 구조가 불가능하다).
 */
enum class EActionType : uint8
{
#define ACTION(Name, ...) Name,
#include "ActionList.inl"
#undef ACTION
};

/**
 * 액션 페이로드 struct -> EActionType 매핑.
 * DECLARE_ACTION_PAYLOAD 매크로가 각 페이로드 정의부에서 템플릿 특수화로 채운다.
 * UActionDispatcher::Dispatch/BindAction이 페이로드 타입만으로 EActionType을 추론하는 데 쓰인다.
 */
template<typename TPayload>
struct TActionTypeOf;

/**
 * ActionName(EActionType의 열거값 이름)만으로 페이로드 struct(FAction_ActionName)를 선언하고
 * 대응하는 EActionType과 연결하는 매크로. 페이로드 타입 선언 + 라우팅 등록을 한 번에 한다.
 *
 * 직접 호출하기보다는 ActionList.inl에 ACTION(...) 항목을 추가하는 방식으로 쓴다 —
 * 그러면 이 매크로(페이로드/매핑)와 EActionType 열거값이 같은 목록에서 함께 생성되어
 * 서로 어긋나지 않는다. (Action.h가 ActionList.inl을 인클루드하며 이 매크로를 호출한다.)
 *
 * 이름 있는 팩토리 함수는 따로 만들 필요 없이 MakeAction<TPayload>(...)으로 바로 생성한다
 * (필드 선언 순서대로 애그리게잇 초기화됨).
 * 필드 목록에 콤마가 들어가면 매크로 인자 분리가 깨지므로, 한 줄에 필드 하나씩 세미콜론으로 끝낸다.
 */
#define DECLARE_ACTION_PAYLOAD(ActionName, ...) \
	struct FAction_##ActionName \
	{ \
		__VA_ARGS__ \
	}; \
	template<> \
	struct TActionTypeOf<FAction_##ActionName> \
	{ \
		static constexpr EActionType Value = EActionType::ActionName; \
	};

/**
 * Store::Handle*() / 위젯 핸들러를 ActionName만으로 선언하는 매크로.
 * FAction_ActionName 타입명을 매번 찾아 쓸 필요 없이, DECLARE_ACTION_PAYLOAD에 쓴
 * 이름 그대로 핸들러를 선언할 수 있다. 실제 정의(.cpp)는 그대로 FAction_ActionName을
 * 파라미터 타입으로 써서 구현한다 — 여기서 감춘 이름을 알아야 하니, 선언부 바로 위에서
 * 확인하면 된다.
 *
 * 사용 예:
 *   // .h
 *   DECLARE_ACTION_HANDLER(HandleHealthChanged, SetHealth);
 *   // .cpp
 *   void UMyWidget::HandleHealthChanged(const FAction_SetHealth& Action) { ... }
 */
#define DECLARE_ACTION_HANDLER(HandlerName, ActionName) \
	void HandlerName(const FAction_##ActionName& Action)
