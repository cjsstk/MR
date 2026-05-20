// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UCMSSubsystem;
class UMRGameResource;
class UHUDStore;
class UCharacterStore;
class UActionDispatcher;
class UMRTravelSubsystem;

UCMSSubsystem* GetCMS(const UObject* InObject);
UMRGameResource* GetGameResource(const UObject* InObject);

/** HUDStore 매니저를 반환한다. GameInstance 생성 이후 유효. */
UHUDStore* GetHUDStore(const UObject* InObject);

/** CharacterStore를 반환한다. GetHUDStore(this)->GetCharacterStore()의 단축 함수. */
UCharacterStore* GetCharacterStore(const UObject* InObject);

/** ActionDispatcher를 반환한다. 액션 디스패치 시 사용. */
UActionDispatcher* GetActionDispatcher(const UObject* InObject);

/** TravelSubsystem을 반환한다. 레벨 이동 요청 시 사용. */
UMRTravelSubsystem* GetTravelSubsystem(const UObject* InObject);