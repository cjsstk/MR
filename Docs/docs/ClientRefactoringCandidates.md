# Q7 클라이언트 코드 개선 리포트

> 작성일: 2026-06-30 · 대상: `Source/Q7` 약 62만 줄 (C++ 클라이언트, 416 cpp / 450 h)
> 분석 영역: Widget / Network / 게임플레이 코어(Unit·PlayerController·Stat) / 횡단 관심사

## 요약

인프라 자체는 건강하다. 문제는 **누적된 보일러플레이트·복붙·God class·레거시 잔재**에 집중돼 있다.

**좋은 점**
- 로깅 인프라(`Q7Log.h`의 60+ 도메인 카테고리, `NO_LOGGING` 시 컴파일 제거) 우수
- 패킷 디스패치는 거대 switch가 아닌 **테이블 기반**(`FNetPacketHandler`)으로 잘 설계됨
- 에셋 경로는 부트스트랩 로더(`GameResource.cpp`, `Q7.cpp`)에 집중 관리
- 디버그 드로잉 대부분 CVar로 가드됨

---

## 🟢 즉시 적용 가능 (저위험 Quick Win)

동작 변경 없이 안전하게 정리 가능. 일이 없을 때 착수하기 가장 좋음.

### 1. `CastChecked` 후 무의미한 null 체크 제거 (정확성)
`CastChecked<T>()`는 실패 시 즉시 크래시하고 Shipping에선 검증 없이 동작하므로, 그 결과를 `if(!x)`로 감싸는 건 **죽은 분기 + 잘못된 안전감**.
- `ClientWorld.cpp:565-569`
- `AnimNotifies/WeaponAnimNotifies.cpp:150`
- `HUD/BaseHUD.cpp:211,229`
- `Groa/GroaAnimInstance.cpp:88`, `Doodad/DoodadAnimInstance.cpp:102`
- → 실패 가능 입력이면 `Cast<>`+체크로, 불변식이면 `if` 제거로 **의미를 둘 중 하나로 통일**. (AnimInstance 캐스트는 BP 재지정 위험이 있어 `Cast<>`가 안전)

### 2. 로그 핫패스의 `FindObject<UEnum>(ANY_PACKAGE, ...)` → `StaticEnum<>()` (성능)
`Q7Log.h:145,152`의 `TEnumToString`이 로그를 찍을 때마다 전체 오브젝트 해시 검색 수행. `ANY_PACKAGE`는 UE5 deprecated. 같은 파일 166행에서 이미 `StaticEnum<>`을 올바르게 사용 중이므로 패턴만 맞추면 됨.
- 동일 패턴: `Cms/Q7Cms.cpp:2701`, `Utils/Cheater.cpp:717,2256,2664`, `Utils/Q7TextUtil.cpp:4626`, `Widget/UpgradeWidgets.cpp:5509`

### 3. 레거시 데드 코드 정리
- **m48 만료 빈 핸들러** (현재 m58/m59): `Network/ClientWorldNet.cpp:3668-3709` 4개 — 본문이 `//TODO: Remove after m48.0` + `return true`뿐.
  `HandleCashShopBuyReadyForPc`(3668), `HandleCashShopBuyPrepareForPc`(3674), `HandleCheckConsumedReceipt`(3680), `HandleCashShopRefundPrepareForPc`(3705).
  ⚠️ **서버가 더 이상 송신 안 하는지 확인 후** 핸들러·헤더 선언·Register를 함께 제거.
- **주석 처리된 옛 구현**: `Network/ClientWorldNetTask.cpp:705-735`(31줄 통째 주석), `Utils/WidgetUtil.h:427-443`(컴파일 실패로 막아둔 템플릿 `// TODO : this Param Compile Fail`), `Widget/WorldWidgets.cpp:5798`(빈 핸들러 `OnHSActionMenuBack`)
- **세미콜론 중복**: `Stat/StatModifier.cpp:521, 591` (`Value;;`)

---

## 🟡 점진적 표준화 (중간 작업, 신규 코드부터 강제)

### 4. 위젯 수동 바인딩 → `meta=(BindWidget)` 표준화 — 최대 핫스팟
같은 코드베이스에서 두 방식 혼용: UE 표준 `meta=(BindWidget)` **1466회** vs 구식 수동 `CastChecked<T>(GetWidgetFromName("Name"))` **2219회**. 수동 방식은 문자열 리터럴이라 오타/이름변경이 **런타임 크래시로만** 드러나고, `NativeConstruct`에 바인딩 코드가 수십~180줄씩 누적.
- 예: `Widget/WorldWidgets.cpp:5361-5542`(180줄 연속), `Widget/GuildWidgets`(수동 99 vs 자동 1), `Widget/MapWidgets`(수동 65+ vs 자동 2)
- → **신규는 `meta=(BindWidget)` 강제**(코드리뷰 규칙). 기존은 파일 단위 점진 전환. 가장 광범위하면서 런타임 안정성까지 개선.

### 5. 복붙 패턴을 헬퍼/베이스로 추출 (동작 불변)
프로젝트에 이미 `WidgetUtil.h`의 Mixin 헬퍼 패턴(`FItemQuantitySelectWidgetHelper` 등)이 있으니 동일 방식으로 흡수.
- **`SetArrowAngle` 5중 복제**: `Widget/MapWidgets.cpp:485,571,693,760,2412` → 마커 베이스 클래스(`UMinimapMarkerBaseWidget`)로
- **Compose/Summon `Make*` 함수 4쌍 복제**(콘텐츠 타입 Class/Groa/Mount/WeaponIllusion만 다름): `Network/ClientWorldNetTask.cpp:2242~3049` (`MakeGroaCompose`/`MakeClassCompose`는 라인 단위 1:1 대응) → traits/템플릿화
- **스탯 디스패치 3중 if-else 5곳 복붙**: `Stat/StatModifier.cpp:502,581,654,671,708` → `ApplyToResultMap(Category, 람다)` 헬퍼로
- **패킷→`FSkillEffectResult` 변환 4함수 복제**: `Unit/Unit.cpp:5834,5850,5870,5887` → `MakeResultFromPacket` 헬퍼
- **슬롯형 버튼 4-바인딩 세트**: `Widget/GachaWidgets.cpp:226-230`, `Widget/ItemWidgets.cpp:3510-3514` → 공통 헬퍼
- **인덱스 위젯 루프 바인딩**(`Printf("XXX%d") + GetWidgetFromName`): `Widget/GuildWidgets.cpp:2954,6194,5793` → `BindIndexedWidgets<T>` 템플릿

### 6. `ToImpl<>` null 체크 일관화 (안전성)
`Network/ClientWorldNetTask.cpp`에서 `ToImpl<T>()` 216회 호출 중 null 체크는 35회뿐 — 나머지는 캐스팅 실패 시 즉시 크래시. → 패킷 핸들러가 이미 람다에서 검증하는 것처럼, **Register 단계에서 검증+캐스팅 후 검증된 참조를 핸들러에 전달**하는 시그니처로 통일(`NetPacketHandler.h:104-134`).
- 관련: `ClientWorldNetTask.cpp:2322` `MakeClassDecisionWaitConfirm`의 `case Class:`에 `break` 누락 — fall-through 의도 불명확(형제 함수는 break 있음). 확인 필요.

### 7. 매직 넘버 상수화 (가독성)
- ms→초 변환 `* 0.001f` 산재: `Unit/Unit.cpp:787,1062,1069,3526,4921,4969,5802` → `SecFromMs()` 또는 명명 상수
- 애니메이션 보정 `+0.5f`/`+0.2f`(`Unit.cpp:4943,4991,5811`), CC 지연 `0.1f`(`Unit.cpp:5104`) → `UQ7UnitSettings` 등 설정으로 이동(기존 `GetSkillEndWaitingSeconds()` 패턴과 일관)
- 값복사 인자 184건(`const TArray<>&` 누락): 예 `Q7GameInstance.h:243`, `Widget/ItemWidgets.h:434`, `Widget/GachaWidgets.h:1034-1035` → const-ref로

---

## 🔴 구조적 리팩토링 (고영향·고위험, 별도 계획 필요)

### 8. God class / God file 분리
한 번에 하지 말 것. 경계가 뚜렷한 것부터.
- **`AUnit`**(`Unit/Unit.cpp` 6,594줄, 멤버 함수 ~330개, 패킷 핸들러 36개) / **`AWorldPlayerController`**(`WorldPlayerController.cpp` 7,616줄):
  - 패킷 핸들러를 비-UObject 핸들러 객체로 분리(1000줄+ 감축 가능)
  - Cheat/Debug(이미 `#if !UE_BUILD_SHIPPING` 가드됨, `WorldPlayerController.cpp:1903~2638`)는 별도 컴파일 단위로
  - 프리징 감지(`7484~7616`)·통계·콜로세움은 컴포넌트로 추출 (기존 `FCounterAttack`/`FNotifyController` 패턴, "Composition over inheritance"와 일관)
- **God 파일 분할**: `GachaWidgets`(70 UCLASS), `WorldWidgets`(44), `GuildWidgets`(63), `MapWidgets`(36)
  - 길드 능력/노동, 미니맵 마커군, 액션버튼군처럼 경계 뚜렷한 묶음부터
  - **1차 방어선은 "신규 위젯을 기존 god file에 더 넣지 않기"**
  - ⚠️ 헤더 의존 그래프(forward declare)를 함께 정리해야 함

---

## ⚠️ 잠재 위험 (사각지대) — 직렬화 매크로 누락 시 무경고 fallback

### 9. POD 패킷 구조체에서 `STREAM_DEFINE` 누락 시 조용히 memcpy됨
> 이 항목은 문서 작성 후 직렬화 계층을 추적하다 발견. 현재 실제 버그는 아니고 **컨벤션에 의존하는 사각지대**.

**문제**: `UGameNet::Send<T>()`(`Network/Lib/GameNet.h:34`)는 `Stream.PushArgs(T::GetType(), Req)`로 패킷을 직렬화한다. `PushArgs` → `PushT`(`Network/Protocol/Stream.h`)가 컴파일 타임에 타입을 보고 경로를 고르는데:
- 패킷 구조체에 **비-POD 멤버(`FString`/`TArray`/중첩 구조체 등)가 있으면** `is_memcpy_able_t`가 거짓 → `ToStream` 경로만 남음 → `STREAM_DEFINE`(=`ToStream`)이 없으면 **컴파일 에러로 즉시 발견** (안전).
- 패킷 구조체가 **순수 POD(int/enum 등)이면** `is_memcpy_able_t`가 참 → `PushMemory(&Req, sizeof(Req))`로 **구조체 메모리를 통째 memcpy**. 이 경우 `STREAM_DEFINE`이 없어도 **컴파일이 통과**하고, 필드 단위 프로토콜 규격이 아닌 in-memory 레이아웃(패딩 포함)이 전송됨 → 서버 기대 포맷과 어긋나면 **런타임에 조용히 깨진 패킷**.

**근거**: `Stream.h:331`(일반 `PushT`→`ToStream`), `:348`(`is_memcpy_able_t` memcpy 경로), `:49-52`(`has_to_stream_type`/`is_stream_struct` 컨셉). 수신측도 `PopArgs`→`PopT`(`:494, 512`)에서 대칭.

**개선 방향**:
- 현재는 codegen이 모든 패킷에 `STREAM_DEFINE`을 항상 넣어주는 것에 안전이 의존 → **`_gen` 파일 수동 편집 금지 규약이 곧 안전장치**임을 문서/리뷰에서 명시.
- 손으로 패킷/네트워크 구조체를 만들 경우 POD라도 `STREAM_DEFINE`을 반드시 붙일 것(코드리뷰 체크리스트化).
- (선택) `Send<T>` 진입부에 `static_assert(has_get_type_stream_struct<T>)`류 컴파일 타임 가드를 추가하면 POD 사각지대까지 컴파일 에러로 승격 가능. 단 memcpy 최적화를 의도적으로 쓰는 구조체가 있는지 먼저 확인 필요.

---

## 권장 착수 순서

위험 대비 효과 순으로 **위→아래**:
1. 🟢 **1·2·3** — 즉시, 안전 (CastChecked 정리 / 로그 enum 성능 / 데드코드 삭제)
2. 🟡 **5·6·7** — 복붙 추출·매직넘버, 동작 불변
3. 🟡 **4** — 위젯 바인딩 신규 강제 + 점진 전환
4. 🔴 **8** — God class 분리, 별도 계획
5. ⚠️ **9** — 직렬화 매크로 사각지대: 규약 명시(즉시) + `static_assert` 가드(선택)

---

## 참고: 분석에서 "문제 아님"으로 확인된 것

- 패킷 핸들러 디스패치 구조는 테이블 기반으로 양호(리팩토링 불필요)
- 헤더 선언만 있고 정의 없는 핸들러 의심 8건은 모두 오탐
- `#pragma once` 누락은 서드파티/자동생성 파일뿐
- `UE_LOG` 직접 사용 17건으로 적음(`LogTemp` 사용 3건만 정리 대상: `CashShopStore.cpp:856`, `CashShopWidget.cpp:2676`, `ClientWorldNet.cpp:4870`)
- `_gen.cpp`(CMS_gen, Common_gen 등)는 자동생성이므로 분석/리팩토링 대상 제외
