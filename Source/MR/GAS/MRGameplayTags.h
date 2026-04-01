// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

/**
 * 프로젝트 전역 Native GameplayTag 선언.
 * 정의(UE_DEFINE_GAMEPLAY_TAG_COMMENT)는 MRGameplayTags.cpp에 있다.
 *
 * 사용 예: AbilitySystemComponent->HasMatchingGameplayTag(MRGameplayTags::Character_State_Moving)
 */
namespace MRGameplayTags
{
	// ─── 캐릭터 상태 ─────────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Moving)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Dead)

	// ─── 어빌리티 식별 ───────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Walk)
}
