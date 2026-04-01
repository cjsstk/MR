// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "MRBaseCharacter.generated.h"

class UAbilitySystemComponent;
class UMRAttributeSetBase;
class UGameplayAbility;
class UGameplayEffect;

/**
 * 플레이어/몬스터가 공통으로 상속받는 GAS 기반 캐릭터 베이스.
 * ASC와 AttributeSet을 소유하며 어빌리티/이펙트 초기화를 담당한다.
 */
UCLASS(Abstract)
class MR_API AMRBaseCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AMRBaseCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UMRAttributeSetBase* GetAttributeSetBase() const { return AttributeSetBase; }

protected:
	virtual void PossessedBy(AController* NewController) override;
	virtual void BeginPlay() override;

	// 기본 어빌리티 부여 - PossessedBy에서 한 번만 호출됨
	virtual void InitializeAbilities();

	// 기본 GameplayEffect 적용 (초기 속성값 설정 등)
	virtual void InitializeEffects();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UMRAttributeSetBase> AttributeSetBase;

	// Blueprint/C++ 서브클래스에서 기본 어빌리티를 지정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

	// 초기 속성값 적용 등에 쓰이는 기본 GameplayEffect
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<TSubclassOf<UGameplayEffect>> DefaultEffects;

private:
	bool bAbilitiesInitialized = false;
};
