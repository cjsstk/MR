// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldHUD.h"
#include "Widget/WorldHUDWidget.h"

void AWorldHUD::BeginPlay()
{
	Super::BeginPlay();

	if (!WorldHUDWidgetClass)
	{
		return;
	}

	APlayerController* PC = GetOwningPlayerController();
	WorldHUDWidget = CreateWidget<UWorldHUDWidget>(PC, WorldHUDWidgetClass);
	if (WorldHUDWidget)
	{
		WorldHUDWidget->AddToViewport();
	}
}
