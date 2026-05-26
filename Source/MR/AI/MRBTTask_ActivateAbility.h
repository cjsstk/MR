// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"
#include "MRBTTask_ActivateAbility.generated.h"

class UAbilitySystemComponent;

/**
 * 몬스터 ASC에서 AbilityTag와 일치하는 GameplayAbility를 활성화한다.
 * bWaitForCompletion=true면 어빌리티가 종료될 때까지 InProgress를 유지한다.
 */
UCLASS()
class MR_API UMRBTTask_ActivateAbility : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UMRBTTask_ActivateAbility();

	/** 활성화할 어빌리티를 식별하는 AbilityTags 태그 (e.g. Ability.MonsterAttack) */
	UPROPERTY(EditAnywhere, Category = "Ability")
	FGameplayTag AbilityTag;

	/** true: 어빌리티가 종료될 때까지 대기 후 Succeeded 반환 */
	UPROPERTY(EditAnywhere, Category = "Ability")
	bool bWaitForCompletion = true;

public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual uint16 GetInstanceMemorySize() const override;

private:
	bool IsAbilityActive(UAbilitySystemComponent* ASC) const;
};
