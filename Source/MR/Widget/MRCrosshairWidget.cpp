// Fill out your copyright notice in the Description page of Project Settings.

#include "MRCrosshairWidget.h"
#include "MRBaseCharacter.h"
#include "MRGameplayTags.h"
#include "AbilitySystemComponent.h"

void UMRCrosshairWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 기본 상태는 숨김
	SetVisibility(ESlateVisibility::Collapsed);

	APlayerController* PC = GetOwningPlayer();
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	AMRBaseCharacter* Character = Cast<AMRBaseCharacter>(Pawn);
	UAbilitySystemComponent* ASC = Character ? Character->GetAbilitySystemComponent() : nullptr;

	if (ASC)
	{
		BoundASC = ASC;
		ASC->RegisterGameplayTagEvent(MRGameplayTags::Character_State_Aiming, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &UMRCrosshairWidget::OnAimingTagChanged);

		// 이미 조준 중인 상태로 위젯이 생성된 경우 즉시 반영
		if (ASC->HasMatchingGameplayTag(MRGameplayTags::Character_State_Aiming))
		{
			SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}
}

void UMRCrosshairWidget::NativeDestruct()
{
	if (UAbilitySystemComponent* ASC = BoundASC.Get())
	{
		ASC->RegisterGameplayTagEvent(MRGameplayTags::Character_State_Aiming, EGameplayTagEventType::NewOrRemoved)
			.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UMRCrosshairWidget::OnAimingTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	SetVisibility(NewCount > 0 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}
