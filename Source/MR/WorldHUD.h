// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "WorldHUD.generated.h"

class UWorldHUDWidget;

/**
 * AWorldHUD
 *
 * 인게임 HUD를 관리한다. BeginPlay에서 WorldHUDWidget을 생성하고 뷰포트에 추가한다.
 * HUD 클래스는 프로젝트 세팅 또는 GameMode에서 지정한다.
 *
 * Blueprint 설정:
 *   - WorldHUDWidgetClass에 BP_WorldHUDWidget을 지정한다.
 */
UCLASS()
class MR_API AWorldHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UWorldHUDWidget* GetWorldHUDWidget() const { return WorldHUDWidget; }

protected:
	/** 뷰포트에 추가할 WorldHUDWidget의 클래스. Blueprint에서 지정. */
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UWorldHUDWidget> WorldHUDWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UWorldHUDWidget> WorldHUDWidget;
};
