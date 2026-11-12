# DrUIKit

언리얼 엔진 5.8용 **스타일 중립 UI 부품** 플러그인이다. 엔진·CommonUI 위젯을 상속해 확장하고, 상속으로 안 되는 것은 UserWidget 안에서 조합한다. 모양은 테마 토큰으로만 칠하고, 실제 디자인은 스타일 샘플(`UISample/<스타일>`)이 입힌다.

> 상태: 0.0.2(뼈대 + 1차 부품 진행 중). 기획: `Docs/PLAN.md`.

## 구성

| 위치 | 역할 |
| --- | --- |
| `unreal/DrUIKit` | 코어 플러그인(모듈 `DrUIKit`, 의존: UMG·CommonUI) |
| `Host/DrUIKitHost.uproject` | 단독 빌드·테스트 호스트(코드 모듈 없음) |
| `UISample/Glass` | 리퀴드 글래스 스타일 샘플(플러그인 `DrUIGlass`: tokens.json 테마, Pretendard 글꼴, 바탕, 갤러리 테스트 레벨 `L_DrUIGallery`) |
| `Tools/verify.ps1` | 호스트 빌드 + `Automation RunTests DrUIKit+DrUIGlass` |

## 지금 있는 것

| 영역 | 클래스 |
| --- | --- |
| 테마 | `UDrUITheme`(토큰 묶음 데이터 에셋, 회색조 기본 테마), `UDrUIThemeSubsystem`(현재 테마·전환·표면 품질·표면 투명도·움직임 줄이기), `UDrUIKitSettings` |
| 상태·움직임 | `EDrUIState`·`DrUIState::Resolve`(9상태 우선순위), `DrUIMotion`(이즈 인·아웃 cubic-bezier, 순차 등장), `FDrUISpring` |
| 기초 부품 | `UDrUIText`(UCommonTextBlock), `UDrUIIcon`(UImage), `UDrUIGlyph`(선으로 그리는 표시: 체크·점·더하기·닫기·화살표), `UDrUISurface`(UBorder + 뒤 흐림, 채움 토큰이 비면 투명) |
| 누르는 부품 | `UDrUIButtonBase`(공통 상태 판정 틀, 추상), `UDrUIButton`(9상태·변형 4종) |
| 선택 부품 | `UDrUICheckBox`(체크 그리기), `UDrUIRadio`(+ `UCommonButtonGroupBase`로 하나만 선택), `UDrUISwitch`(스프링 손잡이), `UDrUITabList`·`UDrUITabButton`(UCommonTabListWidgetBase, 알약·밑줄 인디케이터가 미끄러짐), `UDrUISegmented`(값 하나 고르기), `UDrUISlider`(USlider + 채움 트랙) |

## 갤러리

`UISample/Glass`의 `L_DrUIGallery`(호스트 기본 맵)를 열면 모든 부품이 탭별 페이지에 놓여 있다. 새 부품은 같은 작업에서 갤러리 페이지와 `UDrGlassGallery::CoveredClasses()`에 넣는다(빠뜨리면 `DrUIGlass.Gallery.CoversAllKitWidgets` 실패). `-game -DrUIGalleryCapture -RenderOffscreen`으로 실행하면 테마 × 페이지마다 `Saved/Screenshots`에 찍고 끝난다.

## 검증

```powershell
.\Tools\verify.ps1      # 기본 엔진 경로: C:\Program Files\Epic Games\UE_5.8
```

## 라이선스

MIT
