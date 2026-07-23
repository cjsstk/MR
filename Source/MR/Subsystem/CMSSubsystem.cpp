// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/CMSSubsystem.h"

void UCMSSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// BP 서브클래스(BP_CMS)의 Class Defaults에 등록된 TablePaths를 동기 로드하여 캐시
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
		UE_LOG(LogTemp, Log, TEXT("UCMSSubsystem: '%s' DataTable 로드 완료 (%d rows)"),
			*Name.ToString(), Table->GetRowNames().Num());
	}
}

void UCMSSubsystem::Deinitialize()
{
	LoadedTables.Empty();

	Super::Deinitialize();
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

FGatherableTableRow* UCMSSubsystem::GetGatherableRow(int32 Type) const
{
	return GetRow<FGatherableTableRow>(TEXT("Gatherable"), FName(FString::FromInt(Type)));
}

FDropTableRow* UCMSSubsystem::GetDropTableRow(FDropTableId DropTableId) const
{
	return GetRow<FDropTableRow>(TEXT("Drop"), DropTableId.ToRowName());
}

FItemTableRow* UCMSSubsystem::GetItemRow(FItemId ItemId) const
{
	return GetRow<FItemTableRow>(TEXT("Item"), ItemId.ToRowName());
}

FDecorationTableRow* UCMSSubsystem::GetDecorationRow(FItemId ItemId) const
{
	return GetRow<FDecorationTableRow>(TEXT("Decoration"), ItemId.ToRowName());
}

FWeaponTableRow* UCMSSubsystem::GetWeaponRow(FWeaponId WeaponId) const
{
	return GetRow<FWeaponTableRow>(TEXT("Weapon"), WeaponId.ToRowName());
}

FArmorTableRow* UCMSSubsystem::GetArmorRow(FArmorId ArmorId) const
{
	return GetRow<FArmorTableRow>(TEXT("Armor"), ArmorId.ToRowName());
}

FSkillTableRow* UCMSSubsystem::GetSkillRow(FSkillId SkillId) const
{
	return GetRow<FSkillTableRow>(TEXT("Skill"), SkillId.ToRowName());
}

FRecipeTableRow* UCMSSubsystem::GetRecipeRow(FRecipeId RecipeId) const
{
	return GetRow<FRecipeTableRow>(TEXT("Recipe"), RecipeId.ToRowName());
}

FFieldTableRow* UCMSSubsystem::GetFieldRow(FFieldId FieldId) const
{
	return GetRow<FFieldTableRow>(TEXT("Field"), FieldId.ToRowName());
}
