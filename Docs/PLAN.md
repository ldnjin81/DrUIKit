# DrUIKit 기획서

2026-11-12 초안

> **원본 문서.** 기획·결정의 원본은 이 파일(git)이다.

## 구현 현황 (2026-11-13)

**1차 부품 — 만든 것**(모두 갤러리 `L_DrUIGallery`와 `UDrGlassGallery::CoveredClasses()`에 있음)

| 영역 | 클래스 |
|---|---|
| 기반 | `UDrUITheme`, `UDrUIThemeSubsystem`(테마·표면 품질·표면 투명도·움직임 줄이기), `UDrUIKitSettings`, `EDrUIState`·`DrUIState::Resolve`, `DrUIMotion`(곡선·순차 등장 `PlayStagger`), `FDrUISpring` |
| 기초 | `UDrText`, `UDrIcon`, `UDrGlyph`(선으로 그리는 표시), `UDrSurface`(흐림·채움·테두리 + 그림자·윗면 반사광) |
| 누르는 부품 | `UDrButtonBase`(공통 상태 틀), `UDrButton`(9상태·변형 4종), `UDrHoldButton` |
| 선택 | `UDrToggleBase`, `UDrCheckBox`, `UDrRadio`, `UDrSwitch`, `UDrTabList`·`UDrTabButton`, `UDrSegmentedControl`, `UDrSlider`(눈금·값 말풍선) |
| 상태 표시 | `UDrBadge`, `UDrProgressBar`, `UDrProgressRing`, `UDrSkeleton`, `UDrSpinner` |
| 알림 | `UDrNoticeBase`, `UDrToast`, `UDrToastStack`, `UDrSnackbar` |
| 입력 | `UDrTextField`, `UDrDropdown`·`UDrDropdownMenu`·`UDrDropdownItem`, `UDrHorizontalSelector` |
| 게임용 | `UDrHoldButton`, `UDrRadialMenu` |
| 탐색·표시 | `UDrTooltip`, `UDrListRow`·`UDrListItem`, `UDrPageIndicator`, `UDrButtonPrompt` |

**기획과 달라진 점**

- 이름: 부품은 `UDr<부품>`, 기반 시스템만 `UDrUI`(위 '이름과 구성' 표). Select → `UDrDropdown`, Stepper → `UDrHorizontalSelector`, ChoiceBase → `UDrToggleBase`, Segmented → `UDrSegmentedControl`.
- 테마: 에디터 모듈·토큰 가져오기 커맨드릿 대신 Glass 샘플이 실행 중에 `tokens.json`(색·글자·간격·모서리·흐림·그림자)과 Pretendard TTF(코드로 복합 글꼴)를 읽는다.
- 표면 스타일 클래스(`UDrGlassSurfaceStyle`)는 만들지 않았다. `UDrSurface`가 토큰(채움·테두리·흐림·그림자·반사광)으로 다섯 층을 직접 칠한다.
- 목록: 엔진이 에디터에서 C++ 목록 항목 클래스를 거부하므로(`UListViewBase::RebuildWidget`) 전용 목록 위젯은 두지 않고, `UDrListRow`를 위젯 블루프린트의 부모로 쓰도록 했다. 짧은 목록은 버튼 묶음으로.
- 버튼 프롬프트: 아이콘 에셋 없이 키캡 글자로 그린다(아이콘이 필요하면 엔진 `UCommonActionWidget`).
- 툴팁 화살표는 넣지 않았다(엔진 툴팁 위치가 고정되지 않음).
- 결정(2026-11-13): 테마는 계속 실행 중에 `tokens.json`을 읽는다(테마 에셋·커맨드릿 없음). 어두운 테마 스위치 손잡이는 디자인 규칙대로 `on-accent`(남색) 유지.
- 흐림 환산은 1:1(CSS `blur(r)`의 r과 `SBackgroundBlur` 강도가 둘 다 가우스 표준편차). `saturate()`는 Slate에 없어 쓰지 않는다. 글자 크기는 px × 0.75(96dpi 포인트).
- 아이콘 모양은 게임이 정한다(코어는 에셋 없음). 코어가 그리는 표시는 `UDrGlyph`(체크·점·±·닫기·화살표·느낌표).

**2차 — 만든 것**(2026-11-13, 갤러리 '2차·확장'·'2차·탐색'·'2차·표현'·'게임용' 페이지)

| 영역 | 클래스 |
|---|---|
| 엔진 상속 확장 | `UDrScrollBox`, `UDrCountUp`, `UDrAnimatedSwitcher`, `UDrExpandableArea`·`UDrExpanderHeader`, `UDrTextArea` |
| 게임용 | `UDrCooldown`, `UDrFeed`·`UDrFeedEntry` |
| 행동·입력 | `UDrMorphButton`, `UDrQuantityStepper`·`UDrIconButton`, `UDrChip` |
| 탐색 | `UDrSidebar`·`UDrSidebarItem`, `UDrDock`·`UDrDockItem`, `UDrBottomTabBar`·`UDrBottomTabButton` |
| 알림·덮개 | `UDrBanner`, `UDrBottomSheet`, `UDrCoachMark`·`UDrCoachScrim`, `UDrPopover`·`UDrPopoverPanel` |
| 진행·완료·빈 상태 | `UDrStepIndicator`, `UDrCompletion`, `UDrEmptyState` |
| 움직임·데이터 | `UDrTextReveal`, `UDrSparkline`, `UDrDonut`, `UDrStatCard` |

- 바텀시트·코치마크는 화면 층이 생기기 전까지 게임이 맨 위 오버레이에 꽉 차게 둔다(1차 남은 것: 화면 층과 함께 그 층으로).
- 글자 등장의 '블러 인'은 Slate에 글자 흐림이 없어 올라오며 밝아짐으로 대신했다.

**1차 남은 것**: 확인 대화상자 + 화면 층(`UDrUIFlowSubsystem`) — DrFramework 0.3.0 UI 이동과 함께(DrFramework의 `UDrUIRootLayout`과 이름이 겹쳐 먼저 만들 수 없음). 2차 22개는 아래 표의 우선순위대로.

## 개요

DrUIKit은 언리얼 엔진 5.8 UMG 위에 만드는 **스타일 중립 UI 부품 플러그인**이다. 코어는 코드 레벨에서 동작·상태·입력·테마 틀만 제공하고, 실제 디자인은 저장소에 함께 들어가는 스타일 샘플이 입힌다. 첫 샘플은 `UISample/Glass`(Dr UI 디자인 언어의 리퀴드 글래스)다. 부품은 엔진·CommonUI 위젯을 상속해 확장하고, 상속으로 안 되는 것은 UserWidget 안에서 버튼·텍스트·표면을 조합해 만든다.

**왜 만드나**

- DrFramework는 일부러 스타일 없는 UI 틀(메뉴 동작, 화면 층, HUD 기반)만 갖고 있다. 실제 모양은 게임마다 다시 만들어 왔다(예: DrSample의 메뉴).
- 디자인 시스템(Dr UI 디자인 언어)에 토큰·컴포넌트·이름이 정해졌다. 이것을 언리얼에서 그대로 쓸 수 있는 부품이 필요하다.
- 패턴 이름(토스트, 스낵바, 스켈레톤, 버튼 9상태 등)을 클래스 이름으로 맞추면 기획·명세·AI 지시·코드가 같은 말을 쓴다.

**목표**

1. 테마(토큰 묶음)를 데이터로 정의하고 실행 중에 바꾼다. Glass 샘플은 밝은 유리·어두운 유리 두 테마를 제공한다.
2. 컴포넌트를 단계별로 제공한다. 모든 컴포넌트는 상태(기본·호버·눌림·포커스·비활성 등)를 다 갖는다.
3. 코어는 에셋 없이 C++만으로 동작하고(회색조 기본 모양), 디자인은 스타일 샘플이나 게임의 위젯 블루프린트로 입힌다.
4. 마우스·키보드·게임패드를 CommonUI 입력 체계로 같이 지원한다.
5. 다른 Dr 플러그인처럼 단독 호스트 프로젝트에서 빌드·자동화 테스트·화면 캡처로 검증한다.

**범위 밖**

- 게임 장르 UI(전투 HUD, 인벤토리 슬롯, 미니맵): 게임이 DrUIKit 부품으로 만든다.
- 웹·앱 UI: 같은 디자인 시스템을 쓰지만 이 플러그인과는 별개다.
- 유료 에셋·유료 폰트: 쓰지 않는다.

## 이름과 구성

이름은 **DrUIKit**(2026-11-12 확정). DrFramework·DrTableSystem과 같은 방식의 독립 플러그인 저장소로 만든다.

| 항목 | 값 |
|---|---|
| 저장소 | `DrUIKit`(새 프로젝트). 코어 플러그인 `unreal/DrUIKit`, 스타일 샘플 `UISample/<스타일>`, 단독 검증 호스트 `Host/DrUIKitHost.uproject` |
| 코어 모듈 | `DrUIKit`(런타임). 에디터 모듈은 아직 없음(토큰은 샘플이 실행 중에 `tokens.json`을 읽는다 — 아래 '구현 현황') |
| 스타일 샘플 | `UISample/Glass`: 플러그인 `DrUIGlass`(모듈 하나). 디자인 시스템 `tokens.json`, 테마 에셋 2개, 유리 표면 스타일, 머티리얼, 갤러리 화면. 나중에 다른 스타일도 같은 자리에 추가 |
| 클래스 이름 | 부품(위젯)은 `UDr<부품>`(`UDrButton`, `UDrSurface`, `UDrTabList`, 열거형·Slate도 `EDr…`·`SDr…`), 부품이 아닌 기반 시스템은 `UDrUI<…>`(`UDrUITheme`, `UDrUIThemeSubsystem`, `FDrUISpring`, `EDrUIState`). 파일 이름 = 클래스에서 접두사 뺀 것. Glass 샘플 `UDrGlass…` |
| 설정 | `UDrUIKitSettings`(프로젝트 설정 "DrUIKit": 쓸 테마 목록, 시작 테마, 표면 품질, 움직임 줄이기) |
| 테스트 이름 | `DrUIKit.<영역>.<항목>`, 샘플은 `DrUIGlass.<영역>.<항목>` |
| 코어 의존 모듈 | Core, CoreUObject, Engine, InputCore, SlateCore, Slate, UMG, CommonUI, CommonInput, DeveloperSettings |
| 다른 Dr 플러그인 의존 | 없음. DrFramework 없이 단독으로 빌드된다 |
| 공개 | 기획 확정 후 구현, DrSample에 서브모듈로 포함, GitHub 공개(MIT) |

**Dr 계열과의 관계**

- **DrFramework**: 게임 흐름(부팅, 맵 이동, 일시정지 상태, 네트워크, 콘텐츠, 검증)만 맡고 UI는 갖지 않는다. 지금 DrFramework에 있는 UI는 DrUIKit으로 옮긴다(아래 "DrFramework에서 옮겨 올 것"). 둘은 서로 의존하지 않고 게임에서 만난다.
- **DrTableSystem**: 의존하지 않는다. 문구는 게임이 스트링 테이블로 넣는다.
- **DrSample**: DrUIKit을 서브모듈로 추가해 메뉴·HUD를 DrUIKit으로 바꾼다. 실제 게임 화면 검증도 여기서 한다.
- **디자인 시스템**: 값과 이름의 원본이다. DrUIKit은 그 토큰을 가져와 쓰고, 값을 바꿀 때는 디자인 시스템을 먼저 고친다.

이름 충돌 주의: DrFramework에 이미 `UDrUISettings`, `UDrUIRootLayout`이 있다. DrUIKit은 설정 클래스를 `UDrUIKitSettings`로 두고, 새 클래스를 만들 때 DrFramework 클래스 목록과 겹치는지 확인한다.

### DrFramework에서 옮겨 올 것

DrFramework의 UI 코드(5개 파일, 약 200줄)를 모두 DrUIKit으로 옮기고, DrFramework에는 게임 흐름만 남긴다. 그러면 DrFramework의 CommonUI 의존이 빠지고 두 플러그인이 서로 의존하지 않는다.

| 지금(DrFramework) | 옮긴 뒤 | 비고 |
|---|---|---|
| `UDrUIRootLayout`(게임 층·메뉴 층 스택) | DrUIKit `UDrUIRootLayout`. 층을 늘림: 게임·메뉴·모달·알림(토스트) | 이름 유지, 모듈만 이동 |
| `UDrMenuWidget`(첫 포커스·뒤로 가기·메뉴 입력) | DrUIKit `UDrUIMenuWidget` | 이름 변경, 리다이렉트 |
| `UDrHUDWidget`(HUD 기반, 폰 바뀜 알림) | DrUIKit `UDrUIHUDWidget` | 이름 변경, 리다이렉트 |
| `UDrUISettings` 중 메뉴·HUD 클래스, UI 입력 컨텍스트, 일시정지 입력 | DrUIKit `UDrUIKitSettings` |  |
| `UDrUISettings` 중 타이틀 맵·게임 맵 | DrFramework에 남김(`UDrFlowSettings`로 이름 변경) | 맵 흐름은 게임 흐름 |
| `ADrPlayerController`의 UI 흐름: 메뉴 열기·닫기, 설정, 뒤로, 입력 모드, 타이틀 메뉴·HUD 생성, 일시정지 키 | DrUIKit `UDrUIFlowSubsystem(로컬 플레이어 서브시스템`) | 어떤 컨트롤러를 써도 동작(층·메뉴·HUD는 로컬 플레이어 단위) |
| `ADrPlayerController`의 게임 흐름: 일시정지 상태, 게임 시작, 타이틀 복귀, 재시작 | DrFramework에 남김(ADrPlayerController는 게임 흐름만) | UI 흐름 서브시스템은 일시정지 때 컨트롤러의 `SetPause`만 부른다 |

- DrFramework는 깨지는 변경이라 **0.3.0**으로 낸다. 클래스·설정 이동은 CoreRedirects로 받고, DrSample 등 사용하는 게임의 메뉴·HUD는 부모 클래스를 바꾼다.
- 순서: 지금 검수 중인 DrFramework 0.2.0(패키지 검증 도구)을 먼저 반영하고, DrUIKit 뼈대가 선 뒤 0.3.0에서 UI를 옮긴다.
- DrFramework의 설계 명세서·README의 UI 절은 DrUIKit으로 옮기고 "UI는 DrUIKit"이라고만 남긴다.
- 결정(2026-11-12): UI 흐름은 **로컬 플레이어 서브시스템**(`UDrUIFlowSubsystem`)으로 둔다. 게임은 어떤 컨트롤러를 상속해도 되고, DrFramework의 `ADrPlayerController`는 게임 흐름(일시정지 상태, 게임 시작, 타이틀 복귀, 재시작)만 남긴 채 유지한다. 전용 컨트롤러 안은 컨트롤러 상속이 겹쳐 대신 이 방식을 골랐다.

## 설계 원칙

부품은 세 가지 방법 중 앞의 것부터 고른다. 엔진이 이미 가진 동작(포커스, 입력, 접근성, 게임패드 탐색)을 다시 만들지 않기 위해서다.

**코어는 코드, 디자인은 샘플**

- 코어 `DrUIKit`: 컴포넌트의 동작·상태·입력·접근성, 테마 틀(토큰 이름과 조회·전환), 표면 슬롯(채움·테두리·모서리·그림자·뒤 흐림을 누가 칠할지). 모양 값은 없다.
- 스타일 샘플 `UISample/Glass`: 토큰 값, 유리 표면을 칠하는 스타일 클래스, 머티리얼, 글꼴 설정, 갤러리. DrFramework의 "플러그인은 구조만, 글래스 스타일은 샘플" 경계와 같은 규칙이다.
- 게임은 샘플을 그대로 쓰거나, 샘플을 본떠 자기 스타일을 만든다.

1. **상속**: 엔진이나 CommonUI에 같은 일을 하는 위젯이 있으면 상속해 모양과 상태만 더한다. 예: 버튼은 `UCommonButtonBase`, 탭은 `UCommonTabListWidgetBase`, 숫자는 `UCommonNumericTextBlock`, 입력 칸은 `UEditableTextBox`.
2. **조합**: 여러 부품이 모여 하나의 뜻을 이루면 `UUserWidget` 파생에서 DrUIKit 부품을 조합한다. 예: 사이드바 = 유리 패널 + 버튼 목록 + 배지, 통계 카드 = 패널 + 아이콘 + 숫자 + 델타 칩.
3. **직접 그리기**: 엔진에 없고 조합으로도 안 되는 것만 Slate로 그린다(스파크라인, 도넛, 진행 링). 최소한으로 둔다.

**에셋 없이 동작, 블루프린트로 교체**

- 조합 위젯은 하위 부품을 `BindWidgetOptional`로 받는다. 위젯 블루프린트가 같은 이름의 부품을 두면 그것을 쓰고, 없으면 C++가 기본 구성을 직접 만든다.
- 그래서 플러그인은 `.uasset` 없이도 빌드·테스트·갤러리가 돌고, 게임은 필요한 것만 블루프린트로 바꾼다.
- 코어는 콘텐츠 에셋을 갖지 않는다. 머티리얼·테마 에셋·글꼴은 스타일 샘플(`UISample/Glass`)이 갖는다.

**모양은 토큰에서만**

- 위젯에 색·크기·모서리 값을 직접 쓰지 않는다. 테마에서 토큰 이름(`accent`, `radius-lg`, `body`)으로 꺼낸다.
- 테마가 바뀌면 모든 DrUIKit 위젯이 다시 칠한다.

**상태는 빠짐없이**

- 버튼은 디자인 시스템의 9상태(기본·호버·눌림·포커스·비활성·로딩·성공·오류·선택됨)를 모두 갖는다. 상태는 열거형 하나(`EDrUIState`)로 다루고 테스트로 검사한다.

**문서의 원본은 git**

- 사람과 AI 도구가 같은 기준을 읽도록 모든 기준 문서의 원본은 저장소 안 마크다운이다. 디자인 시스템(토큰·이름 사전)의 원본은 별도로 관리하고, Glass 샘플의 `tokens.json`은 그 사본이다.

## 테마와 토큰

코어는 토큰을 담는 틀(테마 에셋 형식, 조회, 전환)만 갖고 값은 갖지 않는다. 스타일 샘플이 자기 `tokens.json`을 편집기 도구로 가져와 테마 에셋을 만든다(Glass 샘플 = Dr UI 디자인 언어의 값). 코어에는 코드로 만든 회색조 기본 테마 하나만 있어, 샘플 없이도 화면이 깨지지 않는다.

| 구성 | 하는 일 |
|---|---|
| `UDrUITheme`(UPrimaryDataAsset) | 테마 하나의 토큰 전체: 색(이름 → FLinearColor), 글자 스타일(이름 → 크기·굵기·줄 간격·글꼴), 간격, 모서리, 그림자, 흐림 강도 |
| `UDrUIKitSettings` | 쓸 테마 목록(Glass 샘플: 밝은 유리, 어두운 유리), 시작 테마, 표면 품질 단계, 움직임 줄이기 기본값 |
| `UDrUIThemeSubsystem`(EngineSubsystem — 에디터 디자이너 미리보기에서도 같은 테마) | 현재 테마 보관·전환, 토큰 조회(`GetColor("accent")`), 테마가 바뀌면 알림(`OnThemeChanged`) |
| 테마 알림 규칙 | 모든 DrUIKit 위젯이 테마 알림을 받아 `ApplyTheme()`을 다시 부른다 |
| (계획) `UDrUITokenImportCommandlet`(에디터) | `tokens.json` → 테마 에셋 2개. 지금은 만들지 않았다: UE 글꼴 가져오기가 커맨드릿(Slate 없음)에서 어서트로 실패해, 샘플이 실행 중에 `tokens.json`과 TTF를 직접 읽는다. |

**토큰 이름은 디자인 시스템 그대로**

- Glass 샘플 기준 색 23개(`ground`, `glass`, `glass-strong`, `ink`, `accent`, `chip` 등), 글자 9단계(`display`~`code`), 간격 7단계, 모서리 6단계, 그림자 3개, 흐림 3단계.
- 코드에서는 `FName`으로 찾는다. 자주 쓰는 것은 상수(`DrUITokens::Accent`)로 둘 수 있다.
- 디자인 시스템에 토큰이 추가되면 가져오기를 다시 돌린다. 없는 토큰을 찾으면 눈에 띄는 색(자홍)과 로그 경고를 낸다.

**언리얼로 옮기며 바뀌는 것**

- 색: CSS `rgba()`와 hex를 sRGB `FLinearColor`로 바꾼다.
- 글자: px는 언리얼 글꼴 크기(포인트)와 기준이 다르다. 환산 비율을 하나 정해 테마에 기록한다(1단계에서 화면 캡처로 맞춤).
- 그림자: CSS 그림자 문자열을 오프셋·흐림·퍼짐·색으로 나눠 그림자 머티리얼 파라미터로 넣는다.
- 흐림: `blur(24px) saturate(160%)` 중 흐림 강도만 쓴다(언리얼 배경 흐림에는 채도 조절이 없다).

## 스타일 샘플: UISample/Glass

코어의 `UDrSurface`는 표면을 다섯 슬롯(그림자, 뒤 흐림, 채움+테두리, 윗면 반사광, 내용)으로 나누기만 한다. 각 슬롯을 어떻게 칠지는 표면 스타일 클래스가 정하고, Glass 샘플의 `UDrGlassSurfaceStyle`이 아래 표대로 디자인 시스템의 "유리 패널 = 채움 + 테두리 + 그림자 + 흐림 + 윗면 반사광"을 만든다. 코어 기본 스타일은 불투명 채움과 테두리만 칠한다.

Glass 샘플에 들어가는 것: `tokens.json`(디자인 시스템 사본), 테마 에셋 2개(밝은 유리·어두운 유리), `UDrGlassSurfaceStyle`, `UDrGlassBackdrop`, 그림자·반사광 머티리얼, 갤러리 화면(모든 컴포넌트와 상태를 한 화면에).

| 층(아래부터) | 언리얼 방법 | 토큰 |
|---|---|---|
| 1. 그림자 | 둥근 사각형 거리장(SDF) 머티리얼을 패널보다 넓게 그린다 | `shadow-glass`, `shadow-float` |
| 2. 흐림 | `UBackgroundBlur`(모서리 반경 설정) | `backdrop-glass` 등 |
| 3. 채움 + 테두리 | `FSlateRoundedBoxBrush`(둥근 모서리 + 1px 테두리) | `glass`, `glass-stroke`, `radius-xl` |
| 4. 윗면 반사광 | 윗변 안쪽 1px 선(머티리얼 또는 얇은 브러시) | `glass-highlight` |
| 5. 내용 | 자식 위젯 | - |

**성능과 품질 단계**

- 배경 흐림은 화면을 다시 그려 비싸다. 한 화면에 흐림 유리는 넓은 것 3개 안쪽을 기준으로 한다.
- 품질 단계 세 가지(`UDrUIKitSettings`, 게임 설정에서 바꿈): **높음**(흐림 + 반사광), **보통**(흐림 강도 절반), **낮음**(흐림 없이 `glass-strong` 불투명 채움). 낮음에서도 글자 대비가 맞도록 채움색을 바꾼다.
- 조밀한 목록·표는 흐림 없이 `glass-strong`만 쓴다(디자인 시스템 원칙 2).

**바탕 색 덩어리**

- 유리는 뒤에 비칠 것이 있어야 보인다. 게임 화면 위 HUD는 게임 화면이 그 역할을 한다.
- 메뉴·설정처럼 뒤가 비어 있는 화면에는 `UDr``Glass``Backdrop`(크게 흐린 색 원 2~3개, `backdrop-a/b/c`)을 깔다.

**확인할 것(1단계 첫 작업)**: UE 5.8에서 배경 흐림의 모서리 처리 품질, 흐림 안에서 둥근 테두리가 자르이는 모양, 1080p에서 패널 3개일 때의 GPU 비용. 결과에 따라 층 구성을 조정한다.

## 무엇을 만들까

uxwoodytips 이름 사전의 패턴을 하나씩 언리얼(UMG·CommonUI)과 대조했다. 후보는 세 표다. **엔진 위젯을 상속해 모양·상태·테마를 더하는 것 17개**(라디오·탭·체크박스처럼 자주 쓰는 것), **엔진에 없어 새로 만드는 것 42개**, **게임에서 자주 쓰는 것 5개**(파이 메뉴 등). 초안은 1차 30개, 2차 22개, 보류 12개다. 우선순위 칸에서 바로 고칠 수 있다.

**엔진 위젯을 상속해 제공하는 것** (엔진은 동작·틀만 있고 모양은 없다. DrUIKit이 모양·상태·테마·움직임을 더한다. 정확한 범위는 1단계 착수 때 UE 5.8 소스로 확인)

| 이름(사전) | 상속할 엔진 위젯 | DrUIKit이 더하는 것 | 우선순위 |
|---|---|---|---|
| 텍스트 | `UCommonTextBlock` | 글자 스타일 토큰(`display`~`caption`) | 1차 |
| 아이콘 | `UImage` | 색 토큰, 크기 단계 | 1차 |
| 버튼 프롬프트(키·패드 버튼 표시) | `UCommonActionWidget` | 테마 모양 | 1차 |
| 체크박스 | `UCommonButtonBase`(토글) | 체크 그리기 움직임, 9상태 | 1차 |
| 라디오(묶음) | `UCommonButtonBase` + `UCommonButtonGroupBase` | 하나만 선택, 패드 이동 | 1차 |
| 탭 | `UCommonTabListWidgetBase`(탭 등록·선택·패드 좌우 이동·화면 전환 연결만, 모양 없음) | 탭 바 모양(알약·밑줄), 탭 버튼, 움직이는 인디케이터 | 1차 |
| 슬라이더 | `USlider` | 값 말풍선, 눈금 | 1차 |
| 셀렉트(드롭다운) | `UComboBoxKey` | 표면 모양 펼침 목록, 선택 표시 | 1차 |
| 진행 바 | `UProgressBar` | 둥근 끝, 부드러운 채움 | 1차 |
| 스피너 | `UCircularThrobber` | 테마 색·크기 | 1차 |
| 툴팁 | 위젯 툴팁 | 표면 모양, 화살표, 지연 | 1차 |
| 목록 행 | `UListView`·`UTileView` 항목 | 호버 렌즈, 선택 상태 | 1차 |
| 스크롤 영역 | `UScrollBox` | 얇은 스크롤 막대, 가장자리 흐림 | 2차 |
| 카운트 업 숫자 | `UCommonNumericTextBlock` | 단위·델타 표시 | 2차 |
| 화면 전환(페이드·슬라이드) | `UCommonAnimatedSwitcher` | 테마 시간·곡선 | 2차 |
| 접히는 영역(아코디언) | `UExpandableArea` | 열기·닫기 움직임 | 2차 |
| 여러 줄 입력 | `UMultiLineEditableTextBox` | 텍스트 필드와 같은 상태 | 2차 |

**게임에서 자주 쓰는 것** (엔진에 없음)

| 이름 | 하는 일 | 우선순위 |
|---|---|---|
| 파이 메뉴(라디얼 메뉴) | 원형으로 항목 배치, 패드 스틱 방향·마우스 각도로 선택(무기 휠, 감정 표현, 빠른 명령) | 1차 |
| 홀드 버튼(길게 눌러 확인) | 누른 채 채워지는 링, 다 차면 실행(파괴적 행동·건너뛰기) | 1차 |
| 쿨다운 표시 | 아이콘 위 원형 마스크 + 남은 시간, 끝나면 반짝임 | 2차 |
| 알림 피드 | 시간이 지나면 사라지는 줄 목록(획득 로그, 이벤트 알림) | 2차 |
| 수평 선택기(◀ 값 ▶) | 설정 화면의 좌우로 바꾸는 값(CommonRotator를 상속해 모양만) | 1차 |

**엔진에 없어 새로 만드는 것**

| 영역 | 이름(사전) | 언리얼 현황 | 우선순위 |
|---|---|---|---|
| 표면 | 표면(패널·카드) | 뒤 흐림·테두리는 있으나 둥근 그림자·테두리·흐림을 묶은 표면은 없음 | 1차 |
| 행동 | 버튼 9상태(로딩·성공·오류 추가) | CommonButton은 기본·호버·눌림·비활성·선택만 | 1차 |
| 행동 | 버튼 모핑(로딩 → 체크) | 없음 | 2차 |
| 입력 | 텍스트 필드 상태(라벨·도움말·인라인 오류) | 입력 칸만 있음 | 1차 |
| 입력 | 스위치(움직이는 토글) | 체크박스 모양만 | 1차 |
| 입력 | 세그먼트 | 없음(버튼 묶음으로 직접) | 1차 |
| 입력 | 수량 스테퍼(−/+) | SpinBox는 끌어서 조절하는 방식 | 2차 |
| 입력 | 칩·필터 칩 | 없음 | 2차 |
| 입력 | 비밀번호 강도 표시기 | 없음 | 보류 |
| 입력 | OTP 입력 | 없음 | 보류 |
| 입력 | 날짜·시간 선택(캘린더, 휠 피커, 기간) | 없음 | 보류 |
| 입력 | 평점(별점, 평점 슬라이더) | 없음 | 보류 |
| 탐색 | 탭 인디케이터(활성 알약이 미끄러짐) | 탭 목록은 있으나 움직이는 표시 없음 | 1차 |
| 탐색 | 사이드바(레일 ↔ 펼침) | 없음 | 2차 |
| 탐색 | 독(아이콘 확대) | 없음 | 2차 |
| 탐색 | 하단 탭바 | 없음(탭 목록으로 조합) | 2차 |
| 탐색 | 드로어 | 없음 | 보류 |
| 탐색 | 브레드크럼 | 없음 | 보류 |
| 탐색 | 페이지 인디케이터(점) | 없음 | 1차 |
| 탐색 | 페이지네이션 | 없음 | 보류 |
| 알림 | 토스트 | 없음 | 1차 |
| 알림 | 스낵바(실행 취소) | 없음 | 1차 |
| 알림 | 배너 | 없음 | 2차 |
| 알림 | 확인 대화상자(모달) | 활성 위젯 스택만 있고 대화상자는 없음 | 1차 |
| 알림 | 바텀시트 | 없음 | 2차 |
| 알림 | 코치마크(튜토리얼 강조) | 없음 | 2차 |
| 알림 | 배지(숫자·점) | 없음 | 1차 |
| 알림 | 팝오버 | MenuAnchor(위치 잡기)만 | 2차 |
| 로딩 | 스켈레톤 | 없음 | 1차 |
| 로딩 | 진행 링(원형 진행률) | 막대만 있음 | 1차 |
| 로딩 | 단계 표시(스텝 인디케이터) | 없음 | 2차 |
| 로딩 | 블러 업 이미지 | LazyImage는 스피너만 | 보류 |
| 완료 | 체크 그리기·완료 표시 | 없음 | 2차 |
| 완료 | 컨페티 | 없음(나이아가라로는 가능) | 보류 |
| 빈 화면 | 빈 화면 템플릿(이유 + 다음 행동) | 없음 | 2차 |
| 움직임 | 글자 등장(타이핑·스크램블·블러 인) | 없음(카운트 업만 있음) | 2차 |
| 움직임 | 스프링·이즈 트윈(다른 부품이 쓰는 도구) | UMG 애니메이션은 곡선만, 스프링 없음 | 1차 |
| 움직임 | 순차 등장 | 없음 | 1차 |
| 움직임 | 공유 요소·카드 확장 전환 | 없음(전환은 페이드·슬라이드만) | 보류 |
| 제스처 | 스와이프·밀어서 삭제·당겨서 새로고침·길게 누르기 | 터치 이벤트만 | 보류 |
| 데이터 | 스파크라인·도넛·통계 카드 | 없음 | 2차 |
| AI 화면 | 생각 중 표시·스트리밍 답변·추천 질문 칩 | 없음 | 보류 |

초안 기준: 게임 메뉴·HUD에서 바로 쓰고 다른 부품이 기대는 것(표면, 버튼 상태, 움직임 도구, 토스트·대화상자)은 1차, 앱형 화면에 필요한 것(사이드바, 독, 차트)은 2차, 게임에서 드문 것(날짜 선택, OTP, 결제형, 제스처, AI 화면)은 보류. 디자인 시스템 컴포넌트 중 OtpField는 보류로 두었다. 고른 것이 확정되면 클래스 이름과 상속·조합 방식을 항목마다 정한다.

## 움직임, 입력, 접근성, 한글

- **움직임**: 코어에 작은 트윈 도구(`DrUIMotion`·`FDrUISpring`·`DrUIMotion::PlayStagger`: 이즈 인·이즈 아웃·스프링, 순차 등장)를 두고 모든 부품이 이것을 쓴다. 시간·강성 값은 테마 토큰이다. 설정의 "움직임 줄이기"가 켜지면 위치 이동을 페이드로 바꾼다.
- **입력**: 마우스·키보드·게임패드를 CommonUI 입력 라우팅과 포커스 탐색으로 처리한다. 모든 부품은 패드만으로 조작할 수 있어야 한다.
- **접근성**: 포커스 링 항상 표시, 글자 대비 4.5:1(테마 검사 도구가 토큰 짝을 검사), 상태는 색과 아이콘·문구를 함께.
- **한글**: 코어는 글꼴을 갖지 않는다. Glass 샘플은 Pretendard(OFL, 무료)를 쓸 계획이다. 글꼴 파일 추가는 다운로드라 승인 후 진행한다. 문구는 게임의 텍스트로 받고 부품은 문구를 갖지 않는다(한·영 모두 게임 책임).

## 검증과 문서

- **자동화 테스트**(`DrUIKit.*`): 상태 전이(버튼 9상태), 테마 전환, 토큰 조회·누락 경고, 트윈 값, 토스트 대기열. 호스트 프로젝트에서 단독으로 돈다(DrFramework의 `Tools/verify.ps1`과 같은 방식).
- **갤러리**: Glass 샘플에 테스트 레벨을 둔다. 레벨을 열면 모든 UI가 배치돼 있고, 탭·페이지로 넘기며 모든 부품과 상태를 직접 조작해 볼 수 있다. 새 UI를 추가하는 작업은 같은 작업에서 이 레벨에도 추가하고, 빠뜨리면 갤러리 테스트(등록되지 않은 DrUIKit 위젯 클래스 검사)가 실패한다. 샘플을 써 보며 나오는 보완점은 기록해 바로 반영한다. `-RenderOffscreen`으로 밝은·어두운 테마를 캡처해 디자인 시스템 미리보기와 나란히 비교한다.
- **문서**: 저장소에 README, 설계 명세서(`Docs/DRUIKIT_FOR_AI.md`, AI 도구가 읽는 기준), 이 기획서(`Docs/PLAN.md`). 구조가 바뀌면 세 문서를 같이 고친다.

## 단계별 일정

지금은 기획 단계다. 날짜는 만들 목록이 확정된 뒤에 정한다.

1. **기획 확정**: 위 목록의 우선순위 확정, 항목별 클래스 이름과 상속·조합 방식 확정, 이 문서를 `Docs/PLAN.md`로 옮김.
2. **뼈대**: 저장소·호스트·모듈, 테마 틀, 표면, 움직임 도구, 버튼, DrFramework에서 옮겨 온 화면 층·메뉴·HUD·UI 흐름(DrFramework 0.3.0). 뒤 흐림 품질·비용 확인.
3. **1차 부품** + Glass 샘플 갤러리 + 테스트.
4. **DrSample 포함**: 서브모듈 추가, 메뉴·HUD를 DrUIKit으로 교체, 패키지 스모크.
5. **GitHub 공개**, 이후 2차 부품.

## 결정할 것과 위험

**결정할 것**

- 만들 목록과 우선순위(위 표의 드롭다운)
- Glass 샘플을 플러그인(`DrUIGlass`)으로 둘지, 호스트 프로젝트의 콘텐츠로 둘지
- 테마 토큰 원본: 디자인 시스템 저장소를 서브모듈로 둘지, `tokens.json`만 복사해 둘지
- Pretendard 글꼴 포함(다운로드 승인)
- 정적 데이터가 필요한 부품(예: 코치마크 단계)에 DrTableSystem을 쓸지, 게임에 맡길지

**위험**

- 뒤 흐림 비용과 둥근 모서리 품질: 뼈대 단계에서 먼저 측정하고, 안 되면 Glass 샘플에서 흐림 없는 표면으로 대체한다.
- CommonUI 버전 변화: 엔진이 올라갈 때 상속 부품이 깨질 수 있다. 상속은 공개 API만 쓴다.
- 범위 팔창: 후보가 42개다. 1차를 끝내기 전에는 2차를 시작하지 않는다.
