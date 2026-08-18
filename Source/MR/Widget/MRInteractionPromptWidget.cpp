#include "Widget/MRInteractionPromptWidget.h"

void UMRInteractionPromptWidget::SetInteractionInfo(TSoftObjectPtr<UTexture2D> InIcon, const FText& InName)
{
	BP_SetInteractionInfo(InIcon, InName);
}