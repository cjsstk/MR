// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRStore/MVVMWidgetBase.h"
#include "Component/MRInventoryComponent.h"
#include "Action/ActionTypes.h"
#include "MRInventoryWidget.generated.h"

struct FAction_InventorySlotsChanged;

/**
 * 아이템 인벤토리 인벤토리 위젯. 24개 슬롯 그리드.
 * InventoryStore의 Slots 변경 시 갱신.
 * BP에서 그리드 패널, 슬롯 위젯 구현.
 */
UCLASS(Abstract, Blueprintable)
class MR_API UMRInventoryWidget : public UMVVMWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	/**
	 * BP에서 구현. 전체 슬롯 배열로 그리드를 갱신한다.
	 * 각 슬롯의 ItemId, Count를 받아 아이콘/수량 텍스트 표시.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void RefreshSlots(const TArray<FMRInventorySlot>& Slots);

	/** BP에서 구현. 인벤토리 열기/닫기 토글 애니메이션. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void SetInventoryVisible(bool bVisible);

private:
	DECLARE_ACTION_HANDLER(HandleSlotsChanged, InventorySlotsChanged);
};
