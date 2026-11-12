# DrUIKit

언리얼 엔진 5.8용 **스타일 중립 UI 부품** 플러그인이다. 엔진·CommonUI 위젯을 상속해 확장하고, 상속으로 안 되는 것은 UserWidget 안에서 조합한다. 모양은 테마 토큰으로만 칠하고, 실제 디자인은 스타일 샘플(`UISample/<스타일>`)이 입힌다.

> 상태: 0.0.1 뼈대(기획 단계에서 시작). 기획: `Docs/PLAN.md`.

## 구성

| 위치 | 역할 |
| --- | --- |
| `unreal/DrUIKit` | 코어 플러그인(모듈 `DrUIKit`, 의존: UMG·CommonUI) |
| `Host/DrUIKitHost.uproject` | 단독 빌드·테스트 호스트(코드 모듈 없음) |
| `UISample/Glass` | 리퀴드 글래스 스타일 샘플(테마·표면 스타일·갤러리 테스트 레벨) — 준비 중 |
| `Tools/verify.ps1` | 호스트 빌드 + `Automation RunTests DrUIKit` |

## 지금 있는 것(0.0.1)

| 영역 | 클래스 |
| --- | --- |
| 테마 | `UDrUITheme`(토큰 묶음 데이터 에셋, 회색조 기본 테마), `UDrUIThemeSubsystem`(현재 테마·전환·표면 품질·움직임 줄이기), `UDrUIKitSettings` |
| 상태·움직임 | `EDrUIState`·`DrUIState::Resolve`(9상태 우선순위), `DrUIMotion`(이즈 인·아웃 cubic-bezier, 순차 등장), `FDrUISpring` |
| 부품 | `UDrUIText`(UCommonTextBlock), `UDrUIIcon`(UImage), `UDrUISurface`(UBorder + 뒤 흐림), `UDrUIButton`(UCommonButtonBase, 9상태·변형 4종) |

## 검증

```powershell
.\Tools\verify.ps1      # 기본 엔진 경로: C:\Program Files\Epic Games\UE_5.8
```

## 라이선스

MIT
