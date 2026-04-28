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
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Sprinting)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Attacking)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Dodging)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Staggered)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_KnockedDown)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Invincible)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Dead)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_ShieldMode)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Aiming)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_LockOn)

	// ─── 게임플레이 이벤트 ───────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Attack_Hit)

	// ─── 스태미너 상태 ──────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Stamina_RegenBlocked)

	// ─── SetByCaller 크기 키 ────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Damage)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_StaminaCost)

	// ─── 어빌리티 식별 ───────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Walk)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Sprint)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Dodge)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack_Heavy)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack_Melee)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ShieldMode)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Aim)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_LockOn)
}
