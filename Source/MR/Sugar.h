// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UCMSSubsystem;
class UGameResourceSubsystem;

UCMSSubsystem* GetCMS(const UObject* InObject);
UGameResourceSubsystem* GetGameResource(const UObject* InObject);