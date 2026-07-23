// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MRInteractionPromptWidget.generated.h"

/**
 * 인터랙션 가능 오브젝트(사체·광석·식물 등) 근처에서 표시되는 F키 프롬프트 위젯.
 * AMRPlayerCharacter::ShowGatherPrompt / HideGatherPrompt에서 Show/Hide된다.
 * BP에서 실제 텍스트/아이콘 구현.
 */
UCLASS(Abstract, Blueprintable)
class MR_API UMRInteractionPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 표시할 액션 텍스트 설정 (e.g. "박리") */
	UFUNCTION(BlueprintImplementableEvent, Category = "Prompt")
	void SetPromptText(const FText& Text);
};
