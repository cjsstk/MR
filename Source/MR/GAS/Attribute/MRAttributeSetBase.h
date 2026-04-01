// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "MRAttributeSetBase.generated.h"

// UAttributeSet 접근자 매크로 - Getter/Setter/Initter를 한 번에 생성
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 모든 캐릭터(플레이어/몬스터)가 공유하는 기본 RPG 속성 세트.
 * 파생 AttributeSet에서 직업/몬스터 전용 속성을 추가할 수 있다.
 */
UCLASS()
class MR_API UMRAttributeSetBase : public UAttributeSet
{
	GENERATED_BODY()

public:
	UMRAttributeSetBase();

	// GE 적용 전 값 클램핑 (표시/예측값)
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	// GE 실행 후 값 클램핑 및 파생 처리 (실제 base값)
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// ─── 체력 ──────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Health")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UMRAttributeSetBase, Health)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Health")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UMRAttributeSetBase, MaxHealth)

	// ─── 스태미나 ──────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Stamina")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UMRAttributeSetBase, Stamina)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Stamina")
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UMRAttributeSetBase, MaxStamina)

	// ─── 전투 ──────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat")
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UMRAttributeSetBase, AttackPower)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat")
	FGameplayAttributeData DefensePower;
	ATTRIBUTE_ACCESSORS(UMRAttributeSetBase, DefensePower)
};
