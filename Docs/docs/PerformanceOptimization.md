# Q7 클라이언트 최적화 시스템 정리

> Unreal Engine 기반 **모바일 우선(Mobile-first) 크로스플랫폼 MMORPG** 클라이언트(코드명 Q7 / ArcheAge WAR)의 성능·메모리·네트워크 최적화 코드와 기능을 정리한 문서.
> 다수의 유닛/이펙트/네트워크 트래픽이 동시에 발생하는 MMO 특성상, "거리·부하 기반 적응형 차등 처리"가 최적화의 중심 사상이다.

## 목차
1. [핵심 사상 한눈에 보기](#1-핵심-사상-한눈에-보기)
2. [Significance Manager — 최적화의 중심 허브](#2-significance-manager--최적화의-중심-허브)
3. [렌더링 / LOD / Culling](#3-렌더링--lod--culling)
4. [메모리 / 오브젝트 풀링](#4-메모리--오브젝트-풀링)
5. [에셋 로딩 / 스트리밍](#5-에셋-로딩--스트리밍)
6. [멀티스레딩 / 비동기 / 병렬 처리](#6-멀티스레딩--비동기--병렬-처리)
7. [네트워크 최적화](#7-네트워크-최적화)
8. [설정(Config) 기반 최적화](#8-설정config-기반-최적화)
9. [주요 CVar 레퍼런스](#9-주요-cvar-레퍼런스)
10. [핵심 파일 요약](#10-핵심-파일-요약)

---

## 1. 핵심 사상 한눈에 보기

| 축 | 전략 | 대표 구현 |
|----|------|-----------|
| **적응형 부하 관리** | 화면 혼잡도를 실측(EMA)해 Low/Middle/High 3단계로 품질을 자동 조절 | `UQ7SignificanceManager` |
| **거리 기반 차등** | 카메라/플레이어 거리순으로 유닛을 정렬해 가까운 것만 고품질 | `Unit_SignificanceFunction` |
| **적응형 오프로딩** | 비동기/병렬/동기를 항상 쓰지 않고, 부하·잡 개수 임계치로 게이팅 | `AsyncMovementOptimizer`, 투사체 시스템 |
| **재사용 우선** | 메모리 풀·위젯 풀·파티클 풀로 고빈도 할당/해제 제거 | `TSparseMemoryPool`, `FGlobalUserWidgetPool` |
| **지연 로드** | 전 에셋을 Soft Reference로 참조, 필요 시점에 비동기 스트리밍 | `GameAssetCache`, `TSoftObjectPtr` |
| **대역폭/CPU 절감** | 패킷 배칭·LZ4 압축·전송 억제·차원 축소 | `FNetConn`, `SendActorMovement` |
| **런타임 튜닝** | 거의 모든 최적화 경로를 `q7.*` CVar(`ECVF_Scalability`)로 노출 | 전역 |

가장 중요한 특징은 **`UQ7SignificanceManager`가 단일 진실 공급원(single source of truth)** 이라는 점이다. 렌더링 LOD, 애니메이션 틱, 이펙트 표시, 심지어 네트워크 이동 처리의 비동기 전환 여부까지 이 매니저가 측정한 "Overhead Level"에 연동된다.

---

## 2. Significance Manager — 최적화의 중심 허브

**`Source/Q7/Q7SignificanceManager.cpp` / `.h`** — `UQ7SignificanceManager : USignificanceManager`

매 프레임 등록된 모든 `AUnit`을 거리 기준으로 정렬(`bSortSignificanceAscending = true`, 가까울수록 우선)하고, 각 유닛에 우선순위 인덱스(Priority)를 부여한다. 이후 모든 세부 최적화가 이 인덱스를 소비한다.

### 2-1. 유닛 중요도 산정 (`Unit_SignificanceFunction`, 654행)
- 로컬 유닛 / 보스는 항상 최고 우선순위(`Lowest()`) 고정.
- 나머지는 거리를 계산하고, `MaxDrawDistance`(기본 10000) 밖이거나 View Frustum(`IntersectSphere`, 678행) 밖이면 `Max()`를 반환해 컬링 대상으로 분류.
- `Register`/`UnRegister`(862/875행): `ShouldSupportSignificance()`인 유닛만 등록.

### 2-2. Overhead Level — 적응형 품질 자동 조절 (`UpdateContext`, 705행)
화면에 보이는 플레이어/논플레이어 수를 가중 평균(`AverageOverheadScore`, EMA)내어 **Low(0~24%) / Middle(25~49%) / High(50~100%)** 3단계 부하 레벨을 산출(`ComputeOverheadLevel`, 298행)하고 `GEngine->SetDynamicOverheadLevel`(741행)로 엔진에 전달한다.

부하 레벨에 따라 아래 상한값들을 `Lerp`로 부드럽게 조정 (34~46행 테이블, **데스크탑/모바일 분리**):

| 상한 테이블 | 데스크탑 (Low/Mid/High) | 모바일 (Low/Mid/High) | 의미 |
|-------------|------------------------|------------------------|------|
| `MaxShadowCastCount` | 100 / 50 / 30 | 50 / 25 / 15 | 동적 그림자 캐스팅 유닛 수 |
| `MaxGroaVisibleCount` | 50 / 25 / 20 | 20 / 10 / 5 | 펫/소환수(Groa) 표시 수 |
| `MaxSoftBlendCount` | 50 / 25 / 10 | 20 / 10 / 5 | 소프트 마스크 블렌드 수 |
| `MaxHitFXVisibleCount` | 50 / 25 / 10 | 20 / 10 / 5 | 피격 이펙트 수 |

- **논플레이어 가중치**: `q7.Significance.NonPlayerScoreScaleFactor`(기본 0.2) — 몬스터는 부하 계산에서 0.2배로 취급해 **플레이어 표현을 우선 보존**.
- 혼잡할수록 자동으로 표현 품질을 낮춰 프레임을 방어하는 self-tuning 구조.

### 2-3. 유닛별 표현 결정 (`AUnit::SignificanceDecision`, 925행)
매니저가 계산한 Priority/거리로 각 유닛의 렌더 상태를 **dirty-flag 방식(변경 시에만 적용)** 으로 갱신한다:
- **동적 그림자 on/off** — 우선순위 낮은 유닛의 `SetCastShadow(false)` (1003행)
- **Dynamic Min LOD** — `SetMinLOD()`로 메인 메시·Groa·Mount·장착 무기 메시 모두 강제 LOD 상향 (987/1017행)
- **Soft mask blend** — 원거리 페이드 (`SetSoftMaskBlend`, 992행)
- **거리 기반 디졸브** — 몬스터/토템이 draw distance 안에 들어올 때 페이드인 디졸브 (1040행)

### 2-4. URO (Update Rate Optimization) — 애니메이션 틱 프레임 스킵 (`SetupURO`, 903행)
스켈레탈 메시에 다음을 설정하여 원거리/비가시 유닛의 애니메이션 평가를 여러 프레임에 걸쳐 건너뛴다(time-slicing):
- `VisibilityBasedAnimTickOption = OnlyTickPoseWhenRendered` — 렌더링될 때만 포즈 갱신
- `bEnableUpdateRateOptimizations = true`
- `bComponentUseFixedSkelBounds` — 스켈레탈 바운드 고정(매 프레임 바운드 재계산 제거)
- `OnAnimUpdateRateParamsCreated`(689행)에서 `LODToFrameSkipMap` 구성 — LOD가 낮을수록 더 많이 스킵
- 호출처: Unit, Groa, Ship/Mount, Doodad

### 2-5. Niagara 파티클 중요도 (`CalculateSignificance`, 750행)
파티클도 동일한 거리/정렬 알고리즘(로컬 폰 거리 기준)으로 significance를 계산해 컬링한다.

### 2-6. 구동 지점
- **매 프레임**: `Source/Q7/Q7GameViewportClient.cpp:253` — `UpdateViewFrustum` → `SignificanceManager->Update`
- 컷신 등에서는 `PauseDynamicOverheadLevel` / `Resume`로 일시 정지
- **디버그**: `showdebug Unit / Doodad / DistanceDebug / UpscaleAlgorithm / DumpSparseMemory` — overhead score, 가시 유닛 수, URO 파라미터 실시간 표시

---

## 3. 렌더링 / LOD / Culling

### 3-1. LOD
- **커스텀 Dynamic Min LOD** — Significance overhead + `q7.Significance.DynamicMinLod` CVar로 각 스켈레탈 메시의 최소 LOD를 실시간 상향 (2-3 참고)
- **MockUnit(AI 유닛)** — `Unit/MockUnit.cpp:554` `SetForcedLOD(1)`
- **레벨 배치 스켈레탈 메시** — `Private/Q7LevelPlacedSkeletalMeshActor.cpp:12` `LDMaxDrawDistance = 10000`

### 3-2. Culling / Visibility
- **Cull Distance Volume 병렬 갱신** — `Utils/CullDistanceVolumeUtil.cpp` — 레벨 스트리밍 로드 시 정적 프리미티브의 `MaxDrawDistance`를 볼륨 기준으로 재계산. `ParallelFor` + **lock-free 컨테이너**(`TLockFreePointerListUnordered`)로 병렬 처리(144행). CVar: `q7.CullDistanceVolume.Support`, `q7.CullDistanceVolume.ParallelUpdate`
- **거리 컬링** — `q7.Significance.MaxDrawDistance` 밖 + `MaxUnitDrawLimit` 초과 유닛은 `SetHiddenSignificance(true)`. 즉 "가까운 N명만" 표시
- **View Frustum 컬링** — `IntersectSphere` 절두체 교차 판정
- **잔디/환경** — `q7.Grass.CullDistanceScale`, 밀도 스케일, 저티어 그림자 disable (`Utils/PerPlatformEngineScalabilityValueRewrite.cpp`)
- **Fake Deferred Light** — `TodEnv/Q7FakeDeferredLight.cpp` 라이트 메시 `MaxDrawDistance = 3000` + 머티리얼 페이드
- **NameTag 컬링** — `q7.Significance.MaxDrawUnitNameTag`(기본 200) 상위 유닛만 이름표 표시

### 3-3. Tick 최적화
- **애니메이션 tick 억제** — `VisibilityBasedAnimTickOption` 광범위 사용. Doodad는 상호작용 중에만 `AlwaysTickPose`로 승격
- **Doodad tick on/off** — `Doodad/Doodad.cpp:641` `OptimizeDoodad`: Ready 상태에서 `SetRenderStatic(true)` + `SetComponentTickEnabled(false)`, 상호작용 시에만 tick 활성화
- **거리 기반 스폰/디스폰** — `Doodad/DoodadSpawner.cpp:122` `SpawnDistanceSq`/`DespawnDistanceSq`로 원거리 Doodad 액터 자체를 동적 생성/제거
- **위젯 틱 간격 분산** — `Widget/PopupWidgets.cpp` `UpdateTickInterval = 0.3f`로 고빈도 위젯 갱신을 저빈도로 축소

### 3-4. 업스케일링 / 동적 해상도
- **`Q7GameUserSettings.cpp`**: DLSS/DLAA, XeSS(`r.XeSS.Quality`), FSR2/TAAU(`r.FidelityFX.FSR2.QualityMode`) 품질 모드 remap 테이블
- `GRHISupportsDynamicResolution = true`(891행) 동적 해상도
- 그래픽 옵션의 **시야 거리 설정이 significance 유닛 표시 상한과 연동** — `ScalabilityQuality.ViewDistanceQuality` → `q7.Significance.MaxUnitDrawLimit` CVar 반영(694행)

---

## 4. 메모리 / 오브젝트 풀링

### 4-1. 커스텀 스몰 오브젝트 메모리 풀 (`TSparseMemoryPool.h`)
*Modern C++ Design*의 Small Object Allocator 변형. 고정 크기 청크를 미리 할당하고 **free-list(각 슬롯이 다음 빈 슬롯 인덱스를 저장)로 O(1) 할당/해제**. 요청 크기가 슬롯보다 크거나 풀이 가득 차면 `::operator new`로 폴백. → **짧은 수명의 임시 객체가 유발하는 힙 단편화 방지**.

### 4-2. HUDStore Action 객체 풀링 (`HUDStore/HSAction.h`)
- 3단계 풀: `SmallSparsePool<32,32>` / `DefaultSparsePool<128,64>` / `LargeSparsePool<8,1024>`
- `MakeShared`가 컴파일타임에 객체 크기로 **최적 풀을 `std::conditional_t`로 자동 선택**
- 주석에 JIRA 이슈(QSEV-22967) 기반 실측으로 풀 크기를 튜닝한 내역 기록
- → 매 프레임 대량 발생하는 Flux Action 객체의 `TSharedRef` 컨트롤 블록 할당을 풀로 대체

### 4-3. UMG 위젯 풀 (`GlobalUserWidgetPool.h/.cpp`)
- `ActiveWidgets` / `InactiveWidgets` 두 리스트로 `UUserWidget` 재사용. 클래스 일치하는 비활성 위젯을 우선 재사용, 없으면 `CreateWidget`
- `CachedSlateByWidgetObject`(TMap)로 하부 SWidget까지 캐시
- `Tick`에서 `CacheStayCount` 초과분을 `CacheRemoveInterval` 주기로 **점진적 제거**(GC 스파이크 완화)
- `USE_WIDGET_CACHE` 매크로로 on/off. `AddReferencedObjects`로 GC 안정성 확보

### 4-4. Niagara 파티클 풀 (`Utils/ParticleEffectUtil.cpp`)
- 언리얼 내장 Niagara 풀(`ENCPoolMethod::ManualRelease` / `AutoRelease`) 활용. `ReleaseParticle`이 `Deactivate` 후 `ReleaseToPool()`
- `bUseAttachParentBound` 최적화(CVar `q7.Niagara.UseAttachParentBound`)로 바운드 계산 생략
- 코드베이스 전반 25개 파일에서 사용(무기/유닛/기믹/Doodad AnimNotify 이펙트)

### 4-5. 투사체 데이터 지향(DOD) 서브시스템 (`Combat/ProjectileSubsystem.cpp`)
- 별도 Actor 없이 `TArray<FProjectileUpdateContext>`를 `ParallelFor`로 병렬 틱(504행, 20개 이상일 때만 병렬)
- `CompactArray`(47행): 만료 투사체 제거 후 일정 슬랙 + 60초 타임아웃 조건에서만 `Shrink()` → 잦은 재할당 방지
- 파티클은 `ManualRelease` 풀 사용, 만료 시 풀 반환

### 4-6. Garbage Collection
- `Utils/LevelUtil.cpp:16` — 레벨 전환 시 `ForceGarbageCollection(true)`로 명시적 즉시 GC
- `Cms/BaseCmsTable.h:180` — CMS 테이블을 `AddToRoot()`로 GC 대상에서 제외(장기 상주 캐시화)
- 모바일 한정 `gc.MaxObjectsInGame=262144` + `gc.FlushStreamingOnGC=1`(Android)로 메모리 스파이크 완화

---

## 5. 에셋 로딩 / 스트리밍

### 5-1. 게임 에셋 비동기 캐시 (핵심) — `Resource/GameAssetCache.h/.cpp`
- `UAssetManager::GetStreamableManager().RequestAsyncLoad`로 `FSoftObjectPath` 배열을 비동기 로드, `FStreamableHandle` 보관
- 완료 델리게이트, 우선순위(`Priority`), 블로킹 옵션 지원
- `CancelStreaming`으로 로딩 중단 → 텔레포트 취소 시 리소스 낭비 방지
- **사용처**: `AUnit`, `ANpc`, `ADoodad`, `AGimmick`, `MountComponent`, `GroaComponent`, `ALootItem`, `ASummonObject`, `Preview` 등 **거의 모든 월드 액터가 스폰 시 이 통일된 경로로 스트리밍**

### 5-2. Soft Reference 전면 사용 — `Resource/GameResource.h`
- 에셋 테이블 Row 구조체 대부분이 `TSoftObjectPtr<>` / `TSoftClassPtr<>`로 메시/텍스처/머티리얼/Niagara/사운드/애니 참조
- `FQ7ParticleSocket::ConditionalLoad`: 플랫폼 지원 여부 확인 후 로드 → 불필요 플랫폼 에셋 로드 생략
- → 하드 레퍼런스 제거로 초기 패키지 로드/메모리 상주량 최소화

### 5-3. 존/레벨 스트리밍 — `Teleport/ZoneTransferManager.cpp`
- **2단계 게이팅**: `LoadLevel` → `OnLevelStreamCompleted`(서브레벨 스트림) → `OnAssetStreamCompleted`(액터 에셋 스트림) 순차 진행
- **SameLevel 스킵**: 동일 레벨이면 언로드/로드 생략하고 위치만 이동
- **선(先)로드**: `PeekLevelStreaming`로 목적지 스트리밍 레벨 미리 로드
- **타임아웃 보호**: `MaxStreamTimeSec`(10초) 내 미완료 시 강제 완료 또는 로비 복귀
- `q7.LevelStreaming.TileStreamingDistanceScaleFactor`(Scalability)로 타일 스트리밍 반경 조절

> 참고: **World Partition은 사용하지 않고**, 서버 존(Zone) 기반 레벨 로드 + 서브레벨/CullDistanceVolume 스트리밍 + ForceGC 조합으로 메모리를 관리한다.

### 5-4. CMS 데이터 프리로드 필터 — `Cms/BaseCmsTable.h`
- CSV → `UDataTable` 생성 후 `AddToRoot()`로 상주
- `IsPreload=true` 행은 로드 시점에 제거(`RemoveCurrent()`) → 클라이언트에 불필요한 서버 전용 데이터를 메모리에서 배제

---

## 6. 멀티스레딩 / 비동기 / 병렬 처리

### 6-1. 커스텀 스레드 (FRunnable)
- **FNetThread** (`Network/Lib/NetThread.cpp`) — 소켓 I/O 전용 스레드(`TPri_AboveNormal`). 연결/Send/Recv/압축/암호화/통계를 모두 처리. 게임 스레드와는 `Requests`/`Events` 큐 + `FRWLock`으로 교환, 게임 스레드는 `FetchEvents()`로 논블로킹 폴링. **프로젝트 최대 규모의 워커 스레드**
- **FTimeThread** (`Thread/TimeThread.cpp`) — 프레임 히치 감지 전용. `FRunnable` 루프에서 실측 경과 시간이 기준(`q7.limitSeconds`, 1.3초)을 초과하면 `FThreadSafeCounter`에 기록, 게임 스레드가 폴링. 게임 스레드가 멈춰도 독립적으로 히치 감지

### 6-2. TaskGraph 비동기 오프로딩
- **유닛 이동 지면 투영** (`Network/ClientWorldNet.cpp:312`) — 다수 원격 유닛의 지면 라인트레이스(`ProjectToFloor`)를 `FDelegateGraphTask`로 `AnyThread`에 오프로딩. 게임 스레드에서 POD 스냅샷(`FTraceData`)만 캡처해 스레드 안전 보장. CVar `q7.AsyncMovementMode`로 강제 비동기(3)/부하 연동(2)/즉시 병렬 선택
- **네임바 가림 판정** (`Widget/WorldWidgets.cpp:392`) — 라인트레이스를 워커 스레드로 오프로딩. **더블 버퍼링**(`Container[2]`)으로 프레임 간 겹치게 처리해 스톨 방지, **위치 해싱으로 중복 잡 제거**, `q7.NameBar.LineTraceJobMinCount` 이상일 때만 비동기
- **결제 콜백 마샬링** (`InappPurchase/PurchaseManager.cpp`) — 외부 SDK 콜백을 `GameThread`로 안전 마샬링

### 6-3. ParallelFor 데이터 병렬
- **투사체 틱** — DOD 배열을 `ParallelFor`로 병렬화(4-5 참고). 요청 수 > 20일 때만 병렬, 렌더 컴포넌트 접근은 게임 스레드에서 순차
- **Cull Distance Volume** — lock-free 컨테이너로 병렬 계산(3-2 참고)

> **설계 특징 — 적응형 오프로딩**: 비동기/병렬/동기를 항상 쓰지 않고, `SignificanceManager`의 실측 Overhead 레벨과 잡 개수 임계치(CVar)로 게이팅하여 **태스크 디스패치 오버헤드가 이득을 넘지 않을 때만 전환**한다. 워커 스레드에서 UObject 직접 접근을 금지하고 게임 스레드에서 POD 데이터를 미리 복사하는 패턴이 일관되게 적용된다.

---

## 7. 네트워크 최적화

자체 TCP 소켓 + 커스텀 바이너리 스트림(`CL::Stream`) + 코드생성 프로토콜(`*_gen`) 기반. 언리얼 표준 NetDriver/리플리케이션은 게임플레이에 사용하지 않는다.

### 대역폭 절감
- **패킷 배칭** (`Network/Lib/NetConn.cpp:263` `FlushSendQueue`) — 여러 메시지를 하나의 물리 패킷으로 묶음. 한 패킷당 최대 `MAX_BODY_SIZE`(1MB) 또는 63개 스트림. `FStreamHeader`는 **24bit BodySize + 6bit StreamCount + 1bit Compress + 1bit Encrypt = 단 4바이트**
- **LZ4 압축** (`NetConn.cpp:338`) — 묶인 body가 `CompressTargetMinSize`(200바이트) 초과 & 실제로 작아질 때만 `FCompression::CompressMemory(NAME_LZ4)`. 작은 패킷은 압축 생략(CPU 낭비 방지)
- **이동 데이터 차원 축소** (`Common_gen.h:116` `FMovementInfo`) — velocity를 `FNetVector2D`(X,Y만, Z 생략), 회전은 `Yaw` float 하나만 전송(Pitch/Roll 생략). 이동 패킷당 12바이트 절감
- **이동 전송 억제 / Dead-Reckoning** (`ClientWorld.cpp:1542` `SendActorMovement`) — 매 틱 전송하지 않고 상태 전환/속도 유의미 변화/Yaw 임계치 초과/주기적 강제 동기화 시에만 전송. 등속 직선 이동 시 중복 제거로 업스트림 대폭 절감
- **요청 중복 억제** (`ClientWorldNet.cpp:407`) — 패킷 타입별 마지막 전송 타임스탬프를 초 단위로 기록, 같은 초 내 동일 타입 스팸 차단
- **ID 비트 패킹** (`Protocol/StrongId.h:11` `TStrongId`) — int64에 Timestamp(40bit)+WorldId(9bit)+BinaryId(5bit)+Fraction(9bit) 패킹

### CPU 절감
- **비동기 이동 처리** (`ClientWorldNet.cpp:111` `AsyncMovementOptimizer`) — 수신 이동 잡을 모으되 **같은 ActorId는 최신 것으로 덮어써 중복 제거**, 바닥 트레이스를 병렬 처리. Significance 부하와 연동해 필요 시에만 async
- **memcpy 직결 직렬화** (`Protocol/Stream.h:388`) — 산술 타입은 `is_memcpy_able` 경로로 memcpy 직결, 리틀엔디안 고정(스왑 비용 없음)

### Interest Management (AoI)
- 서버가 관심영역 내 액터만 선별해 **배치 스폰**(`Clientzone_gen.h:479` `FZ2CActorsSpawn`, `TArray<FActorSpawnParam>`) / **배치 디스폰**(`Z2CActorsDestroy`)으로 전송
- 클라는 존 단위 액터 레지스트리(`UClientWorld`)로 존 내 액터만 관리

> **한계**: 좌표(`FNetVector`)는 여전히 32bit float로, 비트 단위 quantization(int16 고정소수점)은 미적용. "차원 축소 + 전송 빈도 억제 + LZ4 배치 압축" 조합으로 대역폭을 줄이는 설계.

---

## 8. 설정(Config) 기반 최적화

**모바일 우선 크로스플랫폼**(`DefaultEngine.ini` `TargetedHardwareClass=Mobile`). 표준 `DefaultScalability.ini` 대신 플랫폼별 `Android/IOS/Windows` Scalability로 분리 관리.

### 3대 성능 관리 축
1. **플랫폼별 `*Scalability.ini`의 `sg.*` 5그룹 티어링** — ViewDistance / Shadow / Texture / AA / Effects / Foliage / PostProcess / Shading을 품질 0~3으로 정의, 디바이스 벤치마크 점수(`PerfIndexThresholds`)로 자동 티어링
2. **`DefaultDeviceProfiles.ini`의 기기별 Low/Mid/High 프로파일 + 메모리 버킷** — 실제 상용 GPU(Adreno/Mali/Xclipse/Apple)를 3티어로 매핑, RAM 용량별(`_Smaller/_Smallest`) 자동 조정
3. **커스텀 `q7.*` CVar 시스템** — Significance, URO, 레벨 타일 스트리밍, 잔디 밀도, 유체 시뮬

### 주요 설정
- **텍스처 스트리밍 풀** — `r.Streaming.PoolSize` 300~500MB(모바일 메모리 버킷별), 텍스처 LOD 그룹 캡(모바일 1024 / PC 2048)
- **모바일 렌더링** — `r.MobileHDR=True`, 정적 라이팅·Virtual Texture·레이트레이싱·GPU 파티클·SkinCache **전면 비활성**, `r.EarlyZPass=3`(오버드로우 절감), `r.Mobile.SupportGPUScene=True`(드로우콜 인스턴싱), `r.SecondaryScreenPercentage.GameViewport=75`
- **RHI** — Android Vulkan(파이프라인 LRU 캐시, defrag), iOS Metal. `r.ShaderPipelineCache`(PSO 프리컴파일로 히칭 방지)
- **물리** — Chaos `DefaultThreadingModel=TaskGraph`, `MaxPhysicsDeltaTime=0.0333`, `bSubstepping=False`, `CTF_UseSimpleAsComplex`(충돌은 항상 단순 도형)
- **PC 전용 고급** — 동적 해상도, FSR2, SSGI, Distance Field 그림자
- **`r.MobileContentScaleFactor`** (0.8~1.5) — 기기별 렌더 해상도 조정, 발열/배터리/성능 밸런스의 핵심 레버

---

## 9. 주요 CVar 레퍼런스

대부분 `ECVF_Scalability`로 노출되어 플랫폼/디바이스별 튜닝 가능.

| CVar | 기본값 | 역할 |
|------|--------|------|
| `q7.Significance.MaxUnitDrawLimit` | 100 | 동시 표시 유닛 상한 (시야 거리 옵션 연동) |
| `q7.Significance.MaxDrawDistance` | 10000 | 유닛 드로우 거리 |
| `q7.Significance.DynamicMinLod` | 0 | 동적 최소 LOD |
| `q7.Significance.SortAlgorithm` | 1 | 0=카메라 / 1=로컬폰 기준 정렬 |
| `q7.Significance.NonPlayerScoreScaleFactor` | 0.2 | 논플레이어 부하 가중치 |
| `q7.Significance.MaxDrawUnitNameTag` | 200 | 이름표 표시 상한 |
| `q7.Significance.SoftMaskBlendDistance` | 3000 | 소프트 블렌드 활성 거리 |
| `q7.URO.BaseNonRenderedUpdateRate` | 4 | 비렌더 애님 갱신 주기 |
| `q7.URO.MaxEvalRateForInterpolation` | 4 | 보간 최대 평가 주기 |
| `q7.URO.LodSkipFrameScaleFactor` | 1.0 | LOD별 프레임 스킵 배율 |
| `q7.AsyncMovementMode` | 2 | 이동 처리 비동기 모드(0~3) |
| `q7.NameBar.LineTraceJobMinCount` | - | 네임바 트레이스 비동기 임계치 |
| `q7.CullDistanceVolume.ParallelUpdate` | - | 컬 볼륨 병렬 갱신 |
| `q7.Projectile.UseParallUpdate` | - | 투사체 병렬 틱 |
| `q7.Niagara.UseAttachParentBound` | - | 파티클 바운드 계산 생략 |
| `q7.LevelStreaming.TileStreamingDistanceScaleFactor` | - | 타일 스트리밍 반경 |
| `q7.Grass.CullDistanceScale` / `DensityScale` / `DisableShadow` | - | 잔디 컬링/밀도/그림자 |
| `q7.FluidSim.Enable` / `MaxDraw` | - | 물 유체 시뮬레이션 |

---

## 10. 핵심 파일 요약

| 영역 | 대표 파일 |
|------|-----------|
| **적응형 부하 관리 (중심)** | `Q7SignificanceManager.cpp` / `.h` |
| 매 프레임 구동 | `Q7GameViewportClient.cpp` |
| 유닛 표현 결정 | `Unit/Unit.cpp` (`SignificanceDecision`) |
| 커스텀 메모리 풀 | `TSparseMemoryPool.h`, `HUDStore/HSAction.h` |
| 위젯 풀 | `GlobalUserWidgetPool.h/.cpp` |
| 파티클 풀 | `Utils/ParticleEffectUtil.cpp` |
| 투사체 DOD | `Combat/ProjectileSubsystem.cpp` |
| 컬 디스턴스 병렬 | `Utils/CullDistanceVolumeUtil.cpp` |
| 비동기 에셋 캐시 | `Resource/GameAssetCache.h/.cpp` |
| Soft Reference | `Resource/GameResource.h` |
| 존/레벨 스트리밍 | `Teleport/ZoneTransferManager.cpp`, `ClientWorld.cpp` |
| CMS 캐시/프리로드 | `Cms/BaseCmsTable.h` |
| 네트워크 배칭/압축 | `Network/Lib/NetConn.cpp`, `NetCore.h` |
| 이동 전송 억제 | `ClientWorld.cpp` (`SendActorMovement`) |
| 비동기 이동/네임바 | `Network/ClientWorldNet.cpp`, `Widget/WorldWidgets.cpp` |
| 커스텀 스레드 | `Network/Lib/NetThread.cpp`, `Thread/TimeThread.cpp` |
| 플랫폼 스케일러빌리티 | `Utils/PerPlatformEngineScalabilityValueRewrite.cpp` |
| 그래픽 설정/업스케일 | `Q7GameUserSettings.cpp` |
| Config | `Config/DefaultEngine.ini`, `DefaultDeviceProfiles.ini`, `{Android,IOS,Windows}/*Scalability.ini` |
