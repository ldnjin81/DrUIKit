#include "Widgets/DrInput.h"
#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateColorBrush.h"
#include "Components/Border.h"
#include "Components/EditableText.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/MenuAnchor.h"
#include "Core/DrUIMenu.h"
#include "Components/MultiLineEditableText.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Framework/Application/SlateApplication.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrGlyph.h"
#include "Widgets/DrSurface.h"
#include "Widgets/DrText.h"

namespace {
/** 입력 칸 공통 칠하기: 평소 옅은 우물(hairline), 호버는 옅은 테두리, 포커스·열림은 2px 링, 오류는 빨간 테두리. */
void PaintWell(UDrSurface* Well, bool bHovered, bool bActive, bool bError) {
  if (!Well) return;
  Well->FillToken = bActive ? DrUITokens::GlassStrong : DrUITokens::Hairline;
  Well->FillStrongToken = NAME_None;
  Well->BackdropToken = NAME_None;
  Well->StrokeToken = bError ? DrUITokens::Danger : bActive ? DrUITokens::Focus : bHovered ? DrUITokens::InkFaint : NAME_None;
  Well->StrokeWidth = bActive ? 2.f : bError ? 1.5f : 1.f;
  Well->ApplyTheme();
}

UDrSurface* MakeWell(UWidgetTree* Tree, FName Name) {
  UDrSurface* Well = Tree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), Name);
  Well->RadiusToken = DrUITokens::RadiusSm;
  Well->PaddingToken = NAME_None;
  Well->SetPadding(FMargin(12.f, 9.f));
  return Well;
}
}

// ---------------------------------------------------------------- 텍스트 필드

FText UDrTextField::ClampLength(const FText& InText, int32 InMaxLength) {
  const FString String = InText.ToString();
  return InMaxLength > 0 && String.Len() > InMaxLength ? FText::FromString(String.Left(InMaxLength)) : InText;
}

TSharedRef<SWidget> UDrTextField::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) {
    UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Column"));
    WidgetTree->RootWidget = Column;
    LabelText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("LabelText"));
    if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(LabelText)) S->SetPadding(FMargin(0, 0, 0, 6));
    Field = MakeWell(WidgetTree, TEXT("Field"));
    if (bMultiLine) {
      MultiInput = WidgetTree->ConstructWidget<UMultiLineEditableText>(UMultiLineEditableText::StaticClass(), TEXT("MultiInput"));
      // 최소 줄 수만큼 높이(body 줄 높이 20px 기준), 더 길면 늘어난다.
      USizeBox* MinBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("MinBox"));
      MinBox->SetMinDesiredHeight(20.f * FMath::Max(1, MinLines));
      MinBox->AddChild(MultiInput);
      Field->SetContent(MinBox);
      Field->SetVerticalAlignment(VAlign_Fill);
    } else {
      Input = WidgetTree->ConstructWidget<UEditableText>(UEditableText::StaticClass(), TEXT("Input"));
      Field->SetContent(Input);
    }
    Column->AddChildToVerticalBox(Field);
    UHorizontalBox* Below = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Below"));
    HelperText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("HelperText"));
    if (UHorizontalBoxSlot* S = Below->AddChildToHorizontalBox(HelperText)) S->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    CounterText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("CounterText"));
    if (UHorizontalBoxSlot* S = Below->AddChildToHorizontalBox(CounterText)) S->SetPadding(FMargin(12, 0, 0, 0));
    if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(Below)) S->SetPadding(FMargin(0, 6, 0, 0));
    BindInput();
    if (!PendingText.IsEmpty()) SetInputText(PendingText);
    ApplyTheme();
  }
  // 갤러리·패드로 이 위젯에 포커스가 오면 입력 칸으로 넘긴다.
  if (UWidget* InputWidget = GetInputWidget()) SetDesiredFocusWidget(InputWidget);
  return Super::RebuildWidget();
}

void UDrTextField::NativeOnInitialized() {
  Super::NativeOnInitialized();
  BindInput();  // 위젯 블루프린트가 Input을 두었으면 여기서 잇는다
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
  ApplyTheme();
}

void UDrTextField::NativeDestruct() {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Super::NativeDestruct();
}

void UDrTextField::BindInput() {
  if (bBound) return;
  if (Input) {
    Input->OnTextChanged.AddUniqueDynamic(this, &ThisClass::HandleInputChanged);
    Input->OnTextCommitted.AddUniqueDynamic(this, &ThisClass::HandleInputCommitted);
    bBound = true;
  } else if (MultiInput) {
    MultiInput->OnTextChanged.AddUniqueDynamic(this, &ThisClass::HandleInputChanged);
    MultiInput->OnTextCommitted.AddUniqueDynamic(this, &ThisClass::HandleInputCommitted);
    bBound = true;
  }
}

UWidget* UDrTextField::GetInputWidget() const { return Input ? static_cast<UWidget*>(Input) : static_cast<UWidget*>(MultiInput); }

void UDrTextField::SetInputText(const FText& InText) {
  if (Input) Input->SetText(InText);
  else if (MultiInput) MultiInput->SetText(InText);
}

void UDrTextField::SetText(const FText& InText) {
  PendingText = ClampLength(InText, MaxLength);
  if (GetInputWidget()) {
    TGuardValue<bool> Guard(bClamping, true);  // 코드에서 넣은 값은 '고치기 시작'이 아니므로 오류를 내리지 않는다
    SetInputText(PendingText);
  }
  ApplyVisualState();
}

FText UDrTextField::GetText() const { return Input ? Input->GetText() : MultiInput ? MultiInput->GetText() : PendingText; }

void UDrTextField::SetError(const FText& InError) {
  Error = InError;
  ApplyVisualState();
}

void UDrTextField::HandleInputChanged(const FText& InText) {
  if (bClamping) return;
  const FText Clamped = ClampLength(InText, MaxLength);
  if (!Clamped.EqualTo(InText) && GetInputWidget()) {
    TGuardValue<bool> Guard(bClamping, true);
    SetInputText(Clamped);
  }
  Error = FText::GetEmpty();  // 고치기 시작하면 오류를 내린다(다시 검사는 쓰는 쪽이)
  ApplyVisualState();
  OnTextChanged.Broadcast(Clamped);
}

void UDrTextField::HandleInputCommitted(const FText& InText, ETextCommit::Type) { OnTextCommitted.Broadcast(InText); }

void UDrTextField::NativeOnMouseEnter(const FGeometry& Geometry, const FPointerEvent& Event) { Super::NativeOnMouseEnter(Geometry, Event); bHovered = true; ApplyVisualState(); }
void UDrTextField::NativeOnMouseLeave(const FPointerEvent& Event) { Super::NativeOnMouseLeave(Event); bHovered = false; ApplyVisualState(); }
void UDrTextField::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) { Super::NativeOnAddedToFocusPath(InFocusEvent); bFocused = true; ApplyVisualState(); }
void UDrTextField::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) { Super::NativeOnRemovedFromFocusPath(InFocusEvent); bFocused = false; ApplyVisualState(); }

void UDrTextField::NativeTick(const FGeometry& Geometry, float DeltaTime) {
  Super::NativeTick(Geometry, DeltaTime);
  if (GetIsEnabled() != bWasEnabled) { bWasEnabled = GetIsEnabled(); ApplyVisualState(); }
}

void UDrTextField::ApplyTheme() {
  if (Input) {
    const UDrUITheme& Theme = UDrUIThemeSubsystem::Current();
    FEditableTextStyle Style;
    Style.SetFont(Theme.MakeFont(DrUITokens::Body))
      .SetColorAndOpacity(FSlateColor(Theme.GetColor(DrUITokens::Ink)))
      .SetBackgroundImageSelected(FSlateColorBrush(Theme.GetColor(DrUITokens::AccentSoft)))
      .SetCaretImage(FSlateColorBrush(Theme.GetColor(DrUITokens::Accent)));  // 엔진 기본 커서는 흰색이라 밝은 테마에서 안 보인다
    Input->SetWidgetStyle(Style);
    Input->SetHintText(Hint);
    Input->SetIsPassword(bPassword);
  }
  if (MultiInput) {
    const UDrUITheme& Theme = UDrUIThemeSubsystem::Current();
    FTextBlockStyle Style;
    Style.SetFont(Theme.MakeFont(DrUITokens::Body))
      .SetColorAndOpacity(FSlateColor(Theme.GetColor(DrUITokens::Ink)))
      .SetSelectedBackgroundColor(FSlateColor(Theme.GetColor(DrUITokens::AccentSoft)));
    MultiInput->SetWidgetStyle(Style);
    MultiInput->SetHintText(Hint);
  }
  ApplyVisualState();
}

void UDrTextField::ApplyVisualState() {
  const bool bEnabled = GetIsEnabled();
  bWasEnabled = bEnabled;
  SetRenderOpacity(bEnabled ? 1.f : .4f);
  PaintWell(Field, bHovered && bEnabled, bFocused && bEnabled, HasError());
  if (LabelText) {
    LabelText->SetTokens(DrUITokens::Label, DrUITokens::Ink);
    LabelText->SetText(Label);
    LabelText->SetVisibility(Label.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
  }
  const FText Shown = HasError() ? Error : Helper;
  if (HelperText) {
    HelperText->SetTokens(DrUITokens::Caption, HasError() ? DrUITokens::Danger : DrUITokens::InkMuted);
    HelperText->SetText(Shown);
  }
  if (CounterText) {
    CounterText->SetTokens(DrUITokens::Caption, DrUITokens::InkMuted);
    CounterText->SetText(MaxLength > 0 ? FText::Format(NSLOCTEXT("DrTextField", "Counter", "{0}/{1}"), FText::AsNumber(GetText().ToString().Len()), FText::AsNumber(MaxLength)) : FText::GetEmpty());
    CounterText->SetVisibility(MaxLength > 0 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
  }
  if (HelperText && HelperText->GetParent())
    HelperText->GetParent()->SetVisibility(Shown.IsEmpty() && MaxLength <= 0 ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
}

UDrTextArea::UDrTextArea(const FObjectInitializer& Initializer) : Super(Initializer) {
  bMultiLine = true;
}

TSharedRef<SWidget> UDrTextArea::RebuildWidget() {
  MinLines = Lines;  // 기본 구성을 만들기 전에 넘긴다
  return Super::RebuildWidget();
}

// ---------------------------------------------------------------- 드롭다운 항목·목록

UDrDropdownItem::UDrDropdownItem(const FObjectInitializer& Initializer) : Super(Initializer) {
  PressedScale = 1.f;  // 목록 줄은 줄지 않는다
}

void UDrDropdownItem::SetOption(const FText& InLabel, bool bInChecked) {
  Label = InLabel;
  bChecked = bInChecked;
  ApplyVisualState();
}

void UDrDropdownItem::BuildDefaultTree() {
  Surface = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Surface"));
  Surface->FillToken = NAME_None;
  Surface->FillStrongToken = NAME_None;
  Surface->BackdropToken = NAME_None;
  Surface->RadiusToken = DrUITokens::RadiusSm;
  Surface->PaddingToken = NAME_None;
  Surface->SetPadding(FMargin(10.f, 7.f));
  WidgetTree->RootWidget = Surface;
  UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
  Surface->SetContent(Row);
  LabelText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("LabelText"));
  if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(LabelText)) { S->SetSize(FSlateChildSize(ESlateSizeRule::Fill)); S->SetVerticalAlignment(VAlign_Center); }
  Check = WidgetTree->ConstructWidget<UDrGlyph>(UDrGlyph::StaticClass(), TEXT("Check"));
  Check->Shape = EDrGlyphShape::Check;
  Check->GlyphSize = 16.f;
  if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(Check)) { S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(16, 0, 0, 0)); }
}

void UDrDropdownItem::ApplyVisualState() {
  if (Surface) {
    const bool bLit = VisualState == EDrUIState::Hover || VisualState == EDrUIState::Pressed;
    Surface->FillToken = bLit ? DrUITokens::Hairline : bChecked ? DrUITokens::AccentSoft : NAME_None;
    Surface->StrokeToken = bFocusedNow && DrUIState::IsFocusRingVisible(GetStateInputs()) ? DrUITokens::Focus : NAME_None;
    Surface->StrokeWidth = 2.f;
    Surface->ApplyTheme();
  }
  if (LabelText) {
    LabelText->SetTokens(DrUITokens::Body, VisualState == EDrUIState::Disabled ? DrUITokens::InkFaint : DrUITokens::Ink);
    LabelText->SetText(Label);
  }
  if (Check) {
    Check->SetColorToken(DrUITokens::Accent);
    Check->SetVisibility(bChecked ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);  // 자리는 남겨 너비가 흔들리지 않게
  }
}

UDrDropdownMenu::UDrDropdownMenu(const FObjectInitializer& Initializer) : Super(Initializer) {
  OptionClass = UDrDropdownItem::StaticClass();
  SetIsFocusable(true);  // 열릴 때 포커스를 받아 고른 항목으로 넘긴다
}

void UDrDropdownMenu::EnsureTree() {
  if (!WidgetTree || WidgetTree->RootWidget) return;
  // 팝업 창이 내용 크기에 딱 맞아 그림자가 잘리므로 투명 여백(아래가 더 넓음)을 둔다.
  UBorder* Margin = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ShadowMargin"));
  Margin->SetBrush(FSlateNoResource());
  Margin->SetPadding(FMargin(0.f, 0.f, 14.f, 22.f));  // 왼쪽·위는 0: 팝업이 누른 위젯 왼쪽 아래에 맞춰 뜨므로(그림자는 아래로 진다)
  WidgetTree->RootWidget = Margin;
  Frame = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("Frame"));
  Margin->SetContent(Frame);
  Surface = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Surface"));
  Surface->FillToken = DrUITokens::GlassStrong;
  Surface->BackdropToken = DrUITokens::BackdropSoft;
  Surface->StrokeToken = DrUITokens::GlassStroke;
  Surface->RadiusToken = DrUITokens::RadiusMd;
  Surface->PaddingToken = NAME_None;
  Surface->SetPadding(FMargin(6.f));
  Surface->ShadowToken = DrUITokens::ShadowFloat;
  Surface->HighlightToken = DrUITokens::GlassHighlight;
  Frame->AddChild(Surface);
  List = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("List"));
  Surface->SetContent(List);
}

TSharedRef<SWidget> UDrDropdownMenu::RebuildWidget() {
  EnsureTree();
  Fill();
  return Super::RebuildWidget();
}

void UDrDropdownMenu::SetOptions(const TArray<FText>& InOptions, int32 InSelected, float InMinWidth) {
  Options = InOptions;
  Selected = InSelected;
  MinWidth = InMinWidth;
  if (List) Fill();
}

void UDrDropdownMenu::Fill() {
  if (!List) return;
  List->ClearChildren();
  if (Frame) Frame->SetMinDesiredWidth(MinWidth);
  UWidget* FocusTarget = nullptr;
  for (int32 I = 0; I < Options.Num(); ++I) {
    UDrDropdownItem* Item = WidgetTree->ConstructWidget<UDrDropdownItem>(OptionClass ? OptionClass.Get() : UDrDropdownItem::StaticClass());
    Item->SetOption(Options[I], I == Selected);
    Item->OnClicked().AddWeakLambda(this, [this, I] { OnPicked.ExecuteIfBound(I); });
    List->AddChildToVerticalBox(Item);
    if (I == Selected || !FocusTarget) FocusTarget = Item;
  }
  if (FocusTarget) SetDesiredFocusWidget(FocusTarget);
}

FReply UDrDropdownMenu::NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) {
  if (Event.GetKey() == EKeys::Escape || Event.GetKey() == EKeys::Gamepad_FaceButton_Right) {
    OnDismiss.ExecuteIfBound();
    return FReply::Handled();
  }
  return Super::NativeOnKeyDown(Geometry, Event);
}

// ---------------------------------------------------------------- 드롭다운

UDrDropdown::UDrDropdown(const FObjectInitializer& Initializer) : Super(Initializer) {
  MenuClass = UDrDropdownMenu::StaticClass();
  PressedScale = .99f;
  ChevronTurn.Snap(0.f);
}

void UDrDropdown::BuildDefaultTree() {
  Anchor = WidgetTree->ConstructWidget<UMenuAnchor>(UMenuAnchor::StaticClass(), TEXT("Anchor"));
  Anchor->SetPlacement(MenuPlacement_BelowAnchor);
  Anchor->FitInWindow(true);
  DrUIMenu::HideMenuBackground(Anchor);  // 엔진 메뉴 창의 진회색 배경을 끈다(판은 우리가 칠한다)
  WidgetTree->RootWidget = Anchor;
  Surface = MakeWell(WidgetTree, TEXT("Surface"));
  Anchor->SetContent(Surface);
  UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
  Surface->SetContent(Row);
  ValueText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("ValueText"));
  if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(ValueText)) { S->SetSize(FSlateChildSize(ESlateSizeRule::Fill)); S->SetVerticalAlignment(VAlign_Center); }
  Chevron = WidgetTree->ConstructWidget<UDrGlyph>(UDrGlyph::StaticClass(), TEXT("Chevron"));
  Chevron->Shape = EDrGlyphShape::ChevronDown;
  Chevron->GlyphSize = 16.f;
  if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(Chevron)) { S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(12, 0, 0, 0)); }
  BindAnchor();
}

void UDrDropdown::NativeOnInitialized() {
  BindAnchor();  // 위젯 블루프린트가 Anchor를 두었으면 여기서 잇는다
  Super::NativeOnInitialized();
}

void UDrDropdown::BindAnchor() {
  if (!Anchor || Anchor->OnGetUserMenuContentEvent.IsBound()) return;
  Anchor->OnGetUserMenuContentEvent.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UDrDropdown, HandleGetMenu));
  Anchor->OnMenuOpenChanged.AddUniqueDynamic(this, &ThisClass::HandleMenuOpenChanged);
}

void UDrDropdown::SetOptions(const TArray<FText>& InOptions) {
  Options = InOptions;
  if (!Options.IsValidIndex(SelectedIndex)) SelectedIndex = INDEX_NONE;
  ApplyVisualState();
}

void UDrDropdown::SetSelectedIndex(int32 Index, bool bBroadcast) {
  const int32 Next = Options.IsValidIndex(Index) ? Index : INDEX_NONE;
  const bool bChanged = Next != SelectedIndex;
  SelectedIndex = Next;
  ApplyVisualState();
  if (bChanged && bBroadcast) OnSelectionChanged.Broadcast(SelectedIndex);
}

bool UDrDropdown::IsMenuOpen() const { return Anchor && Anchor->IsOpen(); }

void UDrDropdown::NativeOnClicked() {
  Super::NativeOnClicked();
  if (!Anchor) return;
  // 열린 목록 밖을 누르면 목록이 먼저 닫힌다. 그 같은 누름으로 다시 열리지 않게 엔진 판단을 따른다.
  if (Anchor->IsOpen()) Anchor->Close();
  else if (Anchor->ShouldOpenDueToClick()) Anchor->Open(true);
}

UUserWidget* UDrDropdown::HandleGetMenu() {
  UDrDropdownMenu* Menu = CreateWidget<UDrDropdownMenu>(this, MenuClass ? MenuClass.Get() : UDrDropdownMenu::StaticClass());
  if (!Menu) return nullptr;
  Menu->SetOptions(Options, SelectedIndex, GetCachedGeometry().GetLocalSize().X);
  Menu->OnPicked.BindWeakLambda(this, [this](int32 Index) {
    SetSelectedIndex(Index, true);
    if (Anchor) Anchor->Close();
  });
  Menu->OnDismiss.BindWeakLambda(this, [this] { if (Anchor) Anchor->Close(); });
  return Menu;
}

void UDrDropdown::HandleMenuOpenChanged(bool bIsOpen) {
  bOpen = bIsOpen;
  DriveSpring(ChevronTurn, bIsOpen ? 1.f : 0.f);
  if (Chevron && ChevronTurn.IsSettled()) Chevron->SetRenderTransformAngle(180.f * ChevronTurn.Value);
  if (!bIsOpen) SetFocus();  // 키보드·패드로 고른 뒤 포커스가 사라지지 않게 드롭다운으로 돌려준다
  ApplyVisualState();
}

void UDrDropdown::NativeTick(const FGeometry& Geometry, float DeltaTime) {
  Super::NativeTick(Geometry, DeltaTime);
  if (!ChevronTurn.IsSettled()) {
    ChevronTurn.Step(DeltaTime);
    if (Chevron) Chevron->SetRenderTransformAngle(180.f * ChevronTurn.Value);
  }
}

void UDrDropdown::ApplyVisualState() {
  const bool bDisabled = VisualState == EDrUIState::Disabled;
  SetRenderOpacity(bDisabled ? .4f : 1.f);
  const bool bRing = bOpen || (bFocusedNow && DrUIState::IsFocusRingVisible(GetStateInputs()));
  PaintWell(Surface, VisualState == EDrUIState::Hover || VisualState == EDrUIState::Pressed, bRing, false);
  if (ValueText) {
    const bool bHasValue = Options.IsValidIndex(SelectedIndex);
    ValueText->SetTokens(DrUITokens::Body, bHasValue ? DrUITokens::Ink : DrUITokens::InkFaint);  // 자리표시 글은 ink-faint
    ValueText->SetText(bHasValue ? Options[SelectedIndex] : Placeholder);
  }
  if (Chevron) Chevron->SetColorToken(DrUITokens::InkMuted);
}

// ---------------------------------------------------------------- 수평 선택기

UDrHorizontalSelector::UDrHorizontalSelector(const FObjectInitializer& Initializer) : Super(Initializer) {
  PressedScale = 1.f;  // 누를 때마다 값이 바뀌므로 크기는 그대로
  Slide.Snap(0.f);
}

int32 UDrHorizontalSelector::StepIndex(int32 Current, int32 Direction, int32 Count, bool bInWrap) {
  if (Count <= 0) return INDEX_NONE;
  if (Current < 0 || Current >= Count) return Direction >= 0 ? 0 : Count - 1;
  const int32 Moved = Current + Direction;
  return bInWrap ? ((Moved % Count) + Count) % Count : FMath::Clamp(Moved, 0, Count - 1);
}

void UDrHorizontalSelector::BuildDefaultTree() {
  Surface = MakeWell(WidgetTree, TEXT("Surface"));
  Surface->RadiusToken = DrUITokens::RadiusPill;
  Surface->SetPadding(FMargin(8.f, 8.f));
  WidgetTree->RootWidget = Surface;
  UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
  Surface->SetContent(Row);
  Prev = WidgetTree->ConstructWidget<UDrGlyph>(UDrGlyph::StaticClass(), TEXT("Prev"));
  Prev->Shape = EDrGlyphShape::ChevronLeft;
  Prev->GlyphSize = 18.f;
  if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(Prev)) S->SetVerticalAlignment(VAlign_Center);
  USizeBox* ValueBox = WidgetTree->ConstructWidget<USizeBox>();
  ValueBox->SetMinDesiredWidth(MinValueWidth);
  ValueBox->SetClipping(EWidgetClipping::ClipToBounds);  // 미끄러져 들어오는 글이 화살표 위로 넘치지 않게
  ValueText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("ValueText"));
  ValueText->SetJustification(ETextJustify::Center);
  ValueBox->AddChild(ValueText);
  if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(ValueBox)) { S->SetSize(FSlateChildSize(ESlateSizeRule::Fill)); S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(8, 0)); }
  Next = WidgetTree->ConstructWidget<UDrGlyph>(UDrGlyph::StaticClass(), TEXT("Next"));
  Next->Shape = EDrGlyphShape::ChevronRight;
  Next->GlyphSize = 18.f;
  if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(Next)) S->SetVerticalAlignment(VAlign_Center);
}

void UDrHorizontalSelector::SetOptions(const TArray<FText>& InOptions) {
  Options = InOptions;
  if (!Options.IsValidIndex(SelectedIndex)) SelectedIndex = Options.Num() > 0 ? 0 : INDEX_NONE;
  ApplyVisualState();
}

void UDrHorizontalSelector::SetSelectedIndex(int32 Index, bool bBroadcast) {
  const int32 NextIndex = Options.IsValidIndex(Index) ? Index : INDEX_NONE;
  const bool bChanged = NextIndex != SelectedIndex;
  SelectedIndex = NextIndex;
  ApplyVisualState();
  if (bChanged && bBroadcast) OnSelectionChanged.Broadcast(SelectedIndex);
}

bool UDrHorizontalSelector::Step(int32 Direction) {
  const int32 NextIndex = StepIndex(SelectedIndex, Direction, Options.Num(), bWrap);
  if (NextIndex == INDEX_NONE || NextIndex == SelectedIndex) return false;
  SelectedIndex = NextIndex;
  // 다음이면 오른쪽에서, 이전이면 왼쪽에서 들어온다.
  if (CanAnimate()) { Slide.Value = 14.f * FMath::Sign(Direction); Slide.Target = 0.f; Slide.Velocity = 0.f; }
  else Slide.Snap(0.f);
  ApplyVisualState();
  OnSelectionChanged.Broadcast(SelectedIndex);
  return true;
}

void UDrHorizontalSelector::NativeOnClicked() {
  Super::NativeOnClicked();
  // 마우스로 누르면 왼쪽 절반은 이전, 오른쪽 절반은 다음. 패드·키보드 확인 버튼은 다음.
  const FGeometry& Geometry = GetCachedGeometry();
  const FVector2D CursorPos = FSlateApplication::Get().GetCursorPos();
  const bool bLeft = Geometry.IsUnderLocation(CursorPos) && Geometry.AbsoluteToLocal(CursorPos).X < Geometry.GetLocalSize().X * .5f;
  Step(bLeft ? -1 : 1);
}

FNavigationReply UDrHorizontalSelector::NativeOnNavigation(const FGeometry& Geometry, const FNavigationEvent& Event, const FNavigationReply& DefaultReply) {
  const EUINavigation Type = Event.GetNavigationType();
  if ((Type == EUINavigation::Left || Type == EUINavigation::Right) && GetIsEnabled()) {
    Step(Type == EUINavigation::Left ? -1 : 1);
    return FNavigationReply::Stop();
  }
  return Super::NativeOnNavigation(Geometry, Event, DefaultReply);
}

void UDrHorizontalSelector::NativeTick(const FGeometry& Geometry, float DeltaTime) {
  Super::NativeTick(Geometry, DeltaTime);
  if (!Slide.IsSettled() && ValueText) {
    Slide.Step(DeltaTime);
    ValueText->SetRenderTranslation(FVector2D(Slide.Value, 0.f));
    ValueText->SetRenderOpacity(1.f - .6f * FMath::Min(1.f, FMath::Abs(Slide.Value) / 14.f));
  }
}

void UDrHorizontalSelector::ApplyVisualState() {
  const bool bDisabled = VisualState == EDrUIState::Disabled;
  SetRenderOpacity(bDisabled ? .4f : 1.f);
  const bool bHover = VisualState == EDrUIState::Hover || VisualState == EDrUIState::Pressed;
  PaintWell(Surface, bHover, bFocusedNow && DrUIState::IsFocusRingVisible(GetStateInputs()), false);
  const int32 Count = Options.Num();
  // 감싸지 않으면 끝에서 그쪽 화살표를 흐리게(더 갈 수 없음).
  const bool bCanPrev = Count > 1 && (bWrap || SelectedIndex > 0);
  const bool bCanNext = Count > 1 && (bWrap || SelectedIndex < Count - 1);
  const FName Arrow = bHover ? DrUITokens::Ink : DrUITokens::InkMuted;
  if (Prev) Prev->SetColorToken(bCanPrev ? Arrow : DrUITokens::InkFaint);
  if (Next) Next->SetColorToken(bCanNext ? Arrow : DrUITokens::InkFaint);
  if (ValueText) {
    ValueText->SetTokens(DrUITokens::Label, DrUITokens::Ink);
    ValueText->SetText(Options.IsValidIndex(SelectedIndex) ? Options[SelectedIndex] : FText::GetEmpty());
    if (Slide.IsSettled()) { ValueText->SetRenderTranslation(FVector2D::ZeroVector); ValueText->SetRenderOpacity(1.f); }
  }
}
