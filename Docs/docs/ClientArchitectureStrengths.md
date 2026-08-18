# Q7 클라이언트 아키텍처 강점 문서

> 작성일: 2026-06-30 · 대상: `Source/Q7` 약 62만 줄 (C++ 클라이언트, 416 cpp / 450 h)
> 목적: 이 프로젝트가 **잘 하고 있는 설계·관행**을 근거(file:line)와 함께 기록. 신규 인원 온보딩 및 신규 코드 작성 시 따라야 할 레퍼런스.

## 한눈에 보기

62만 줄 규모의 MMORPG 클라이언트임에도 **인프라 계층이 건강하다.** 문제(리팩토링 후보)는 누적된 보일러플레이트에 있지, 근간 설계에 있지 않다. 핵심 강점 8가지:

| # | 강점 | 핵심 위치 |
|---|------|----------|
| 1 | 테이블 기반 패킷 디스패치 | `Network/Lib/NetPacketHandler.h` |
| 2 | 도메인 로깅 + NO_LOGGING 컴파일 제거 + JSON | `Utils/Q7Log.h` |
| 3 | Mixin 헬퍼 / 제네릭 위젯 인프라 | `Utils/WidgetUtil.h` |
| 4 | Composition over Inheritance (컨트롤러) | `PlayerController/CounterAttack.h`, `NotifyController.h` |
| 5 | Shipping 빌드 디버그/치트 이중 차단 | `Utils/Cheater.h`, CVar `ECVF_Cheat` |
| 6 | 에셋 경로 부트스트랩 집중 | `Q7.cpp`, `Resource/GameResource.cpp` |
| 7 | 데이터 주도 CMS (제네릭 테이블 + 리전 필터) | `Cms/BaseCmsTable.h` |
| 8 | 기능별 모듈 분리 + 모듈별 문서화 | `Source/Q7/*` 49개 디렉토리 + CLAUDE.md |

---

## 1. 네트워크/패킷 계층 — 테이블 기반 O(1) 디스패치 + 코드젠 분리

**무엇이 좋은가**: 수백 개 패킷을 거대 `switch`/if-else 없이 **패킷 타입 번호를 배열 인덱스로 쓰는 함수 테이블**로 O(1) 디스패치한다. 프로토콜 정의(코드젠)와 핸들러 로직(손코딩)이 물리적으로 분리되고 컴파일 타임 타입 안전성이 보장된다.

**근거**
- `Network/Lib/NetPacketHandler.h:8` — `FNetPacketHandler<THandler, PACKET_MAX>` 템플릿. `TArray<THandleFunc, TInlineAllocator<PACKET_MAX>>`로 패킷 수만큼 인라인 할당된 함수 테이블(82행).
- `Register<T_REQ>()` (29-51행): 패킷 구조체 타입에서 인덱스를 얻어(`GetPacketStreamType<T_REQ>()`, 32행) 슬롯에 **역직렬화+핸들러 호출을 캡슐화한 람다**를 심음. 람다가 `Stream.PopArgs(Req)`로 파싱 후 `Invoke(Func, Handler, Req)` 호출 → 핸들러는 파싱된 타입 안전 구조체만 받음.
- `Handle()` (63-79행): `HandleFuncs[StreamType]` 조회 실행, `IsValidIndex` 범위 가드.
- 타입→인덱스가 컴파일 타임 상수: `Network/ClientNetBase.h:21` `static constexpr GetPacketStreamType()`.
- 등록 = 문서: `Network/ClientWorldNet.cpp:794~` `PacketHandler.Register(&UClientWorldNet::HandleError);` 멤버 함수 포인터만 넘기면 인자 타입에서 패킷 번호 자동 추론. 핸들러는 `bool HandleXxx(FW2CXxx const& Packet)` 형태로 나열.
- 테이블 크기가 프로토콜 enum `Max`에 연동: `ClientWorldNet.h:963`.
- 코드젠 분리: `Network/Protocol/Clientworld_gen.h:1` `// This file is generated ... Do not edit manually`. 직렬화 계약은 `STREAM_DEFINE` 매크로(`Protocol/StructSerialize.h:17`) 하나로 `ToStream`/`FromStream`/`Tie` 동시 생성 → push/pop 순서 불일치 원천 차단.
- UI 블로킹 정책의 데이터화: `UnBlockUIs` 비트배열로 "이 패킷 수신 시 UI 잠금 해제"를 코드가 아닌 등록 플래그로 관리.

**왜 좋은가**: 거대 switch 부재로 유지보수성↑, 파싱-핸들러 결합 실수 방지, 프로토콜 드리프트 방지, 동일 템플릿을 AuthNet/ClientWorldNet/Workbench가 재사용.

---

## 2. 로깅 시스템 — 도메인 카테고리 + NO_LOGGING 컴파일 제거 + 구조화 JSON

**무엇이 좋은가**: 단일 헤더에서 (1) 60여 개 도메인 카테고리, (2) 릴리즈 빌드에서 로그를 **전처리기 단계에서 완전 소거**, (3) 타입 안전 key-value JSON 직렬화를 제공.

**근거** (`Utils/Q7Log.h`)
- 도메인 카테고리 254-312행: `Q7_NET`/`Q7_COMBAT`/`Q7_QUEST`/`Q7_GUILD`/`Q7_SIEGE`... 60여 개, 카테고리별 기본 verbosity 개별 설정 → 시스템별 로그 소음 독립 제어.
- NO_LOGGING 제거 192-252행: `#if NO_LOGGING`일 때 모든 `Q7JsonLogXxx` 매크로를 빈 정의로 치환 → 릴리즈에서 JSON 직렬화 비용이 코드에서 사라짐(0 비용).
- JSON 매크로: `Q7MakeJsonLog(Verbosity, Subject, KV...)`(126행) + `Q7KV(Key, Value)`(140행). 타입별 오버로드로 안전 직렬화(22-105행) — **int64/uint64는 문자열로**(정밀도 손실 방지), enum은 이름 문자열, UStruct는 리플렉션 raw JSON, 포인터는 null 가드. `TIsLogValueType` 트레이트로 SFINAE 분기.
- 컨텍스트 자동 주입: `Q7JsonLogNetConn`(328행)이 연결 객체의 Name/Id/Remote를 자동 KV로 붙임.

**왜 좋은가**: 구조화 로그로 수집기(Sentry 등) 필드 검색/집계 가능, 릴리즈 성능 무영향, int64 문자열화로 64비트 ID 정밀도 버그 예방, 도메인별 노이즈 격리.

---

## 3. 유틸/헬퍼 인프라 — Mixin 헬퍼 + 제네릭 위젯 템플릿

**무엇이 좋은가**: `WidgetUtil.h`가 상속 없이 기능을 조합하는 **Mixin 헬퍼 클래스군**, 위젯 생성/순회 **제네릭 템플릿**, 전역 변환 함수를 한 곳에 모음.

**근거** (`Utils/WidgetUtil.h`)
- Mixin 헬퍼 486-619행: `FPurchaseNoticeWidgetHelper`(486), `FItemQuantitySelectWidgetHelper`(502, 수량 선택 UI+콜백 자체 보유), `FGemDetailHelper`(554), `FAutoImbueSettingBase`(576, 가상 훅), `FPreviewPanelHelper`(606). 각 헬퍼는 `InitXxx(UUserWidget* Parent)`로 부모에 부착 → 위젯이 필요한 헬퍼만 골라 mixin.
- 제네릭 템플릿 299-399행: `ForEachChild<T>`(타입+visibility 필터 순회), `CreateChildWidgetIfNotFound`(있으면 재사용/없으면 생성/타입 다르면 교체, 위젯 풀링 로직 일반화), `TSoftClassPtr` 오버로드로 지연 로드.
- 변환 함수 집약 62-280행: `GetGradeFilterType`가 아이템/가챠/클래스/그로아/탈것/함선/평판 등급 enum에 오버로드.
- `Q7TextUtil.h`: `Q7_MENU_TEXT`/`Q7_POPUP_TEXT` 매크로로 스트링테이블 조회 단축, 지역화 텍스트 접근 일원화.

**왜 좋은가**: 상속 폭발 방지(mixin 조합), 위젯 생성/풀링 중복 제거, 지역화 문자열 산재 방지.

---

## 4. Composition over Inheritance — 컨트롤러 관심사의 서브오브젝트 분리

**무엇이 좋은가**: 비대해지기 쉬운 `AWorldPlayerController`가 전투·알림·회복·반격을 **UObject가 아닌 순수 C++ 클래스(F-prefix)로 분리해 멤버로 소유**. 상속 대신 조합.

**근거**
- `PlayerController/WorldPlayerController.h:667-674` — `FSkillActionManager`, `FTargetActionManager`, `FRecoverController`, `FNotifyController`, `TSharedPtr<FCounterAttack>`를 멤버로 보유하고 접근자로 노출. 상속 계층은 얕게 유지, 기능은 서브오브젝트로 채움.
- `PlayerController/CounterAttack.h:21` — 순수 클래스. 생성자가 협력 객체를 **참조 주입**(`explicit FCounterAttack(const FTargetActionManager&, bool)`, 29행), 상태머신(`ECounterAttackTargetState`, 8-19행)을 자기 안에 캡슐화. `TSharedPtr`로 지연 생성(참조 준비 후 초기화).
- `PlayerController/NotifyController.h:7` — 순수 클래스. 저체력/PK/포션부족/무게초과/적접근/킬어시스트 6개 관심사를 하나의 알림 도메인으로 묶되 컨트롤러 본체에서 분리. 협력 객체를 `TWeakObjectPtr`로 **약참조 보유**(130-131행) → 순환 소유·댕글링 방지.

**왜 좋은가**: God object 방지(단일 책임 유지), 격리된 로직으로 이해·수정 용이, UObject 리플렉션/GC 오버헤드 회피, 약참조로 소유 안전성 확보.

---

## 5. 빌드 구성 안전성 — Shipping 빌드에서 디버그/치트 이중 차단

**무엇이 좋은가**: 디버그/치트 코드가 `#if !UE_BUILD_SHIPPING` 가드 + `ECVF_Cheat` CVar 플래그로 이중 차단 → 배포 빌드에 개발 코드가 새지 않음.

**근거**
- 치트 시스템 전체 가드: `Utils/Cheater.h:3` `#if !UE_BUILD_SHIPPING`로 `FCheater` 클래스 전체를 감쌈 → Shipping에서 심볼 소멸.
- 디버그 드로잉 이중 방어: `Unit/Unit.cpp:3703-3706` `#if !UE_BUILD_SHIPPING` 내부에서 `if (CVarDebugForcedMovement.GetValueOnGameThread())` 후 `DrawDebugSphere`. (437, 3464행 동일 패턴)
- CVar에 `ECVF_Cheat`: `Unit/Unit.cpp:79-107` `CVarDebugArmState`/`CVarDebugMovement`/`CVarShowDamage` 등 모두 `ECVF_Cheat`로 등록 → Shipping에서 자동 잠금.
- 서브시스템 디버그 토글: `Combat/ProjectileSubsystem.cpp:129-136` `GProjectileDebug` + `q7.Projectile.Debug` CVar 전체가 빌드 가드 내부.
- ini 저장 유틸도 가드: `Utils/Q7ConsoleVariable.h:18-20`.
- 규모: `UE_BUILD_SHIPPING/TEST/DEBUG`가 30+ 파일 86회+ 사용 → 팀 컨벤션.

**왜 좋은가**: 디버그/치트는 성능 비용 + 보안 리스크를 동시에 가짐. 컴파일 타임 제거로 런타임 오버헤드 0, `ECVF_Cheat`가 실수로 남은 CVar마저 배포 환경에서 무력화.

---

## 6. 에셋/리소스 관리 — 하드코딩 경로의 부트스트랩 집중

**무엇이 좋은가**: `/Game/...` 경로가 소수 부트스트랩 로더에만 집중, 100+ 위젯/게임플레이 파일에 산재하지 않음. `ConstructorHelpers`는 사실상 미사용.

**근거**
- `TEXT("/Game/...")` 24회, 단 8개 파일에만 존재 — `Q7.cpp`(7), `GameResource.cpp`(5), `Q7Cms.cpp`(2)에 집중.
- CMS 루트 상수화: `Q7.cpp:251` `const TCHAR* CmsPath = TEXT("/Game/Cms")`.
- BP 로드 집중: `Q7.cpp:528,542,556` `StaticLoadClass`로 한 곳에서.
- 실제 에셋은 DataTable + `TSoftObjectPtr<>` 필드로 위임(`GameResource.h`의 `FModelAssetRow` 등) → 비동기 스트리밍 가능.
- `ConstructorHelpers`는 단 1개 파일(`TodEnv/Q7FakeDeferredLight.cpp`)에 국한.

**왜 좋은가**: 경로 산재 시 리소스 이동/리네이밍 추적 불가·크래시 위험. 부트스트랩 집중 + `TSoftObjectPtr`는 경로 변경을 데이터 편집으로 흡수하고 생성자 동기 로드 병목을 회피.

---

## 7. CMS(데이터 테이블) — 데이터 주도 설계 + 리전 필터 내장

**무엇이 좋은가**: 기획 데이터가 codegen + 제네릭 테이블 베이스로 타입 안전 관리되고, WorldServer(리전/서버 타입)별 필터링이 조회 API에 내장.

**근거** (`Cms/BaseCmsTable.h`)
- 제네릭 베이스 `TBaseCmsTable<TKey, TRow>`(20-316행): `FindRow`/`GetAllRows`/range-for/`Contains`를 한 곳에 구현, 모든 CMS 테이블이 상속 → 조회 코드 중복 제거.
- 리전 필터 내장: 이터레이터 `Advance`가 `IsMatchWorldServer`로 자동 스킵(83-87행), `FindRow`(215-245)/`GetAllRows`(259-272)가 자동 필터 → 호출부가 서버 타입 신경 쓸 필요 없음.
- 타입 안전 키: `FindRow(const TKey& Key)`의 키가 `int32`가 아닌 `FAchieveType`/`FDungeonId` 래퍼 구조체 → ID 종류 혼동을 컴파일 타임 차단.
- codegen 분리: `CMS_gen.*`/`CMSType_gen.h`/`Const_gen.*`는 자동 생성(수정 금지), 손코딩(`Q7Cms.cpp`, `Q7Const.cpp`)과 분리.
- 편집/배포 이원화: 패키징 `.uasset`은 `Load()`, 개발 중 CSV는 `LoadCSV()`(124-183행) → 에디터에서 CSV 즉시 반영.

**왜 좋은가**: 수만 행 기획 데이터를 리전별로 배포하는 MMORPG에서, 필터를 조회 계층에 내장하면 리전별 버그가 구조적으로 차단됨. 타입 안전 키가 ID 혼동 실수를 컴파일러가 검출.

---

## 8. 모듈/디렉토리 구조 — 기능별 관심사 분리 + 문서화

**무엇이 좋은가**: `Source/Q7` 하위가 49개 기능 디렉토리로 분리, 각 디렉토리마다 `CLAUDE.md` 문서 존재.

**근거**
- 도메인 디렉토리: `Combat/`, `StatusChange/`, `Stat/`, `ActionManager/`, `Network/`, `Cms/`, `HUDStore/`, `Widget/`, `Mount/`, `Teleport/` 등 49개. UI 상태관리(`HUDStore/`)와 UI 뷰(`Widget/`) 분리 = Flux 패턴.
- 자동 생성 코드 격리: `Network/Protocol/*_gen`, `Cms/*_gen`이 수기 코드와 물리 분리 → codegen 재실행이 수기 코드 미접촉.
- 계층 명확: `Q7GameInstance`(전역) → `Q7World`(네트워크+월드 허브) → `ClientWorld`(존 액터 레지스트리) 3계층.
- 공통 인터페이스: `IActorInterface`를 Unit/NPC/Doodad/Gimmick/LootItem/Torpedo가 구현 → 월드 액터 취급 통일.
- 매직넘버 외부화: `UnitAnimInstance.h:22-70` `UQ7UnitAnimInstanceSettings : UDeveloperSettings`가 `SkillEndWaitingSeconds` 등을 `GlobalConfig`로 노출, 소비 지점은 `GetDefault<...>()->GetSkillEndWaitingSeconds()`. (`UQ7ProjectileSettings` 등 동일 패턴)
- const 규율: const 참조 반환 + const 멤버 함수를 게터에 일관 적용, `BaseCmsTable`은 const/비const 오버로드 모두 제공.
- 문서화: 루트 + 하위 모듈 `CLAUDE.md` 총 49개, `docs/`에 설계 문서.

**왜 좋은가**: 62만 줄에서 기능 경계가 병렬 작업·영향 범위 국소화를 가능케 함. 매직넘버 외부화로 재컴파일 없이 밸런싱, const 규율로 의도치 않은 변경 컴파일러 차단, 모듈 문서로 온보딩 단축.

---

## 신규 코드 작성 시 따라야 할 레퍼런스

이 강점들은 곧 **팀 컨벤션**이다. 신규 코드는 다음을 따를 것:
1. 패킷 핸들러는 `PacketHandler.Register(&Handler)` 패턴 (직접 파싱 금지)
2. 로그는 `Q7JsonLogXxx` + `Q7KV` 사용 (raw `UE_LOG(LogTemp)` 금지)
3. 공통 UI 조각은 `WidgetUtil.h`의 헬퍼/템플릿 재사용, 필요 시 같은 헤더에 추가
4. 컨트롤러/거대 클래스의 관심사는 F-prefix 순수 클래스로 분리 (composition)
5. 디버그/치트 코드는 `#if !UE_BUILD_SHIPPING` + `ECVF_Cheat` 가드 필수
6. 에셋은 하드코딩 경로 대신 DataTable + `TSoftObjectPtr`
7. 매직넘버는 `UDeveloperSettings` 클래스로 외부화
