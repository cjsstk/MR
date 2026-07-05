// Fill out your copyright notice in the Description page of Project Settings.

#include "MRInventoryComponent.h"
#include "CMSSubsystem.h"
#include "MRDataTable.h"
#include "MRBaseCharacter.h"
#include "MREffect_ItemHeal.h"
#include "MRGameplayTags.h"
#include "Action/Action.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Engine/GameInstance.h"

UMRInventoryComponent::UMRInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 24슬롯 전부 빈 슬롯으로 초기화
	Slots.SetNum(MaxSlots);
}

int32 UMRInventoryComponent::AddItem(FItemId ItemId, int32 Amount)
{
	if (ItemId.IsNone() || Amount <= 0)
	{
		return 0;
	}

	UCMSSubsystem* CMS = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UCMSSubsystem>() : nullptr;
	if (!CMS)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMRInventoryComponent::AddItem: CMS not found"));
		return 0;
	}

	FItemTableRow* ItemRow = CMS->GetItemRow(ItemId);
	if (!ItemRow)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMRInventoryComponent::AddItem: Item '%s' not found in CMS"), *ItemId.ToString());
		return 0;
	}

	const int32 MaxStack = ItemRow->MaxStack;
	int32 Remaining = Amount;

	// 1차: 동일 아이템이 이미 있는 슬롯에 MaxStack까지 채움
	for (int32 i = 0; i < MaxSlots && Remaining > 0; ++i)
	{
		if (Slots[i].ItemId == ItemId.Value && Slots[i].Count < MaxStack)
		{
			const int32 CanAdd = MaxStack - Slots[i].Count;
			const int32 Adding = FMath::Min(Remaining, CanAdd);
			Slots[i].Count += Adding;
			Remaining      -= Adding;
			DispatchSlotUpdate(i);
		}
	}

	// 2차: 빈 슬롯에 새로 배치
	for (int32 i = 0; i < MaxSlots && Remaining > 0; ++i)
	{
		if (Slots[i].IsEmpty())
		{
			const int32 Adding = FMath::Min(Remaining, MaxStack);
			Slots[i].ItemId = ItemId.Value;
			Slots[i].Count  = Adding;
			Remaining       -= Adding;
			DispatchSlotUpdate(i);
		}
	}

	const int32 Added = Amount - Remaining;
	UE_LOG(LogTemp, Log, TEXT("UMRInventoryComponent::AddItem: '%s' x%d (요청 %d, 추가 %d)"),
		*ItemId.ToString(), Added, Amount, Added);

	return Added;
}

int32 UMRInventoryComponent::RemoveItem(FItemId ItemId, int32 Amount)
{
	if (ItemId.IsNone() || Amount <= 0)
	{
		return 0;
	}

	int32 Remaining = Amount;

	for (int32 i = 0; i < MaxSlots && Remaining > 0; ++i)
	{
		if (Slots[i].ItemId == ItemId.Value && Slots[i].Count > 0)
		{
			const int32 Removing = FMath::Min(Remaining, Slots[i].Count);
			Slots[i].Count -= Removing;
			Remaining      -= Removing;

			if (Slots[i].Count <= 0)
			{
				// 슬롯 초기화
				Slots[i].ItemId = 0;
				Slots[i].Count  = 0;
			}

			DispatchSlotUpdate(i);
		}
	}

	const int32 Removed = Amount - Remaining;
	UE_LOG(LogTemp, Log, TEXT("UMRInventoryComponent::RemoveItem: '%s' x%d 제거"), *ItemId.ToString(), Removed);

	return Removed;
}

bool UMRInventoryComponent::UseItem(int32 SlotIndex)
{
	if (!Slots.IsValidIndex(SlotIndex) || Slots[SlotIndex].IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("UMRInventoryComponent::UseItem: 유효하지 않은 슬롯 %d"), SlotIndex);
		return false;
	}

	UCMSSubsystem* CMS = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UCMSSubsystem>() : nullptr;
	if (!CMS)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMRInventoryComponent::UseItem: CMS not found"));
		return false;
	}

	const FItemId ItemId(Slots[SlotIndex].ItemId);
	FItemTableRow* ItemRow = CMS->GetItemRow(ItemId);
	if (!ItemRow)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMRInventoryComponent::UseItem: Item '%s' not found in CMS"), *ItemId.ToString());
		return false;
	}

	if (ItemRow->ItemType != EMRItemType::Consumable)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMRInventoryComponent::UseItem: '%s'은 소비 아이템이 아닙니다"), *ItemId.ToString());
		return false;
	}

	// 오너 캐릭터 ASC를 통해 힐 GE 적용
	AMRBaseCharacter* Character = Cast<AMRBaseCharacter>(GetOwner());
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMRInventoryComponent::UseItem: 오너가 AMRBaseCharacter가 아닙니다"));
		return false;
	}

	UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMRInventoryComponent::UseItem: ASC not found"));
		return false;
	}

	// Heal GE 적용 (SetByCaller로 회복량 주입)
	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	const TSubclassOf<UGameplayEffect> HealGEClass = UMREffect_ItemHeal::StaticClass();
	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(HealGEClass, 1.f, EffectContext);
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(MRGameplayTags::SetByCaller_HealAmount, ItemRow->EffectValue);
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

		UE_LOG(LogTemp, Log, TEXT("UMRInventoryComponent::UseItem: '%s' 사용 — 회복량 %.1f"), *ItemId.ToString(), ItemRow->EffectValue);
	}

	// 수량 차감
	Slots[SlotIndex].Count -= 1;
	if (Slots[SlotIndex].Count <= 0)
	{
		Slots[SlotIndex].ItemId = 0;
		Slots[SlotIndex].Count  = 0;
	}
	DispatchSlotUpdate(SlotIndex);

	// Store에 UseInventoryItem 액션 디스패치 (위젯 등 추가 반응이 필요할 때 사용)
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UActionDispatcher* Dispatcher = GI->GetSubsystem<UActionDispatcher>())
		{
			Dispatcher->Dispatch(MakeAction<FAction_UseInventoryItem>(SlotIndex));
		}
	}

	return true;
}

int32 UMRInventoryComponent::GetItemCount(FItemId ItemId) const
{
	int32 Total = 0;
	for (const FMRInventorySlot& Slot : Slots)
	{
		if (Slot.ItemId == ItemId.Value)
		{
			Total += Slot.Count;
		}
	}
	return Total;
}

TArray<FMRInventorySlot> UMRInventoryComponent::ExtractSlots() const
{
	return Slots;
}

void UMRInventoryComponent::RestoreSlots(const TArray<FMRInventorySlot>& InSlots)
{
	// 전체 슬롯 초기화 후 복원
	Slots.SetNum(MaxSlots);
	for (FMRInventorySlot& Slot : Slots)
	{
		Slot.ItemId = 0;
		Slot.Count  = 0;
	}

	const int32 CopyCount = FMath::Min(InSlots.Num(), MaxSlots);
	for (int32 i = 0; i < CopyCount; ++i)
	{
		Slots[i] = InSlots[i];
	}

	// 복원된 슬롯 전체를 Store에 동기화
	for (int32 i = 0; i < MaxSlots; ++i)
	{
		DispatchSlotUpdate(i);
	}

	UE_LOG(LogTemp, Log, TEXT("UMRInventoryComponent::RestoreSlots: %d슬롯 복원"), CopyCount);
}

void UMRInventoryComponent::DispatchSlotUpdate(int32 SlotIndex) const
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GI)
	{
		return;
	}

	UActionDispatcher* Dispatcher = GI->GetSubsystem<UActionDispatcher>();
	if (!Dispatcher)
	{
		return;
	}

	const FMRInventorySlot& Slot = Slots[SlotIndex];
	Dispatcher->Dispatch(MakeAction<FAction_AddInventoryItem>(Slot.ItemId, Slot.Count, SlotIndex));
}
