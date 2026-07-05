// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRStore/MVVMWidgetBase.h"
#include "Action/ActionTypes.h"
#include "CharacterStatusWidget.generated.h"

class UProgressBar;
class UCharacterStore;
struct FAction_SetHealth;
struct FAction_SetStamina;

/**
 * UCharacterStatusWidget
 *
 * 캐릭터 HP, 스태미나 바를 표시하는 위젯.
 * CharacterStore를 구독하여 상태 변경 시 자동으로 UI를 갱신한다.
 *
 * Blueprint 설정:
 *   - HealthBar, StaminaBar 이름의 UProgressBar 위젯을 UMG에 배치해야 한다.
 */
UCLASS()
class MR_API UCharacterStatusWidget : public UMVVMWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

private:
	DECLARE_ACTION_HANDLER(HandleHealthChanged, SetHealth);
	DECLARE_ACTION_HANDLER(HandleStaminaChanged, SetStamina);

	// UMG 에디터에서 동일한 이름의 위젯과 자동 바인딩
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> StaminaBar;
};
