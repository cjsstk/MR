// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UCMSSubsystem;
class UGameResourceSubsystem;
class UHUDStore;
class UCharacterStore;

UCMSSubsystem* GetCMS(const UObject* InObject);
UGameResourceSubsystem* GetGameResource(const UObject* InObject);

/** HUDStore 매니저를 반환한다. GameInstance 생성 이후 유효. */
UHUDStore* GetHUDStore(const UObject* InObject);

/** CharacterStore를 반환한다. GetHUDStore(this)->GetCharacterStore()의 단축 함수. */
UCharacterStore* GetCharacterStore(const UObject* InObject);