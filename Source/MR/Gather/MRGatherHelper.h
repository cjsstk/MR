// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MRStrongId.h"
#include "Subsystem/MRDropHelper.h"
#include "MRGatherHelper.generated.h"

class AMRPlayerCharacter;

/**
 * 채집 공용 로직 유틸리티.
 * 드롭 테이블에서 아이템을 굴려 플레이어 인벤토리에 지급하고 결과 팝업 액션을 디스패치한다.
 * 몬스터 박리·광석/식물 채집이 모두 이 함수를 재사용한다.
 */
UCLASS()
class MR_API UMRGatherHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * DropTableId로 드롭을 계산해 Player 인벤토리에 지급하고 결과 액션을 디스패치한다.
	 * 유효한 결과가 없으면 빈 FMRDropResult를 반환한다.
	 */
	static FMRDropResult GrantDropToPlayer(AMRPlayerCharacter* Player, FDropTableId DropTableId);
};
