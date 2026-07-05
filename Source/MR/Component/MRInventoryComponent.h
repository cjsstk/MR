// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MRStrongId.h"
#include "MRInventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct MR_API FMRInventorySlot
{
	GENERATED_BODY()

	// 아이템 ID (FItemTableRow의 RowName). 0이면 빈 슬롯.
	UPROPERTY(BlueprintReadOnly)
	int32 ItemId = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Count = 0;

	bool IsEmpty() const { return ItemId == 0 || Count <= 0; }
};

/**
 * 플레이어 아이템 인벤토리 컴포넌트.
 * 24슬롯, 동일 아이템 스택 합산(MaxStack 준수), 소비 아이템 사용 지원.
 * 상태 변경 시 ActionDispatcher를 통해 InventoryStore에 동기화한다.
 */
UCLASS(ClassGroup = (MR), meta = (BlueprintSpawnableComponent))
class MR_API UMRInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	static constexpr int32 MaxSlots = 24;

	UMRInventoryComponent(const FObjectInitializer& ObjectInitializer);

	/**
	 * 아이템 추가. 실제로 추가된 수량을 반환.
	 * 동일 ItemId 슬롯에 MaxStack까지 스택 합산 후, 빈 슬롯에 새로 배치.
	 * 인벤토리가 꽉 차면 나머지는 버려짐.
	 */
	int32 AddItem(FItemId ItemId, int32 Amount);

	/** 아이템 제거. 실제로 제거된 수량을 반환. */
	int32 RemoveItem(FItemId ItemId, int32 Amount);

	/**
	 * 소비 아이템 사용. 내부에서 GE 적용 + 수량 차감.
	 * 성공하면 true 반환.
	 */
	bool UseItem(int32 SlotIndex);

	/** 특정 아이템의 총 보유 수량 */
	int32 GetItemCount(FItemId ItemId) const;

	const TArray<FMRInventorySlot>& GetSlots() const { return Slots; }

	/** 레벨 이동 시 직렬화 */
	TArray<FMRInventorySlot> ExtractSlots() const;

	/** 레벨 이동 후 복원. 복원 후 각 슬롯 상태를 Store에 동기화한다. */
	void RestoreSlots(const TArray<FMRInventorySlot>& InSlots);

private:
	/** 단일 슬롯 상태를 Store에 동기화하는 내부 헬퍼. */
	void DispatchSlotUpdate(int32 SlotIndex) const;

	UPROPERTY()
	TArray<FMRInventorySlot> Slots;
};
