// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 정수 기반 강타입(Strong Type) ID의 공통 베이스.
 *
 * 서로 다른 종류의 ID(ItemId, SkillId 등)가 함수 시그니처에서 실수로 섞이지 않도록
 * C++ 함수 경계(CMSSubsystem 조회, MRInventoryComponent API 등)에서만 사용한다.
 * DataTable Row나 FMRInventorySlot 같은 실제 저장/직렬화 필드는 그대로 int32를 쓰고,
 * 이 타입으로의 변환은 함수 호출 시점에만 한다 — 아무 곳에도 안 쓰는 리플렉션/
 * BlueprintReadOnly를 붙일 이유가 없고, 에디터에서도 평범한 int 입력칸으로 남는다.
 */
struct FStrongIdBase
{
	int32 Value = 0;

	FStrongIdBase() = default;
	explicit FStrongIdBase(int32 InValue) : Value(InValue) {}

	bool IsNone() const { return Value == 0; }

	/** DataTable 조회에 사용할 RowName으로 변환한다 (숫자 문자열). */
	FName ToRowName() const { return FName(*FString::FromInt(Value)); }

	FString ToString() const { return FString::FromInt(Value); }
};

/**
 * ID 종류 하나를 선언하는 매크로. 새 ID 종류 추가 시 이 한 줄이면 된다.
 * 비교 연산자를 베이스가 아니라 여기(파생 타입)에 두는 이유: 베이스 타입 기준으로
 * 정의하면 FItemId == FSkillId처럼 서로 다른 종류가 암시적 업캐스트로 비교 가능해져
 * 버려서, 강타입을 쓰는 목적 자체가 무너진다.
 */
#define DECLARE_STRONG_ID(IdType) \
	struct IdType : public FStrongIdBase \
	{ \
		IdType() = default; \
		explicit IdType(int32 InValue) : FStrongIdBase(InValue) {} \
		friend bool operator==(const IdType& A, const IdType& B) { return A.Value == B.Value; } \
		friend bool operator!=(const IdType& A, const IdType& B) { return A.Value != B.Value; } \
		friend uint32 GetTypeHash(const IdType& Id) { return ::GetTypeHash(Id.Value); } \
	};

DECLARE_STRONG_ID(FItemId)
DECLARE_STRONG_ID(FSkillId)
DECLARE_STRONG_ID(FWeaponId)
DECLARE_STRONG_ID(FArmorId)
DECLARE_STRONG_ID(FRecipeId)
DECLARE_STRONG_ID(FDropTableId)
DECLARE_STRONG_ID(FFieldId)
