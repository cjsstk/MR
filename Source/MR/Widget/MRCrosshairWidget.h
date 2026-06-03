// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "MRCrosshairWidget.generated.h"

class UAbilitySystemComponent;

/**
 * 활 조준 중 표시되는 크로스헤어 위젯.
 *
 * Character.State.Aiming 태그가 부착될 때 표시, 제거될 때 숨긴다.
 * BP에서 크로스헤어 시각 요소를 구성한다.
 */
UCLASS()
class MR_API UMRCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void OnAimingTagChanged(const FGameplayTag Tag, int32 NewCount);

	TWeakObjectPtr<UAbilitySystemComponent> BoundASC;
};
