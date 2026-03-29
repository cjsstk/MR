// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WorldHUDWidget.generated.h"

class UCharacterStatusWidget;

/**
 * UWorldHUDWidget
 *
 * 인게임 HUD의 루트 위젯. 자식 위젯들을 담는 컨테이너 역할.
 * 각 자식 위젯이 필요한 Store를 직접 구독하므로 이 클래스는 Store를 다루지 않는다.
 *
 * Blueprint 설정:
 *   - CharacterStatusWidget 이름의 위젯을 UMG에 배치해야 한다.
 */
UCLASS()
class MR_API UWorldHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UCharacterStatusWidget* GetCharacterStatusWidget() const { return CharacterStatusWidget; }

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCharacterStatusWidget> CharacterStatusWidget;
};
