// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/CMSSubsystem.h"

void UCMSSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// MonsterTable 로드 및 Type 인덱스 빌드
	if (!MonsterTable.IsNull())
	{
		if (UDataTable* Table = MonsterTable.LoadSynchronous())
		{
			LoadedTables.Add(TEXT("Monster"), Table);

		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UCMSSubsystem: MonsterTable 로드 실패 (%s)"), *MonsterTable.ToString());
		}
	}

	// TablePaths에 등록된 DataTable을 동기 로드하여 캐시
	for (auto& [Name, SoftTable] : TablePaths)
	{
		if (SoftTable.IsNull())
		{
			UE_LOG(LogTemp, Warning, TEXT("UCMSSubsystem: TablePaths[%s] 경로가 비어 있습니다."), *Name.ToString());
			continue;
		}

		UDataTable* Table = SoftTable.LoadSynchronous();
		if (!Table)
		{
			UE_LOG(LogTemp, Warning, TEXT("UCMSSubsystem: '%s' DataTable 로드 실패 (%s)"),
				*Name.ToString(), *SoftTable.ToString());
			continue;
		}

		LoadedTables.Add(Name, Table);
	}
}

void UCMSSubsystem::Deinitialize()
{
	LoadedTables.Empty();

	Super::Deinitialize();
}

void UCMSSubsystem::RegisterTable(FName TableName, UDataTable* Table)
{
	if (!Table)
	{
		UE_LOG(LogTemp, Warning, TEXT("UCMSSubsystem::RegisterTable: 유효하지 않은 DataTable (Name=%s)"), *TableName.ToString());
		return;
	}

	LoadedTables.Add(TableName, Table);
}

UDataTable* UCMSSubsystem::GetTable(FName TableName) const
{
	const TObjectPtr<UDataTable>* Found = LoadedTables.Find(TableName);
	return Found ? Found->Get() : nullptr;
}

FMonsterTableRow* UCMSSubsystem::GetMonsterRow(int32 Type) const
{
	return GetRow<FMonsterTableRow>(TEXT("Monster"), FName(FString::FromInt(Type)));
}
