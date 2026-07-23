// 액션 단일 정의 목록 (Single Source of Truth)
//
// 여기에 ACTION(이름, 필드...) 한 줄(항목)만 추가하면 다음이 모두 자동 생성된다:
//   - EActionType 열거값                (ActionTypes.h)
//   - 페이로드 struct FAction_이름        (Action.h)
//   - TActionTypeOf<FAction_이름> 매핑    (Action.h)
// enum 목록과 페이로드 목록이 서로 어긋날 수 없으므로,
// "enum만 추가하고 페이로드를 빠뜨리는" 종류의 실수가 구조적으로 불가능하다.
//
// 규칙:
//   - 필드는 한 줄에 하나씩 세미콜론으로 끝낸다.
//   - 타입에 최상위 콤마(예: TMap<A, B>)를 쓰지 않는다 — 매크로 인자 분리가 깨진다.
//     콤마가 필요하면 using 별칭을 만들어 콤마 없는 이름으로 쓴다.
//   - 필드가 없는 액션은 ACTION(이름) 처럼 두 번째 인자를 생략한다.
//
// 이 파일은 ACTION 매크로를 정의한 쪽(ActionTypes.h / Action.h)에서 #include 한다.
// 자체적으로 #include 가드를 두지 않는다 — 서로 다른 ACTION 정의로 여러 번 인클루드된다.

// ─── 캐릭터 ────────────────────────────────────────────────────────────────
ACTION(SetHealth,
	float Current = 0.f;
	float Max = 100.f;
)

ACTION(SetStamina,
	float Current = 0.f;
	float Max = 100.f;
)

// ─── 인벤토리 ──────────────────────────────────────────────────────────────
ACTION(AddInventoryItem,
	int32 ItemId = 0;
	int32 Count = 0;
	int32 SlotIndex = INDEX_NONE;
)

ACTION(RemoveInventoryItem,
	int32 ItemId = 0;
	int32 Count = 0;
)

ACTION(UseInventoryItem,
	int32 SlotIndex = INDEX_NONE;
)

ACTION(SyncInventorySlots)

ACTION(ShowGatherResult,
	int32 ItemId = 0;
	int32 Count = 0;
)

// ─── Store 알림 전용 (실제로 Dispatch되지 않고 Store::Notify()에서만 쓰임) ────────
// 여러 액션이 하나의 알림으로 모이는 경우(예: Add/Remove/UseInventoryItem이 전부
// 슬롯 변경 알림 하나로 모임), 그 알림 자체를 위한 전용 타입.
ACTION(InventorySlotsChanged,
	TArray<FMRInventorySlot> Slots;
)

ACTION(InventoryGatherResultChanged,
	TArray<FMRDropResult> Items;
)
