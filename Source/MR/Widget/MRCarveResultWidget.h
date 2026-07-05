// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRStore/MVVMWidgetBase.h"
#include "Subsystem/MRDropHelper.h"
#include "Action/ActionTypes.h"
#include "MRCarveResultWidget.generated.h"

struct FAction_InventoryCarveResultChanged;

/**
 * 박리 결과 팝업 위젯. InventoryStore의 CarveResult 변경 시 획득 아이템 목록을 표시하고
 * 일정 시간 후 자동으로 사라진다.
 * BP에서 애니메이션·레이아웃 구현.
 */
UCLASS(Abstract, Blueprintable)
class MR_API UMRCarveResultWidget : public UMVVMWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	/**
	 * BP에서 구현. 획득 아이템 목록을 받아 화면에 표시.
	 * DisplayName과 Count를 받아 리스트 형태로 렌더링.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "CarveResult")
	void ShowAcquiredItems(const TArray<FMRDropResult>& Items);

	/** BP에서 구현. 결과 위젯 숨김 (페이드아웃 등). */
	UFUNCTION(BlueprintImplementableEvent, Category = "CarveResult")
	void HideResult();

private:
	DECLARE_ACTION_HANDLER(HandleCarveResultChanged, InventoryCarveResultChanged);
};
