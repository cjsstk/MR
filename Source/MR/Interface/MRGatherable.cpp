// Fill out your copyright notice in the Description page of Project Settings.

#include "Interface/MRGatherable.h"
#include "MRPlayerCharacter.h"

namespace MRGatherableInteract
{
	void EnterRange(AActor* Gatherable, AActor* OtherActor)
	{
		AMRPlayerCharacter* Player = Cast<AMRPlayerCharacter>(OtherActor);
		if (!Player)
		{
			return;
		}

		// 채집 가능 상태가 아니면 프롬프트를 띄우지 않는다.
		IMRGatherable* Target = Cast<IMRGatherable>(Gatherable);
		if (!Target || !Target->CanBeGathered())
		{
			return;
		}

		FMRGatherSpec Spec;
		Target->GetGatherSpec(Spec);

		Player->ShowGatherPrompt(Spec.InteractionText);
		Player->SetGatherable(Gatherable);
	}

	void ExitRange(AActor* Gatherable, AActor* OtherActor)
	{
		AMRPlayerCharacter* Player = Cast<AMRPlayerCharacter>(OtherActor);
		if (!Player)
		{
			return;
		}

		Player->HideGatherPrompt();
		Player->ClearGatherable(Gatherable);
	}
}
