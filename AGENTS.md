# DrUIKit 작업 규칙 (사람·AI 도구 공통)

- 코어(`unreal/DrUIKit`)는 **코드 레벨·스타일 중립**이다. 색·크기·모서리 값을 코드에 쓰지 않고 테마 토큰 이름(디자인 시스템과 같은 이름)으로만 꺼낸다. 콘텐츠 에셋을 두지 않는다.
- **엔진: UE 5.9 기준, 5.8 하위호환 유지.** 5.8에 없는 API는 `#include "Misc/EngineVersionComparison.h"` 후 `#if UE_VERSION_NEWER_THAN_OR_EQUAL(5, 9, 0)`로 나누고, 5.9에서 지원 중단된 API는 5.8에도 있는 대체 API로 바꾼다. 빌드·테스트는 5.9로 하고, 5.8 엔진이 있으면 5.8로도 확인한다. **에셋(.uasset·.umap)은 5.8 에디터로 저장한다** — 5.9에서 다시 저장한 에셋은 5.8에서 열리지 않는다. 5.9 에디터·커맨드릿으로 에셋을 만들거나 고쳐야 하면 먼저 사용자와 상의한다.
- 부품은 ① 엔진·CommonUI 위젯 상속 ② `UUserWidget` 안 조합 ③ Slate 직접 그리기 순으로 고른다. 하위 부품은 `BindWidgetOptional`, 없으면 C++가 기본 구성을 만든다.
- 이름: 부품(위젯)은 `UDr<부품>`(`UDrButton`, `UDrToast` …, 딸린 열거형·Slate 클래스도 `EDr…`·`SDr…`), 부품이 아닌 기반 시스템은 `UDrUI<…>`(`UDrUITheme`, `UDrUIThemeSubsystem`, `FDrUISpring`, `EDrUIState` …), 설정 `UDrUIKitSettings`, 테스트 `DrUIKit.<영역>.<항목>`. 파일 이름은 클래스 이름에서 접두사를 뺀 것. 새 이름은 DrFramework·DrTableSystem의 클래스 이름과 겹치지 않게 확인한다.
- 모든 부품은 상태(기본·호버·눌림·포커스·비활성·로딩·성공·오류·선택됨 중 해당하는 것)를 다 가진다. 상태 판정은 `DrUIState::Resolve`.
- **스타일 샘플(`UISample/<스타일>`)의 테스트 레벨(갤러리)은 모든 UI를 담는다. 새 UI를 추가하는 작업은 같은 작업에서 갤러리에 그 UI(모든 상태)를 추가한다. 빠뜨리면 갤러리 테스트가 실패한다.** 갤러리는 탭·페이지로 나눠 넘기며 직접 조작해 볼 수 있어야 한다.
- 구조가 바뀌면 같은 작업에서 README와 `Docs/PLAN.md`(구현 현황 절 포함)를 고친다. 기획·결정의 원본은 `Docs/PLAN.md`(git)다. 디자인 언어(토큰·이름 사전)의 원본은 저장소 밖에서 관리하고, `UISample/Glass/DrUIGlass/Tokens/tokens.json`은 그 복사본이다(바뀌면 같이 복사).
- 스타일은 `UISample/<스타일>/DrUI<스타일>` 플러그인에 `Tokens/tokens.json`(첫 스타일 Glass와 같은 토큰 이름을 모두)과 `Tokens/style.json`을 둔다. 코어 `DrUIStyle`이 자동으로 찾고, `DrUIKit.Style.LoadedStylesComplete`가 빠진 토큰을 잡는다. 호스트 `.uproject`에도 켠다.
- 모든 부품은 패드·키보드만으로 조작할 수 있어야 한다. 확인·뒤로 판정은 `DrUIInput::GetAction`(키를 직접 비교하지 않음), 덮개는 열 때 `DrUIInput::PushModal`·포커스 기억, 닫을 때 `PopModal`·복원. 키 표시는 `UDrKeyGlyph`/`UDrButtonPrompt`(글자 키캡을 따로 만들지 않음). 목록·뎁스 이동은 `NavTargets`/`DrUIInput::ChainVertical`로 정하고(엔진 탐색 규칙은 CommonUI 버튼에 안 통함), 마우스(뎁스 없이 바로)와 패드·키보드(포커스 차례)를 `DrUIInput::GetLastInputKind`로 나눈다. 바꾼 뒤에는 `-DrUIGamepadTour`로 확인한다.
- 검증: `Tools/verify.ps1`(호스트 빌드 + `DrUIKit.*` 테스트). 빌드·테스트는 Windows(UE 5.9, 5.8 호환 유지).
