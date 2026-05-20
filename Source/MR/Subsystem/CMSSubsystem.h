// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "MRDataTable.h"
#include "CMSSubsystem.generated.h"

/**
 * 게임 데이터(DataTable)를 이름 기반으로 관리하고 Row 조회 API를 제공하는 서브시스템.
 * UMRCMSSettings(Project Settings > MR > CMS)에 등록된 DataTable을 Initialize 시점에 자동 로드한다.
 */
UCLASS()
class MR_API UCMSSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;


	/** 등록된 DataTable을 반환한다. 없으면 nullptr. */
	UDataTable* GetTable(FName TableName) const;

	/**
	 * DataTable에서 Row를 조회한다.
	 * @param TableName  RegisterTable 또는 TablePaths에서 사용한 키
	 * @param RowName    DataTable의 RowName
	 * @return 해당 Row 포인터. 테이블이 없거나 Row가 없으면 nullptr.
	 */
	template <typename T>
	T* GetRow(FName TableName, FName RowName) const
	{
		UDataTable* Table = GetTable(TableName);
		if (!Table)
		{
			return nullptr;
		}
		return Table->FindRow<T>(RowName, TEXT("UCMSSubsystem::GetRow"));
	}

	/** Monster Row를 Type으로 조회한다. 없으면 nullptr. */
	FMonsterTableRow* GetMonsterRow(int32 Type) const;

	/** Drop Table Row를 DropTableId(RowName)로 조회한다. 없으면 nullptr. */
	FDropTableRow* GetDropTableRow(FName DropTableId) const;

	FItemTableRow*       GetItemRow(FName ItemId) const;
	FDecorationTableRow* GetDecorationRow(FName DecoId) const;
	FWeaponTableRow*     GetWeaponRow(FName WeaponId) const;
	FArmorTableRow*      GetArmorRow(FName ArmorId) const;
	FSkillTableRow*      GetSkillRow(FName SkillId) const;
	FRecipeTableRow*     GetRecipeRow(FName RecipeId) const;
	FFieldTableRow*      GetFieldRow(FName FieldId) const;

private:
	/** Initialize 시 TablePaths를 동기 로드해서 캐싱한다. */
	TMap<FName, TObjectPtr<UDataTable>> LoadedTables;

};
