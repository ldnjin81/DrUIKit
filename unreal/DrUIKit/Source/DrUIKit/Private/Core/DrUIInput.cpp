#include "Core/DrUIInput.h"
#include "DrUIKit.h"
#include "CommonInputSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/PanelWidget.h"
#include "Components/Widget.h"
#include "Components/WidgetSwitcher.h"
#include "Framework/Application/IInputProcessor.h"
#include "Framework/Application/SlateApplication.h"
#include "Input/Events.h"
#include "Layout/WidgetPath.h"
#include "Theme/DrUIKitSettings.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Types/ISlateMetaData.h"
#include "Core/DrUIFeedback.h"
#include "Widgets/DrButtonBase.h"
#include "Widgets/DrActionBar.h"
#include "Widgets/DrOptionRow.h"
#include "Widgets/DrPressAnyKey.h"
#include "Widgets/DrTabList.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"

EDrUIInputAction DrUIInput::GetActionForKey(const FKey& Key) {
  if (Key == EKeys::Gamepad_FaceButton_Bottom || Key == EKeys::Enter || Key == EKeys::SpaceBar || Key == EKeys::Virtual_Gamepad_Accept.GetVirtualKey()) return EDrUIInputAction::Accept;
  if (Key == EKeys::Gamepad_FaceButton_Right || Key == EKeys::Escape || Key == EKeys::Virtual_Gamepad_Back.GetVirtualKey()) return EDrUIInputAction::Back;
  if (Key == EKeys::Gamepad_LeftShoulder || Key == EKeys::Q) return EDrUIInputAction::PrevTab;
  if (Key == EKeys::Gamepad_RightShoulder || Key == EKeys::E) return EDrUIInputAction::NextTab;
  if (Key == EKeys::Gamepad_LeftTrigger || Key == EKeys::Z) return EDrUIInputAction::PrevSubTab;
  if (Key == EKeys::Gamepad_RightTrigger || Key == EKeys::C) return EDrUIInputAction::NextSubTab;
  return EDrUIInputAction::None;
}

EDrUIInputAction DrUIInput::GetAction(const FKeyEvent& Event) {
  // 확인·취소는 엔진 내비게이션 설정이 정한다(플랫폼이 확인·취소 버튼을 바꾸면 따라간다).
  if (FSlateApplication::IsInitialized()) {
    const EUINavigationAction Navigation = FSlateApplication::Get().GetNavigationActionFromKey(Event);
    if (Navigation == EUINavigationAction::Accept) return EDrUIInputAction::Accept;
    if (Navigation == EUINavigationAction::Back) return EDrUIInputAction::Back;
  }
  const EDrUIInputAction Action = GetActionForKey(Event.GetKey());
  return Action == EDrUIInputAction::Accept || Action == EDrUIInputAction::Back ? EDrUIInputAction::None : Action;
}

static UCommonInputSubsystem* InputFor(const UWidget* Context) {
  if (Context) if (const ULocalPlayer* Player = Context->GetOwningLocalPlayer()) return UCommonInputSubsystem::Get(Player);
  if (GEngine && GEngine->GameViewport) if (UWorld* World = GEngine->GameViewport->GetWorld()) if (ULocalPlayer* Player = World->GetFirstLocalPlayerFromController()) return UCommonInputSubsystem::Get(Player);
  return nullptr;
}

bool DrUIInput::IsUsingGamepad(const UWidget* Context) {
  const UCommonInputSubsystem* Input = InputFor(Context);
  return Input && Input->GetCurrentInputType() == ECommonInputType::Gamepad;
}

EDrUIGamepadGlyphs DrUIInput::GlyphsForGamepadName(FName GamepadName) {
  const FString Name = GamepadName.ToString();
  if (Name.Contains(TEXT("PS")) || Name.Contains(TEXT("DualSense")) || Name.Contains(TEXT("DualShock")) || Name.Contains(TEXT("PlayStation"))) return EDrUIGamepadGlyphs::PlayStation;
  if (Name.Contains(TEXT("Switch")) || Name.Contains(TEXT("Joy")) || Name.Contains(TEXT("Nintendo"))) return EDrUIGamepadGlyphs::Nintendo;
  return EDrUIGamepadGlyphs::Xbox;
}

EDrUIGamepadGlyphs DrUIInput::ResolveGlyphs(const UWidget* Context) {
  const UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  const EDrUIGamepadGlyphs Setting = Subsystem ? Subsystem->GetGamepadGlyphs() : GetDefault<UDrUIKitSettings>()->GamepadGlyphs;
  if (Setting != EDrUIGamepadGlyphs::Auto) return Setting;
  const UCommonInputSubsystem* Input = InputFor(Context);
  return Input ? GlyphsForGamepadName(Input->GetCurrentGamepadName()) : EDrUIGamepadGlyphs::Xbox;
}

FText DrUIInput::GlyphName(const FKey& Key, EDrUIGamepadGlyphs Glyphs) {
  // 얼굴 버튼은 자리(아래·오른쪽·왼쪽·위)로 이름이 정해진다. 닌텐도는 아래가 B, 오른쪽이 A.
  struct FNames { const TCHAR* Xbox; const TCHAR* PlayStation; const TCHAR* Nintendo; };
  static const TMap<FKey, FNames> Pad = {
    {EKeys::Gamepad_FaceButton_Bottom, {TEXT("A"), TEXT("✕"), TEXT("B")}}, {EKeys::Gamepad_FaceButton_Right, {TEXT("B"), TEXT("○"), TEXT("A")}},
    {EKeys::Gamepad_FaceButton_Left, {TEXT("X"), TEXT("□"), TEXT("Y")}}, {EKeys::Gamepad_FaceButton_Top, {TEXT("Y"), TEXT("△"), TEXT("X")}},
    {EKeys::Gamepad_LeftShoulder, {TEXT("LB"), TEXT("L1"), TEXT("L")}}, {EKeys::Gamepad_RightShoulder, {TEXT("RB"), TEXT("R1"), TEXT("R")}},
    {EKeys::Gamepad_LeftTrigger, {TEXT("LT"), TEXT("L2"), TEXT("ZL")}}, {EKeys::Gamepad_RightTrigger, {TEXT("RT"), TEXT("R2"), TEXT("ZR")}},
    {EKeys::Gamepad_LeftThumbstick, {TEXT("LS"), TEXT("L3"), TEXT("LS")}}, {EKeys::Gamepad_RightThumbstick, {TEXT("RS"), TEXT("R3"), TEXT("RS")}},
    {EKeys::Gamepad_Special_Left, {TEXT("View"), TEXT("Create"), TEXT("−")}}, {EKeys::Gamepad_Special_Right, {TEXT("Menu"), TEXT("Options"), TEXT("+")}},
    {EKeys::Gamepad_DPad_Up, {TEXT("↑"), TEXT("↑"), TEXT("↑")}}, {EKeys::Gamepad_DPad_Down, {TEXT("↓"), TEXT("↓"), TEXT("↓")}},
    {EKeys::Gamepad_DPad_Left, {TEXT("←"), TEXT("←"), TEXT("←")}}, {EKeys::Gamepad_DPad_Right, {TEXT("→"), TEXT("→"), TEXT("→")}},
    {EKeys::Gamepad_LeftX, {TEXT("L←→"), TEXT("L←→"), TEXT("L←→")}}, {EKeys::Gamepad_LeftY, {TEXT("L↑↓"), TEXT("L↑↓"), TEXT("L↑↓")}},
    {EKeys::Gamepad_RightX, {TEXT("R←→"), TEXT("R←→"), TEXT("R←→")}}, {EKeys::Gamepad_RightY, {TEXT("R↑↓"), TEXT("R↑↓"), TEXT("R↑↓")}}};
  if (const FNames* Found = Pad.Find(Key)) {
    const TCHAR* Name = Glyphs == EDrUIGamepadGlyphs::PlayStation ? Found->PlayStation : Glyphs == EDrUIGamepadGlyphs::Nintendo ? Found->Nintendo : Found->Xbox;
    return FText::FromString(Name);
  }
  static const TMap<FKey, const TCHAR*> Keyboard = {
    {EKeys::Escape, TEXT("Esc")}, {EKeys::Enter, TEXT("Enter")}, {EKeys::SpaceBar, TEXT("Space")}, {EKeys::Tab, TEXT("Tab")},
    {EKeys::BackSpace, TEXT("Back")}, {EKeys::LeftShift, TEXT("Shift")}, {EKeys::LeftControl, TEXT("Ctrl")}, {EKeys::LeftAlt, TEXT("Alt")},
    {EKeys::Up, TEXT("↑")}, {EKeys::Down, TEXT("↓")}, {EKeys::Left, TEXT("←")}, {EKeys::Right, TEXT("→")},
    {EKeys::LeftMouseButton, TEXT("LMB")}, {EKeys::RightMouseButton, TEXT("RMB")}, {EKeys::MiddleMouseButton, TEXT("MMB")}};
  if (const TCHAR* const* Found = Keyboard.Find(Key)) return FText::FromString(*Found);
  return Key.IsValid() ? Key.GetDisplayName(false) : FText::GetEmpty();
}

namespace {
EDrUIInputKind GLastInputKind = EDrUIInputKind::Pointer;
TArray<TWeakObjectPtr<UWidget>> GModals;
TArray<TWeakObjectPtr<UDrButtonBase>> GShortcutButtons;
TArray<TWeakObjectPtr<UDrTabList>> GTabLists;
TMap<int32, TFunction<bool(const FKey&, bool)>> GAnyKeyListeners;
int32 GNextListenerId = 1;

bool IsInside(const UWidget* Widget, const UWidget* Container) {
  // 위젯 블루프린트 안 위젯은 Outer가 WidgetTree → 그 위젯, 패널 안 위젯은 부모 패널을 따라 올라간다.
  for (const UObject* It = Widget; It; ) {
    if (It == Container) return true;
    if (const UWidget* AsWidget = Cast<UWidget>(It); AsWidget && AsWidget->GetParent()) It = AsWidget->GetParent();
    else It = It->GetOuter();
  }
  return false;
}
}

EDrUIInputKind DrUIInput::GetLastInputKind() { return GLastInputKind; }

void DrUIInput::PushModal(UWidget* Modal) {
  if (!Modal) return;
  GModals.Remove(Modal);
  GModals.Add(Modal);
}

void DrUIInput::PopModal(UWidget* Modal) { GModals.Remove(Modal); }

UWidget* DrUIInput::GetTopModal() {
  GModals.RemoveAll([](const TWeakObjectPtr<UWidget>& W) { return !W.IsValid(); });
  // 팝업(드롭다운 목록·팝오버)은 별도 창이라 '보이는 경로' 검사로는 잡히지 않는다. 덮개는 닫을 때 반드시 빼므로 등록 순서만 본다.
  return GModals.IsEmpty() ? nullptr : GModals.Last().Get();
}

bool DrUIInput::IsInActiveLayer(const UWidget* Widget) {
  const UWidget* Top = GetTopModal();
  return !Top || IsInside(Widget, Top);
}

bool DrUIInput::IsShown(const UWidget* Widget) {
  if (!Widget || !FSlateApplication::IsInitialized()) return false;
  const TSharedPtr<SWidget> Slate = Widget->GetCachedWidget();
  if (!Slate) return false;
  FWidgetPath Path;
  return FSlateApplication::Get().FindPathToWidget(Slate.ToSharedRef(), Path, EVisibility::Visible);
}

static TSharedPtr<SWidget> FirstFocusable(const TSharedRef<SWidget>& Widget) {
  if (!Widget->GetVisibility().IsVisible()) return nullptr;
  if (Widget->SupportsKeyboardFocus()) return Widget;
  FChildren* Children = Widget->GetChildren();
  for (int32 I = 0; Children && I < Children->Num(); ++I)
    if (TSharedPtr<SWidget> Found = FirstFocusable(Children->GetChildAt(I))) return Found;
  return nullptr;
}

UWidget* DrUIInput::FindFirstFocusable(UWidget* Root) {
  if (!Root || !Root->IsVisible() || !Root->GetIsEnabled()) return nullptr;
  if (UUserWidget* User = Cast<UUserWidget>(Root)) {
    if (User->IsFocusable()) return User;
    return User->WidgetTree ? FindFirstFocusable(User->WidgetTree->RootWidget) : nullptr;
  }
  if (UWidgetSwitcher* Switcher = Cast<UWidgetSwitcher>(Root)) return FindFirstFocusable(Switcher->GetActiveWidget());
  if (UPanelWidget* Panel = Cast<UPanelWidget>(Root)) {
    for (int32 I = 0; I < Panel->GetChildrenCount(); ++I) if (UWidget* Found = FindFirstFocusable(Panel->GetChildAt(I))) return Found;
    return nullptr;
  }
  // 슬라이더·입력 칸 등 UMG 기본 위젯은 Slate가 포커스를 받는지로 판단한다(겉을 감싼 위젯이면 그 안까지).
  const TSharedPtr<SWidget> Slate = Root->GetCachedWidget();
  return Slate && FirstFocusable(Slate.ToSharedRef()) ? Root : nullptr;
}

bool DrUIInput::FocusWidget(UWidget* Widget, bool bByPointer) {
  // UMG 위젯을 찾은 뒤 그 안에서 실제로 포커스를 받는 Slate 위젯에 준다(CommonUI 버튼은 안쪽 버튼, DrUIKit 슬라이더는 감싼 오버레이 안 SSlider).
  UWidget* Target = FindFirstFocusable(Widget);
  const TSharedPtr<SWidget> Slate = Target ? Target->GetCachedWidget() : nullptr;
  const TSharedPtr<SWidget> Focusable = Slate ? FirstFocusable(Slate.ToSharedRef()) : nullptr;
  // 마우스로 옮긴 포커스면 원인을 Mouse로(포커스 링을 숨긴다).
  return Focusable && FSlateApplication::Get().SetUserFocus(0, Focusable, bByPointer ? EFocusCause::Mouse : EFocusCause::Navigation);
}

bool FDrUINavTargets::Handle(UWidget* Self, EUINavigation Direction, FNavigationReply& OutReply) const {
  const TFunction<UWidget*()>* Target = Targets.Find(Direction);
  if (!Target || !*Target) return false;
  // 위에 덮개(같은 창에 그려진 드롭다운 목록 등)가 떠 있으면 그 안의 이동이 여기까지 올라온다. 아래 층은 손대지 않는다.
  if (!DrUIInput::IsInActiveLayer(Self)) return false;
  UWidget* To = (*Target)();
  if (!To) return false;  // 엔진 기본 이동
  if (To != Self && DrUIInput::FocusWidget(To)) DrUIFeedback::Play(Self, EDrUIFeedback::Focus);
  OutReply = FNavigationReply::Stop();
  return true;
}

void DrUIInput::ChainVertical(const TArray<UWidget*>& Items, bool bWrap) {
  for (int32 I = 0; I < Items.Num(); ++I) {
    FDrUINavTargets* Targets = nullptr;
    if (UDrButtonBase* Button = Cast<UDrButtonBase>(Items[I])) Targets = &Button->NavTargets;
    else if (UDrOptionRow* Row = Cast<UDrOptionRow>(Items[I])) Targets = &Row->NavTargets;
    if (!Targets) continue;
    const int32 Up = I > 0 ? I - 1 : bWrap ? Items.Num() - 1 : I;
    const int32 Down = I + 1 < Items.Num() ? I + 1 : bWrap ? 0 : I;
    TWeakObjectPtr<UWidget> UpWidget = Items[Up], DownWidget = Items[Down];
    Targets->Set(EUINavigation::Up, [UpWidget] { return UpWidget.Get(); });
    Targets->Set(EUINavigation::Down, [DownWidget] { return DownWidget.Get(); });
  }
}

bool DrUIInput::FocusFirstIn(UWidget* Container, int32 UserIndex) {
  if (!Container || !FSlateApplication::IsInitialized()) return false;
  const TSharedPtr<SWidget> Slate = Container->GetCachedWidget();
  const TSharedPtr<SWidget> Target = Slate ? FirstFocusable(Slate.ToSharedRef()) : nullptr;
  return Target && FSlateApplication::Get().SetUserFocus(UserIndex, Target, EFocusCause::Navigation);
}

void DrUIInput::RegisterShortcutButton(UDrButtonBase* Button) { GShortcutButtons.AddUnique(Button); }
void DrUIInput::UnregisterShortcutButton(UDrButtonBase* Button) { GShortcutButtons.Remove(Button); }
void DrUIInput::RegisterTabList(UDrTabList* TabList) { GTabLists.AddUnique(TabList); }
void DrUIInput::UnregisterTabList(UDrTabList* TabList) { GTabLists.Remove(TabList); }

int32 DrUIInput::AddAnyKeyListener(TFunction<bool(const FKey&, bool)> Listener) {
  const int32 Id = GNextListenerId++;
  GAnyKeyListeners.Add(Id, MoveTemp(Listener));
  return Id;
}

void DrUIInput::RemoveAnyKeyListener(int32 Id) { GAnyKeyListeners.Remove(Id); }

void FDrUIFocusMemory::Save(int32 UserIndex) {
  User = UserIndex;
  Saved = FSlateApplication::IsInitialized() ? FSlateApplication::Get().GetUserFocusedWidget(UserIndex) : nullptr;
}

bool FDrUIFocusMemory::Restore() {
  const TSharedPtr<SWidget> Target = Saved.Pin();
  Saved.Reset();
  if (!Target || !FSlateApplication::IsInitialized()) return false;
  FWidgetPath Path;
  if (!FSlateApplication::Get().FindPathToWidget(Target.ToSharedRef(), Path, EVisibility::Visible)) return false;
  return FSlateApplication::Get().SetUserFocus(User, Path, EFocusCause::SetDirectly);
}

/**
 * 전역 UI 입력 처리기(슬레이트 입력 전처리기). 포커스가 어디에 있든 받아야 하는 게임패드 조작을 맡는다.
 * 순서: 아무 버튼 대기 → 버튼 단축키(맨 위 층만) → LB·RB / LT·RT 탭 넘김. 오른쪽 스틱은 포커스가 든 DrUIKit 스크롤 상자를 굴린다.
 */
class FDrUIInputProcessor : public IInputProcessor {
public:
  virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override {
    if (FMath::Abs(RightStickY) < .2f) return;
    // 포커스에서 위로 올라가며 DrUIKit 스크롤 상자(태그)를 찾는다.
    for (TSharedPtr<SWidget> It = SlateApp.GetUserFocusedWidget(0); It; It = It->GetParentWidget()) {
      const TSharedPtr<FTagMetaData> Tag = It->GetMetaData<FTagMetaData>();
      if (!Tag || Tag->Tag != TEXT("DrScrollBox")) continue;
      SScrollBox& Box = static_cast<SScrollBox&>(*It);
      const float Offset = FMath::Clamp(Box.GetScrollOffset() - RightStickY * DrUIInput::StickScrollSpeed * DeltaTime, 0.f, Box.GetScrollOffsetOfEnd());
      Box.SetScrollOffset(Offset);
      return;
    }
  }

  virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& Event) override {
    if (Event.GetKey() == EKeys::Gamepad_RightY) RightStickY = Event.GetAnalogValue();
    if (Event.GetKey().IsGamepadKey() && FMath::Abs(Event.GetAnalogValue()) > .3f) GLastInputKind = EDrUIInputKind::Gamepad;
    return false;
  }

  virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& Event) override {
    if (Event.GetCursorDelta().SizeSquared() > 4.f) GLastInputKind = EDrUIInputKind::Pointer;
    return false;
  }

  virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& Event) override {
    const FKey Key = Event.GetKey();
    if (!Key.IsAnalog()) GLastInputKind = Key.IsGamepadKey() ? EDrUIInputKind::Gamepad : EDrUIInputKind::Keyboard;
    if (Event.IsRepeat()) return false;
    if (!GAnyKeyListeners.IsEmpty() && !Key.IsAnalog()) {
      bool bConsumed = false;
      TArray<TFunction<bool(const FKey&, bool)>> Listeners;
      GAnyKeyListeners.GenerateValueArray(Listeners);
      for (const auto& Listener : Listeners) bConsumed |= Listener(Key, Key.IsGamepadKey());
      if (bConsumed) return true;
    }
    // 글 입력 중에는 키보드 단축키(Q·E 등)를 가로채지 않는다.
    if (!Key.IsGamepadKey() && IsTyping(SlateApp)) return false;
    for (const TWeakObjectPtr<UDrButtonBase>& Weak : GShortcutButtons) {
      UDrButtonBase* Button = Weak.Get();
      if (!Button || !Button->MatchesShortcut(Key) || !Button->GetIsEnabled() || !DrUIInput::IsInActiveLayer(Button) || !DrUIInput::IsShown(Button)) continue;
      Button->TriggerShortcut();
      return true;
    }
    const EDrUIInputAction Action = DrUIInput::GetActionForKey(Key);
    UE_LOG(LogDrUIKit, Verbose, TEXT("DrUIInput: %s 동작 %d, 탭 목록 %d개"), *Key.ToString(), int32(Action), GTabLists.Num());
    const bool bSub = Action == EDrUIInputAction::PrevSubTab || Action == EDrUIInputAction::NextSubTab;
    if (bSub || Action == EDrUIInputAction::PrevTab || Action == EDrUIInputAction::NextTab) {
      // 같은 종류(위 탭·하위 탭)를 받는 보이는 탭 목록 중 가장 나중에 만든 것(안쪽).
      for (int32 I = GTabLists.Num() - 1; I >= 0; --I) {
        UDrTabList* Tabs = GTabLists[I].Get();
        if (!Tabs || Tabs->Paging != (bSub ? EDrTabPaging::Triggers : EDrTabPaging::Bumpers)) continue;
        UE_LOG(LogDrUIKit, Verbose, TEXT("DrUIInput: 탭 목록 %s 층 %d 보임 %d"), *Tabs->GetName(), DrUIInput::IsInActiveLayer(Tabs), DrUIInput::IsShown(Tabs));
        if (!DrUIInput::IsInActiveLayer(Tabs) || !DrUIInput::IsShown(Tabs)) continue;
        const bool bNext = Action == EDrUIInputAction::NextTab || Action == EDrUIInputAction::NextSubTab;
        Tabs->SelectAdjacentTab(bNext ? 1 : -1, Key.IsGamepadKey());
        return true;
      }
    }
    return false;
  }

  virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& Event) override {
    GLastInputKind = EDrUIInputKind::Pointer;
    bool bConsumed = false;
    if (!GAnyKeyListeners.IsEmpty()) {
      TArray<TFunction<bool(const FKey&, bool)>> Listeners;
      GAnyKeyListeners.GenerateValueArray(Listeners);
      for (const auto& Listener : Listeners) bConsumed |= Listener(Event.GetEffectingButton(), false);
    }
    return bConsumed;
  }

  virtual const TCHAR* GetDebugName() const override { return TEXT("DrUIInput"); }

private:
  float RightStickY = 0.f;
  static bool IsTyping(FSlateApplication& SlateApp) {
    const TSharedPtr<SWidget> Focused = SlateApp.GetUserFocusedWidget(0);
    return Focused && (Focused->GetType() == TEXT("SEditableText") || Focused->GetType() == TEXT("SMultiLineEditableText"));
  }
};

namespace { TSharedPtr<FDrUIInputProcessor> GProcessor; }

void DrUIInputProcessor::Register() {
  if (GProcessor || !FSlateApplication::IsInitialized()) return;
  GProcessor = MakeShared<FDrUIInputProcessor>();
  const bool bAdded = FSlateApplication::Get().RegisterInputPreProcessor(GProcessor, 0);
  UE_LOG(LogDrUIKit, Log, TEXT("DrUIInput: 전역 입력 처리기 등록 %s"), bAdded ? TEXT("성공") : TEXT("실패"));
}

void DrUIInputProcessor::Unregister() {
  if (GProcessor && FSlateApplication::IsInitialized()) FSlateApplication::Get().UnregisterInputPreProcessor(GProcessor);
  GProcessor.Reset();
}
