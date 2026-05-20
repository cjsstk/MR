// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/DataTable.h"
#include "MRCMSSettings.generated.h"

/**
 * CMS(Content Management System) 서브시스템 설정.
 * Project Settings > MR > CMS 에서 편집할 수 있다.
 * Key: 조회에 쓸 이름 (e.g. "Monster"), Value: DataTable 에셋 경로 (Soft Reference)
 */
UCLASS(config=Game, defaultconfig, meta=(DisplayName="CMS", CategoryName="MR"))
class MR_API UMRCMSSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(config, EditAnywhere, Category="CMS")
	TMap<FName, TSoftObjectPtr<UDataTable>> TablePaths;

	static const UMRCMSSettings* Get() { return GetDefault<UMRCMSSettings>(); }
};
