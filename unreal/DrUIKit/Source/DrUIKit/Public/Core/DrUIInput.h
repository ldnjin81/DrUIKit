#pragma once
#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "DrUIInput.generated.h"

class SWidget;
class UDrButtonBase;
class UDrTabList;
class UWidget;
struct FKeyEvent;

/** UI가 알아듣는 입력 뜻(키 자체가 아니라). 확인·취소는 엔진 내비게이션 설정을 따라 플랫폼별 교체가 반영된다. */
UENUM(BlueprintType)
enum class EDrUIInputAction : uint8 {
  None,
  /** 확인(패드 A, Enter, Space). 일본·닌텐도 배치에서는 엔진 설정대로 바뀐다. */
  Accept,
  /** 뒤로·취소(패드 B, Esc). */
  Back,
  /** 이전·다음 탭(LB·RB, 키보드 Q·E). */
  PrevTab,
  NextTab,
  /** 이전·다음 하위 탭(LT·RT, 키보드 Z·C). */
  PrevSubTab,
  NextSubTab
};

/** 패드 버튼을 어느 표기로 보일지. 자동이면 CommonInput의 현재 패드 이름을 따른다(모르면 Xbox). */
UENUM(BlueprintType)
enum class EDrGamepadGlyphs : uint8 { Auto, Xbox, PlayStation, Nintendo };

/**
 * 게임패드 UX 공통 도구.
 * - 입력 뜻 판정(GetAction): 부품마다 "B면 닫기"를 따로 쓰지 않는다.
 * - 패드 종류별 버튼 이름(GlyphName): Xbox A·B·LB, PlayStation ✕·○·L1, Nintendo B·A·L.
 * - 덮개 스택(PushModal): 대화상자·팝오버·시트·드롭다운·코치마크가 열려 있으면 그 안의 버튼 단축키만 듣는다.
 * - 포커스 기억(FDrUIFocusMemory): 덮개를 닫으면 연 곳으로 포커스를 돌려준다.
 * 전역 입력 처리기(코어 모듈이 등록)는 오른쪽 스틱 스크롤, LB·RB 탭 넘김, 버튼 단축키, 아무 버튼 대기를 맡는다.
 */
namespace DrUIInput {
DRUIKIT_API EDrUIInputAction GetAction(const FKeyEvent& Event);
/** 키만으로 판정(Accept·Back은 엔진 기본 배치 기준). */
DRUIKIT_API EDrUIInputAction GetActionForKey(const FKey& Key);
/** 지금 보일 패드 표기(설정 → 자동이면 CommonInput 패드 이름). Context는 로컬 플레이어를 찾는 데 쓴다. */
DRUIKIT_API EDrGamepadGlyphs ResolveGlyphs(const UWidget* Context);
/** 패드 이름(CommonInput)으로 표기 고르기: PS·DualSense → PlayStation, Switch·Joy → Nintendo, 그 밖 Xbox. */
DRUIKIT_API EDrGamepadGlyphs GlyphsForGamepadName(FName GamepadName);
/** 키캡 글자. 패드 키는 표기별, 키보드는 짧은 이름(Esc·Enter …), 모르면 엔진 표시 이름. */
DRUIKIT_API FText GlyphName(const FKey& Key, EDrGamepadGlyphs Glyphs);
/** 지금 패드로 조작 중인지(CommonInput 마지막 입력). */
DRUIKIT_API bool IsUsingGamepad(const UWidget* Context);

/** 덮개(모달)를 연다/닫는다. 열린 덮개가 있으면 단축키는 맨 위 덮개 안의 버튼만 받는다. */
DRUIKIT_API void PushModal(UWidget* Modal);
DRUIKIT_API void PopModal(UWidget* Modal);
DRUIKIT_API UWidget* GetTopModal();
/** Widget이 지금 단축키를 받을 수 있는 층에 있는지(덮개가 없거나 맨 위 덮개 안). */
DRUIKIT_API bool IsInActiveLayer(const UWidget* Widget);
/** Root 아래(위젯 트리 순서)에서 처음 만나는 보이고 켜진 포커스 받을 수 있는 UMG 위젯(스위처는 보이는 칸만). 없으면 nullptr. */
DRUIKIT_API UWidget* FindFirstFocusable(UWidget* Root);
/** Container 안에서 처음 만나는 포커스 받을 수 있는 위젯에 포커스(덮개가 열릴 때 패드가 바로 안에서 움직이게). 했으면 true. */
DRUIKIT_API bool FocusFirstIn(UWidget* Container, int32 UserIndex = 0);
/** Widget이 지금 화면에 보이는 경로 안에 있는지(조상이 접혀 있거나 창에 없으면 false). */
DRUIKIT_API bool IsShown(const UWidget* Widget);

/** 단축키가 있는 버튼(UDrButtonBase::ShortcutGamepadKey 등)을 전역 처리기에 알린다. */
DRUIKIT_API void RegisterShortcutButton(UDrButtonBase* Button);
DRUIKIT_API void UnregisterShortcutButton(UDrButtonBase* Button);
/** LB·RB(LT·RT)로 넘길 탭 목록. */
DRUIKIT_API void RegisterTabList(UDrTabList* TabList);
DRUIKIT_API void UnregisterTabList(UDrTabList* TabList);
/** 아무 입력(키·패드 버튼·마우스 버튼)이 들어오면 부른다. true를 돌려주면 그 입력은 다른 UI로 가지 않는다. 반환값은 떼기용 번호. */
DRUIKIT_API int32 AddAnyKeyListener(TFunction<bool(const FKey& Key, bool bGamepad)> Listener);
DRUIKIT_API void RemoveAnyKeyListener(int32 Id);
/** 오른쪽 스틱 세로 값으로 스크롤할 속도(px/초, 기울기 1일 때). */
inline constexpr float StickScrollSpeed = 1400.f;
}

/** 덮개가 열릴 때 포커스를 기억했다가 닫을 때 되돌린다. 기억한 위젯이 사라졌으면 아무것도 하지 않는다. */
struct DRUIKIT_API FDrUIFocusMemory {
  void Save(int32 UserIndex = 0);
  /** 되돌렸으면 true. */
  bool Restore();
  bool IsSaved() const { return Saved.IsValid(); }
private:
  TWeakPtr<SWidget> Saved;
  int32 User = 0;
};

/** 코어 모듈 시작·끝에서 전역 입력 처리기를 붙이고 뗀다. */
namespace DrUIInputProcessor {
void Register();
void Unregister();
}
