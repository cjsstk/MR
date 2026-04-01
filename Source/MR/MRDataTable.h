// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MREnum.h"
#include "MRDataTable.generated.h"

/** 몬스터 데이터 테이블 Row. RowName = 몬스터 ID (e.g. "Rathalos") */
USTRUCT(BlueprintType)
struct MR_API FMonsterTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/**
	 * 몬스터 고유 타입 번호. 다른 테이블에서 몬스터를 참조할 때 이 값을 사용한다.
	 * 중복되면 로드 시 경고가 출력된다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	int32 Type = 0;

	/** 표시 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	FText DisplayName;

	/** 최대 체력 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	float MaxHealth = 1000.f;
};

/**
 * 프로젝트 전용 DataTable 베이스 클래스.
 * 테이블별 Row 구조체는 이 파일에 FTableRowBase 서브구조체로 정의한다.
 */
UCLASS()
class MR_API UMRDataTable : public UDataTable
{
	GENERATED_BODY()
};
