// Fill out your copyright notice in the Description page of Project Settings.

#include "MRGameplayTags.h"

namespace MRGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Moving,      "Character.State.Moving",      "캐릭터가 이동 입력을 받아 이동 중인 상태")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Sprinting,  "Character.State.Sprinting",   "캐릭터가 달리기(스프린트) 중인 상태")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Attacking,  "Character.State.Attacking",   "캐릭터가 공격 중인 상태")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Dodging,    "Character.State.Dodging",     "캐릭터가 회피 모션 중인 상태")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Staggered,  "Character.State.Staggered",   "캐릭터가 피격 경직 중인 상태")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_KnockedDown,"Character.State.KnockedDown", "캐릭터가 넘어진(다운) 상태 — 회피·공격 불가")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Invincible, "Character.State.Invincible",  "무적 프레임 — 피격 판정 무시")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Dead,       "Character.State.Dead",        "캐릭터가 사망한 상태")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_ShieldMode, "Character.State.ShieldMode",  "한손검 방패 모드 — 방패를 든 자세로 이동 가능, 공격/피격 시 해제")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Aiming,     "Character.State.Aiming",      "활 조준 모드 — 카메라 정면 방향으로 조준 자세, 홀드 중 유지")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_LockOn,      "Character.State.LockOn",      "근접무기 록온 모드 — 특정 몬스터를 카메라가 지속 추적")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Traveling,   "Character.State.Traveling",   "레벨 이동 중 — 입력 및 어빌리티 차단")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Flying,      "Character.State.Flying",      "비행 중")

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Attack_Hit, "Event.Attack.Hit", "히트 판정 시스템이 충돌을 감지했을 때 발행하는 이벤트")

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Stamina_RegenBlocked, "State.Stamina.RegenBlocked", "스태미너 회복이 차단된 상태 (소모 직후 딜레이 중)")

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Damage,       "SetByCaller.Damage",       "GE 적용 시 호출측이 주입하는 데미지 크기")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_StaminaCost,  "SetByCaller.StaminaCost",  "GE 적용 시 호출측이 주입하는 스태미나 소모 크기")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_MaxHealth,    "SetByCaller.MaxHealth",    "몬스터 스탯 초기화 시 주입하는 최대 체력")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_AttackPower,  "SetByCaller.AttackPower",  "몬스터 스탯 초기화 시 주입하는 공격력")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_DefensePower, "SetByCaller.DefensePower", "몬스터 스탯 초기화 시 주입하는 방어력")

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Walk,         "Ability.Walk",         "걷기/이동 어빌리티")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Sprint,       "Ability.Sprint",       "스프린트 어빌리티")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Dodge,        "Ability.Dodge",        "회피/구르기 어빌리티")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Attack,       "Ability.Attack",       "약공격 콤보 어빌리티")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Attack_Heavy, "Ability.Attack.Heavy", "강공격 콤보 어빌리티")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Attack_Melee, "Ability.Attack.Melee", "활 근접 공격 어빌리티")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ShieldMode,   "Ability.ShieldMode",   "한손검 방패 모드 토글 어빌리티")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Aim,          "Ability.Aim",          "활 조준 모드 홀드 어빌리티")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_LockOn,       "Ability.LockOn",       "근접무기 록온 토글 어빌리티")

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Attack_BowNormal, "Ability.Attack.BowNormal", "활 일반(비조준) 공격 어빌리티")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Attack_BowAimed,  "Ability.Attack.BowAimed",  "활 조준 공격 어빌리티")

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Monster_Bite,        "Ability.Monster.Bite",        "몬스터 물기 공격")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Monster_ClawAttack,  "Ability.Monster.ClawAttack",  "몬스터 발톱 공격")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Monster_TwoHitCombo, "Ability.Monster.TwoHitCombo", "몬스터 2타 콤보 공격")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Monster_SpitFireBall,"Ability.Monster.SpitFireBall","몬스터 화염구 발사")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Monster_SpreadFire,  "Ability.Monster.SpreadFire",  "몬스터 범위 화염 브레스")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Monster_TakeOff,     "Ability.Monster.TakeOff",     "몬스터 이륙 전환")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Monster_Land,        "Ability.Monster.Land",        "몬스터 착지 전환")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Monster_FlySpitFireBall, "Ability.Monster.FlySpitFireBall", "몬스터 비행 중 화염구 발사")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Monster_FlySpreadFire,   "Ability.Monster.FlySpreadFire",   "몬스터 비행 중 범위 화염 브레스")
}
