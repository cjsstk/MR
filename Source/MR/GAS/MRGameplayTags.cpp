// Fill out your copyright notice in the Description page of Project Settings.

#include "MRGameplayTags.h"

namespace MRGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Moving,    "Character.State.Moving",    "캐릭터가 이동 입력을 받아 이동 중인 상태")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Sprinting, "Character.State.Sprinting", "캐릭터가 달리기(스프린트) 중인 상태")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Dead,      "Character.State.Dead",      "캐릭터가 사망한 상태")

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Walk,   "Ability.Walk",   "걷기/이동 어빌리티")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Sprint, "Ability.Sprint", "스프린트 어빌리티")
}
