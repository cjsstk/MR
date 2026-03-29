// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRStore/MVVMWidgetBase.h"
#include "CharacterStatusWidget.generated.h"

class UProgressBar;
class UCharacterStore;
struct FCharacterState;

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
	virtual void OnStoreStateChanged(UStoreBase* Store, FName FieldName) override;

private:
	void RefreshHealth(const FCharacterState& State);
	void RefreshStamina(const FCharacterState& State);

	// UMG 에디터에서 동일한 이름의 위젯과 자동 바인딩
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> StaminaBar;
};
