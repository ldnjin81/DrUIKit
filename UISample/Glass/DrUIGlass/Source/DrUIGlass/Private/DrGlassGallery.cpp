#include "DrGlassGallery.h"
#include "Blueprint/WidgetTree.h"
#include "CommonAnimatedSwitcher.h"
#include "Groups/CommonButtonGroupBase.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/SizeBoxSlot.h"
#include "Components/Spacer.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"
#include "Components/WidgetSwitcher.h"
#include "DrGlassBackdrop.h"
#include "Engine/World.h"
#include "HAL/PlatformMisc.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "TimerManager.h"
#include "UnrealClient.h"
#include "Widgets/DrToggle.h"
#include "Widgets/DrButtonPrompt.h"
#include "Widgets/DrGlyph.h"
#include "Widgets/DrHoldButton.h"
#include "Widgets/DrIcon.h"
#include "Widgets/DrInput.h"
#include "Widgets/DrListRow.h"
#include "Widgets/DrNotice.h"
#include "Widgets/DrPageIndicator.h"
#include "Widgets/DrRadialMenu.h"
#include "Widgets/DrSlider.h"
#include "Widgets/DrStatus.h"
#include "Widgets/DrSurface.h"
#include "Widgets/DrTabList.h"
#include "Widgets/DrText.h"
#include "Widgets/DrTooltip.h"

#define LOCTEXT_NAMESPACE "DrGlassGallery"

TArray<UClass*> UDrGlassGallery::CoveredClasses() {
  return {UDrText::StaticClass(), UDrIcon::StaticClass(), UDrSurface::StaticClass(), UDrButton::StaticClass(),
    UDrGlyph::StaticClass(), UDrCheckBox::StaticClass(), UDrRadio::StaticClass(), UDrSwitch::StaticClass(),
    UDrTabButton::StaticClass(), UDrTabList::StaticClass(), UDrSegmentedControl::StaticClass(), UDrSlider::StaticClass(),
    UDrBadge::StaticClass(), UDrProgressBar::StaticClass(), UDrProgressRing::StaticClass(), UDrSkeleton::StaticClass(), UDrSpinner::StaticClass(),
    UDrToast::StaticClass(), UDrToastStack::StaticClass(), UDrSnackbar::StaticClass(),
    UDrTextField::StaticClass(), UDrDropdown::StaticClass(), UDrDropdownMenu::StaticClass(), UDrDropdownItem::StaticClass(), UDrHorizontalSelector::StaticClass(),
    UDrHoldButton::StaticClass(), UDrRadialMenu::StaticClass(),
    UDrTooltip::StaticClass(), UDrListRow::StaticClass(), UDrPageIndicator::StaticClass(), UDrButtonPrompt::StaticClass()};
}

UDrText* UDrGlassGallery::MakeText(const FText& Text, FName Style, FName Color) {
  UDrText* Widget = WidgetTree->ConstructWidget<UDrText>();
  Widget->TextStyle = Style;
  Widget->ColorToken = Color;
  Widget->SetText(Text);
  return Widget;
}

UDrButton* UDrGlassGallery::MakeButton(const FText& Label, EDrButtonVariant Variant) {
  UDrButton* Button = WidgetTree->ConstructWidget<UDrButton>();
  Button->Label = Label;
  Button->Variant = Variant;
  return Button;
}

UWidget* UDrGlassGallery::Section(const FText& Title, UWidget* Body) {
  UDrSurface* Card = WidgetTree->ConstructWidget<UDrSurface>();
  Card->FillToken = TEXT("glass-strong");
  Card->RadiusToken = TEXT("radius-xl");
  Card->PaddingToken = TEXT("space-6");
  Card->BackdropToken = NAME_None;  // 조밀한 내용은 흐림 없이(디자인 시스템 원칙 2)
  UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>();
  Card->SetContent(Column);
  if (UVerticalBoxSlot* TitleSlot = Column->AddChildToVerticalBox(MakeText(Title, TEXT("title")))) TitleSlot->SetPadding(FMargin(0, 0, 0, 12));
  Column->AddChildToVerticalBox(Body);
  return Card;
}

UWidget* UDrGlassGallery::BuildFoundationPage() {
  UVerticalBox* Page = WidgetTree->ConstructWidget<UVerticalBox>();
  auto Add = [Page](UWidget* W) { if (UVerticalBoxSlot* S = Page->AddChildToVerticalBox(W)) S->SetPadding(FMargin(0, 0, 0, 16)); };

  UVerticalBox* Type = WidgetTree->ConstructWidget<UVerticalBox>();
  for (const TCHAR* Style : {TEXT("display"), TEXT("title"), TEXT("heading"), TEXT("body"), TEXT("label"), TEXT("caption"), TEXT("overline"), TEXT("metric"), TEXT("code")})
    if (UVerticalBoxSlot* S = Type->AddChildToVerticalBox(MakeText(FText::Format(LOCTEXT("TypeSample", "{0} · 다시 오셨네요 ₩1,172만"), FText::FromString(Style)), Style)))
      S->SetPadding(FMargin(0, 2));
  Add(Section(LOCTEXT("TypeTitle", "글자 스타일(UDrText)"), Type));

  UWrapBox* Swatches = WidgetTree->ConstructWidget<UWrapBox>();
  Swatches->SetInnerSlotPadding(FVector2D(12, 12));
  for (const TCHAR* Token : {TEXT("ground"), TEXT("glass"), TEXT("glass-strong"), TEXT("ink"), TEXT("ink-muted"), TEXT("accent"), TEXT("accent-soft"), TEXT("chip"), TEXT("ember"), TEXT("success"), TEXT("warning"), TEXT("danger")}) {
    UVerticalBox* Cell = WidgetTree->ConstructWidget<UVerticalBox>();
    UDrSurface* Chip = WidgetTree->ConstructWidget<UDrSurface>();
    Chip->FillToken = Token; Chip->FillStrongToken = NAME_None; Chip->BackdropToken = NAME_None;
    Chip->RadiusToken = TEXT("radius-md"); Chip->PaddingToken = NAME_None;
    Chip->SetContent(WidgetTree->ConstructWidget<USpacer>());
    USizeBox* ChipSize = WidgetTree->ConstructWidget<USizeBox>();
    ChipSize->SetWidthOverride(96.f);
    ChipSize->SetHeightOverride(48.f);
    ChipSize->AddChild(Chip);
    Cell->AddChildToVerticalBox(ChipSize);
    Cell->AddChildToVerticalBox(MakeText(FText::FromString(Token), TEXT("caption"), TEXT("ink-muted")));
    Swatches->AddChild(Cell);
  }
  Add(Section(LOCTEXT("ColorTitle", "색 토큰(UDrSurface 채움)"), Swatches));

  UHorizontalBox* Surfaces = WidgetTree->ConstructWidget<UHorizontalBox>();
  for (const auto& [Fill, Caption] : TArray<TPair<FName, FText>>{{TEXT("glass"), LOCTEXT("Glass", "glass + 흐림(유리 패널)")}, {TEXT("glass-strong"), LOCTEXT("Strong", "glass-strong(글이 많은 곳)")}}) {
    UDrSurface* Panel = WidgetTree->ConstructWidget<UDrSurface>();
    Panel->FillToken = Fill;
    Panel->RadiusToken = TEXT("radius-xl");
    Panel->PaddingToken = TEXT("space-5");
    if (Fill != TEXT("glass")) Panel->BackdropToken = NAME_None;
    UVerticalBox* Inner = WidgetTree->ConstructWidget<UVerticalBox>();
    Inner->AddChildToVerticalBox(MakeText(Caption, TEXT("heading")));
    Inner->AddChildToVerticalBox(MakeText(LOCTEXT("SurfaceBody", "표면 품질을 낮음으로 바꾸면 흐림이 꺼지고 불투명해진다."), TEXT("body"), TEXT("ink-muted")));
    Panel->SetContent(Inner);
    if (UHorizontalBoxSlot* S = Surfaces->AddChildToHorizontalBox(Panel)) { S->SetSize(FSlateChildSize(ESlateSizeRule::Fill)); S->SetPadding(FMargin(0, 0, 16, 0)); }
  }
  // 이 카드만 흐림 위에 직접 놓는다(표면이 바탕 원을 흐리게 보여야 하므로 Section 없이).
  if (UVerticalBoxSlot* S = Page->AddChildToVerticalBox(Surfaces)) S->SetPadding(FMargin(0, 0, 0, 16));

  UHorizontalBox* Icons = WidgetTree->ConstructWidget<UHorizontalBox>();
  for (const TCHAR* Token : {TEXT("ink"), TEXT("ink-muted"), TEXT("accent"), TEXT("danger")}) {
    UDrIcon* Icon = WidgetTree->ConstructWidget<UDrIcon>();
    Icon->ColorToken = Token;
    if (UHorizontalBoxSlot* S = Icons->AddChildToHorizontalBox(Icon)) S->SetPadding(FMargin(0, 0, 12, 0));
  }
  Icons->AddChildToHorizontalBox(MakeText(LOCTEXT("IconNote", "아이콘 모양(텍스처)은 게임이 지정한다. 여기서는 색 토큰만 확인."), TEXT("caption"), TEXT("ink-muted")));
  Add(Section(LOCTEXT("IconTitle", "아이콘(UDrIcon)"), Icons));
  return Page;
}

UWidget* UDrGlassGallery::BuildButtonPage() {
  UVerticalBox* Page = WidgetTree->ConstructWidget<UVerticalBox>();
  const TArray<TPair<EDrButtonVariant, FText>> Variants = {
    {EDrButtonVariant::Primary, LOCTEXT("Primary", "Primary")}, {EDrButtonVariant::Secondary, LOCTEXT("Secondary", "Secondary")},
    {EDrButtonVariant::Ghost, LOCTEXT("Ghost", "Ghost")}, {EDrButtonVariant::Destructive, LOCTEXT("Destructive", "Destructive")}};
  for (const auto& [Variant, Name] : Variants) {
    UWrapBox* Row = WidgetTree->ConstructWidget<UWrapBox>();
    Row->SetInnerSlotPadding(FVector2D(12, 12));
    // 1) 직접 눌러 보는 버튼: 누르면 로딩 1초 → 성공.
    UDrButton* Live = MakeButton(Variant == EDrButtonVariant::Destructive ? LOCTEXT("Delete", "삭제") : LOCTEXT("Save", "저장"), Variant);
    Live->OnClicked().AddWeakLambda(this, [this, Live] {
      Live->SetLoading(true);
      FTimerHandle Handle;
      if (UWorld* World = GetWorld()) World->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateWeakLambda(Live, [Live] { Live->ShowSuccess(LOCTEXT("Saved", "✓ 저장했어요")); }), 1.f, false);
    });
    Row->AddChild(Live);
    // 2) 상태 고정 견본(호버·눌림·포커스는 마우스·키보드로 직접 확인).
    UDrButton* Disabled = MakeButton(LOCTEXT("DisabledLabel", "비활성"), Variant); Disabled->SetIsEnabled(false); Row->AddChild(Disabled);
    UDrButton* Loading = MakeButton(LOCTEXT("LoadingLabel", "저장 중"), Variant); Row->AddChild(Loading);
    UDrButton* Error = MakeButton(LOCTEXT("ErrorLabel", "오류"), Variant); Row->AddChild(Error);
    UDrButton* Selected = MakeButton(LOCTEXT("SelectedLabel", "✓ 선택됨"), Variant); Selected->SetIsSelectable(true); Row->AddChild(Selected);
    Loading->SetLoading(true);
    Error->SetError(true, LOCTEXT("Retry", "다시 시도"));
    Selected->SetIsSelected(true, false);
    if (UVerticalBoxSlot* S = Page->AddChildToVerticalBox(Section(FText::Format(LOCTEXT("VariantTitle", "버튼 · {0}"), Name), Row))) S->SetPadding(FMargin(0, 0, 0, 16));
  }
  Page->AddChildToVerticalBox(MakeText(LOCTEXT("ButtonNote", "호버·눌림·포커스는 마우스와 Tab 키로 직접 확인. 첫 버튼은 누르면 로딩 → 성공(1.5초 뒤 기본)."), TEXT("caption"), TEXT("ink-muted")));
  return Page;
}

UWidget* UDrGlassGallery::BuildSelectionPage() {
  UVerticalBox* Page = WidgetTree->ConstructWidget<UVerticalBox>();
  auto Add = [Page](UWidget* W) { if (UVerticalBoxSlot* S = Page->AddChildToVerticalBox(W)) S->SetPadding(FMargin(0, 0, 0, 16)); };
  auto MakeRow = [this] { UWrapBox* Row = WidgetTree->ConstructWidget<UWrapBox>(); Row->SetInnerSlotPadding(FVector2D(24, 12)); return Row; };

  // 체크박스: 직접 눌러 보기 + 고정 견본(켜짐·비활성).
  UWrapBox* Checks = MakeRow();
  for (const auto& [Label, bOn, bEnabled] : TArray<TTuple<FText, bool, bool>>{
         {LOCTEXT("CheckLive", "알림 받기"), false, true}, {LOCTEXT("CheckOn", "자동 저장"), true, true},
         {LOCTEXT("CheckOffDisabled", "비활성"), false, false}, {LOCTEXT("CheckOnDisabled", "비활성 켜짐"), true, false}}) {
    UDrCheckBox* Check = WidgetTree->ConstructWidget<UDrCheckBox>();
    Check->Label = Label;
    Check->SetOn(bOn);
    Check->SetIsEnabled(bEnabled);
    Checks->AddChild(Check);
  }
  Add(Section(LOCTEXT("CheckTitle", "체크박스(UDrCheckBox) · 켜면 체크를 그린다"), Checks));

  // 라디오: CommonUI 버튼 묶음이 하나만 선택을 맡는다.
  UWrapBox* Radios = MakeRow();
  UCommonButtonGroupBase* Group = NewObject<UCommonButtonGroupBase>(this);
  Group->SetSelectionRequired(true);
  RadioGroups.Add(Group);
  for (const FText& Label : {LOCTEXT("RadioEasy", "쉬움"), LOCTEXT("RadioNormal", "보통"), LOCTEXT("RadioHard", "어려움")}) {
    UDrRadio* Radio = WidgetTree->ConstructWidget<UDrRadio>();
    Radio->Label = Label;
    Group->AddWidget(Radio);
    Radios->AddChild(Radio);
  }
  Group->SelectButtonAtIndex(1, false);
  UDrRadio* RadioDisabled = WidgetTree->ConstructWidget<UDrRadio>();
  RadioDisabled->Label = LOCTEXT("RadioDisabled", "비활성");
  RadioDisabled->SetIsEnabled(false);
  Radios->AddChild(RadioDisabled);
  Add(Section(LOCTEXT("RadioTitle", "라디오(UDrRadio + UCommonButtonGroupBase) · 하나만 선택"), Radios));

  // 스위치.
  UWrapBox* Switches = MakeRow();
  for (const auto& [Label, bOn, bEnabled] : TArray<TTuple<FText, bool, bool>>{
         {LOCTEXT("SwitchLive", "진동"), false, true}, {LOCTEXT("SwitchOn", "자막"), true, true}, {LOCTEXT("SwitchDisabled", "비활성"), true, false}}) {
    UDrSwitch* Switch = WidgetTree->ConstructWidget<UDrSwitch>();
    Switch->Label = Label;
    Switch->SetOn(bOn);
    Switch->SetIsEnabled(bEnabled);
    Switches->AddChild(Switch);
  }
  Add(Section(LOCTEXT("SwitchTitle", "스위치(UDrSwitch) · 손잡이가 스프링으로 미끄러진다"), Switches));

  // 슬라이더: 값 글자, 4단계 눈금(StepSize .25), 비활성.
  UWrapBox* Sliders = MakeRow();
  for (const auto& [Value, Step, bEnabled] : TArray<TTuple<float, float, bool>>{{.35f, .05f, true}, {.5f, .25f, true}, {.7f, .05f, false}}) {
    UHorizontalBox* Line = WidgetTree->ConstructWidget<UHorizontalBox>();
    UDrSlider* Slider = WidgetTree->ConstructWidget<UDrSlider>();
    Slider->SetValue(Value);
    Slider->SetStepSize(Step);
    Slider->SetIsEnabled(bEnabled);
    USizeBox* Width = WidgetTree->ConstructWidget<USizeBox>();
    Width->SetWidthOverride(220.f);
    Width->AddChild(Slider);
    if (UHorizontalBoxSlot* S = Line->AddChildToHorizontalBox(Width)) S->SetVerticalAlignment(VAlign_Center);
    UDrText* ValueText = MakeText(FText::AsPercent(Value), TEXT("label"), TEXT("ink-muted"));
    if (UHorizontalBoxSlot* S = Line->AddChildToHorizontalBox(ValueText)) { S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(12, 0, 0, 0)); }
    Slider->OnValueChanged.AddUniqueDynamic(this, &ThisClass::HandleSliderDemo);
    SliderDemoValues.Add(Slider, ValueText);
    Sliders->AddChild(Line);
  }
  Add(Section(LOCTEXT("SliderTitle", "슬라이더(UDrSlider) · 엔진 USlider + 채움 트랙. 두 번째는 4단계, 세 번째는 비활성"), Sliders));

  // 세그먼트: 고른 값을 옆 글자로 보여 준다.
  UHorizontalBox* SegmentRow = WidgetTree->ConstructWidget<UHorizontalBox>();
  UDrSegmentedControl* Segmented = WidgetTree->ConstructWidget<UDrSegmentedControl>();
  const TArray<FText> Ranges = {LOCTEXT("Day", "일"), LOCTEXT("Week", "주"), LOCTEXT("Month", "월"), LOCTEXT("Year", "년")};
  Segmented->SetOptions(Ranges);
  USizeBox* SegmentSize = WidgetTree->ConstructWidget<USizeBox>();
  SegmentSize->SetWidthOverride(320.f);
  SegmentSize->AddChild(Segmented);
  if (UHorizontalBoxSlot* S = SegmentRow->AddChildToHorizontalBox(SegmentSize)) S->SetVerticalAlignment(VAlign_Center);
  UDrText* SegmentValue = MakeText(FText::GetEmpty(), TEXT("body"), TEXT("ink-muted"));
  if (UHorizontalBoxSlot* S = SegmentRow->AddChildToHorizontalBox(SegmentValue)) { S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(16, 0, 0, 0)); }
  Segmented->OnSelectionChanged.AddUniqueDynamic(this, &ThisClass::HandleSegmentDemo);
  SegmentDemoValue = SegmentValue;
  SegmentDemoOptions = Ranges;
  Segmented->SetSelectedIndex(1);
  Add(Section(LOCTEXT("SegmentTitle", "세그먼트(UDrSegmentedControl) · 값 하나 고르기"), SegmentRow));

  // 탭(밑줄형): 위 페이지 탭은 알약형. 밑줄형은 작은 스위처에 붙여 확인한다.
  UVerticalBox* TabDemo = WidgetTree->ConstructWidget<UVerticalBox>();
  UDrTabList* Underline = WidgetTree->ConstructWidget<UDrTabList>();
  Underline->Style = EDrTabStyle::Underline;
  UCommonAnimatedSwitcher* TabPages = WidgetTree->ConstructWidget<UCommonAnimatedSwitcher>();
  Underline->SetLinkedSwitcher(TabPages);
  for (const auto& [Id, Label, Body] : TArray<TTuple<FName, FText, FText>>{
         {TEXT("Graphics"), LOCTEXT("TabGraphics", "그래픽"), LOCTEXT("TabGraphicsBody", "그래픽 설정 내용")},
         {TEXT("Sound"), LOCTEXT("TabSound", "소리"), LOCTEXT("TabSoundBody", "소리 설정 내용")},
         {TEXT("Controls"), LOCTEXT("TabControls", "조작"), LOCTEXT("TabControlsBody", "조작 설정 내용(패드는 탭 목록의 이전·다음 입력으로 넘긴다)")}}) {
    UDrText* Content = MakeText(Body, TEXT("body"), TEXT("ink-muted"));
    TabPages->AddChild(Content);
    Underline->AddTab(Id, Label, Content);
  }
  if (UVerticalBoxSlot* S = TabDemo->AddChildToVerticalBox(Underline)) { S->SetHorizontalAlignment(HAlign_Left); S->SetPadding(FMargin(0, 0, 0, 12)); }
  TabDemo->AddChildToVerticalBox(TabPages);
  Add(Section(LOCTEXT("TabTitle", "탭(UDrTabList, UDrTabButton) · 밑줄형. 위 페이지 탭이 알약형"), TabDemo));

  // 표시(UDrGlyph): 모든 모양.
  UWrapBox* Glyphs = MakeRow();
  for (const EDrGlyphShape Shape : {EDrGlyphShape::Check, EDrGlyphShape::Dot, EDrGlyphShape::Minus, EDrGlyphShape::Plus, EDrGlyphShape::Close,
         EDrGlyphShape::ChevronLeft, EDrGlyphShape::ChevronRight, EDrGlyphShape::ChevronDown, EDrGlyphShape::Alert}) {
    UDrGlyph* Glyph = WidgetTree->ConstructWidget<UDrGlyph>();
    Glyph->Shape = Shape;
    Glyph->GlyphSize = 24.f;
    Glyphs->AddChild(Glyph);
  }
  Add(Section(LOCTEXT("GlyphTitle", "표시(UDrGlyph) · 에셋 없이 선으로 그림"), Glyphs));
  return Page;
}

UWidget* UDrGlassGallery::BuildStatusPage() {
  UVerticalBox* Page = WidgetTree->ConstructWidget<UVerticalBox>();
  auto Add = [Page](UWidget* W) { if (UVerticalBoxSlot* S = Page->AddChildToVerticalBox(W)) S->SetPadding(FMargin(0, 0, 0, 16)); };
  auto MakeRow = [this] { UWrapBox* Row = WidgetTree->ConstructWidget<UWrapBox>(); Row->SetInnerSlotPadding(FVector2D(20, 12)); return Row; };
  // 높이 40 줄 가운데에 놓는다(크기가 다른 배지·링·스피너를 한 줄에 맞추기).
  auto Center = [this](UWidget* W) {
    USizeBox* Box = WidgetTree->ConstructWidget<USizeBox>();
    Box->SetHeightOverride(40.f);
    if (USizeBoxSlot* S = Cast<USizeBoxSlot>(Box->AddChild(W))) { S->SetVerticalAlignment(VAlign_Center); S->SetHorizontalAlignment(HAlign_Center); }
    return Box;
  };

  // 배지: 숫자·넘침·점·색 + 버튼 위에 붙인 배지(눌러서 +1).
  UWrapBox* Badges = MakeRow();
  for (const auto& [Count, bDot, Tone] : TArray<TTuple<int32, bool, EDrBadgeTone>>{
         {3, false, EDrBadgeTone::Alert}, {12, false, EDrBadgeTone::Accent}, {120, false, EDrBadgeTone::Danger},
         {5, false, EDrBadgeTone::Success}, {8, false, EDrBadgeTone::Warning}, {42, false, EDrBadgeTone::Neutral}, {0, true, EDrBadgeTone::Alert}}) {
    UDrBadge* Badge = WidgetTree->ConstructWidget<UDrBadge>();
    Badge->Count = Count;
    Badge->bDot = bDot;
    Badge->Tone = Tone;
    Badges->AddChild(Center(Badge));
  }
  UOverlay* Inbox = WidgetTree->ConstructWidget<UOverlay>();
  UDrButton* InboxButton = MakeButton(LOCTEXT("Inbox", "받은 편지함 +1"));
  if (UOverlaySlot* S = Inbox->AddChildToOverlay(InboxButton)) S->SetPadding(FMargin(0, 6, 8, 0));
  UDrBadge* InboxBadge = WidgetTree->ConstructWidget<UDrBadge>();
  InboxBadge->Count = 1;
  if (UOverlaySlot* S = Inbox->AddChildToOverlay(InboxBadge)) { S->SetHorizontalAlignment(HAlign_Right); S->SetVerticalAlignment(VAlign_Top); }
  InboxButton->OnClicked().AddWeakLambda(InboxBadge, [InboxBadge] { InboxBadge->SetCount(InboxBadge->Count + 1); });
  Badges->AddChild(Inbox);
  Add(Section(LOCTEXT("BadgeTitle", "배지(UDrBadge) · 숫자·99+·점, 색 6종. 오른쪽 버튼을 누르면 +1"), Badges));

  // 진행 막대 + 진행 링: 한 버튼으로 같이 25%씩 채운다.
  UVerticalBox* Progress = WidgetTree->ConstructWidget<UVerticalBox>();
  auto BarLine = [&](float Percent, FName Fill, bool bMarquee, const FText& Caption) {
    UHorizontalBox* Line = WidgetTree->ConstructWidget<UHorizontalBox>();
    UDrProgressBar* Bar = WidgetTree->ConstructWidget<UDrProgressBar>();
    Bar->FillToken = Fill;
    Bar->SetPercent(Percent);
    Bar->SetIsMarquee(bMarquee);
    USizeBox* Width = WidgetTree->ConstructWidget<USizeBox>();
    Width->SetWidthOverride(360.f);
    Width->AddChild(Bar);
    if (UHorizontalBoxSlot* S = Line->AddChildToHorizontalBox(Width)) S->SetVerticalAlignment(VAlign_Center);
    if (UHorizontalBoxSlot* S = Line->AddChildToHorizontalBox(MakeText(Caption, TEXT("caption"), TEXT("ink-muted")))) { S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(12, 0, 0, 0)); }
    if (UVerticalBoxSlot* S = Progress->AddChildToVerticalBox(Line)) S->SetPadding(FMargin(0, 0, 0, 10));
    return Bar;
  };
  UDrProgressBar* LiveBar = BarLine(.25f, TEXT("accent"), false, LOCTEXT("BarLive", "아래 버튼으로 채움"));
  BarLine(1.f, TEXT("success"), false, LOCTEXT("BarDone", "완료(success)"));
  BarLine(.7f, TEXT("danger"), false, LOCTEXT("BarDanger", "위험(danger)"));
  BarLine(0.f, TEXT("accent"), true, LOCTEXT("BarMarquee", "끝없는 진행(SetIsMarquee)"));
  UWrapBox* RingRow = MakeRow();
  UDrProgressRing* LiveRing = WidgetTree->ConstructWidget<UDrProgressRing>();
  LiveRing->Percent = .25f;
  RingRow->AddChild(LiveRing);
  for (const auto& [Percent, Size, Fill] : TArray<TTuple<float, float, FName>>{{.75f, 40.f, TEXT("success")}, {.4f, 24.f, TEXT("warning")}, {.9f, 56.f, TEXT("accent")}}) {
    UDrProgressRing* Ring = WidgetTree->ConstructWidget<UDrProgressRing>();
    Ring->Percent = Percent;
    Ring->RingSize = Size;
    Ring->FillToken = Fill;
    RingRow->AddChild(Center(Ring));
  }
  UDrProgressRing* Spin = WidgetTree->ConstructWidget<UDrProgressRing>();
  Spin->bIndeterminate = true;
  RingRow->AddChild(Spin);
  UDrButton* Advance = MakeButton(LOCTEXT("Advance", "진행 +25%"));
  Advance->OnClicked().AddWeakLambda(this, [LiveBar, LiveRing] {
    const float Next = LiveRing->Percent >= .999f ? 0.f : LiveRing->Percent + .25f;
    LiveBar->SetPercent(Next);
    LiveRing->SetPercent(Next);
  });
  RingRow->AddChild(Center(Advance));
  if (UVerticalBoxSlot* S = Progress->AddChildToVerticalBox(RingRow)) S->SetPadding(FMargin(0, 6, 0, 0));
  Add(Section(LOCTEXT("ProgressTitle", "진행 막대(UDrProgressBar) · 진행 링(UDrProgressRing). 첫 막대와 첫 링은 버튼으로 함께 채움, 마지막 링은 끝없는 진행"), Progress));

  // 스켈레톤: 목록 행 하나를 불러오는 모양.
  UHorizontalBox* Skeleton = WidgetTree->ConstructWidget<UHorizontalBox>();
  UDrSkeleton* Avatar = WidgetTree->ConstructWidget<UDrSkeleton>();
  Avatar->BlockSize = FVector2D(40.f);
  Avatar->RadiusToken = TEXT("radius-pill");
  if (UHorizontalBoxSlot* S = Skeleton->AddChildToHorizontalBox(Avatar)) { S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(0, 0, 12, 0)); }
  UVerticalBox* Lines = WidgetTree->ConstructWidget<UVerticalBox>();
  for (const float Width : {220.f, 140.f}) {
    UDrSkeleton* Line = WidgetTree->ConstructWidget<UDrSkeleton>();
    Line->BlockSize = FVector2D(Width, 12.f);
    if (UVerticalBoxSlot* S = Lines->AddChildToVerticalBox(Line)) S->SetPadding(FMargin(0, 3));
  }
  if (UHorizontalBoxSlot* S = Skeleton->AddChildToHorizontalBox(Lines)) S->SetVerticalAlignment(VAlign_Center);
  Add(Section(LOCTEXT("SkeletonTitle", "스켈레톤(UDrSkeleton) · 천천히 밝아졌다 어두워짐. 움직임 줄이기면 멈춤"), Skeleton));

  // 스피너: 크기·색.
  UWrapBox* Spinners = MakeRow();
  for (const auto& [Radius, Color] : TArray<TPair<float, FName>>{{6.f, TEXT("ink")}, {8.f, TEXT("ink-muted")}, {12.f, TEXT("accent")}}) {
    UDrSpinner* Spinner = WidgetTree->ConstructWidget<UDrSpinner>();
    Spinner->SetRadius(Radius);
    Spinner->ColorToken = Color;
    Spinners->AddChild(Center(Spinner));
  }
  Add(Section(LOCTEXT("SpinnerTitle", "스피너(UDrSpinner) · 엔진 UCircularThrobber를 토큰 색으로. 버튼 로딩도 이것을 쓴다"), Spinners));
  return Page;
}

UWidget* UDrGlassGallery::BuildNoticePage() {
  UVerticalBox* Page = WidgetTree->ConstructWidget<UVerticalBox>();
  auto Add = [Page](UWidget* W) { if (UVerticalBoxSlot* S = Page->AddChildToVerticalBox(W)) S->SetPadding(FMargin(0, 0, 0, 16)); };
  auto MakeRow = [this] { UWrapBox* Row = WidgetTree->ConstructWidget<UWrapBox>(); Row->SetInnerSlotPadding(FVector2D(12, 12)); return Row; };
  const TArray<TPair<EDrNoticeTone, FText>> Tones = {
    {EDrNoticeTone::Info, LOCTEXT("ToastInfo", "새 버전을 받을 수 있어요")}, {EDrNoticeTone::Success, LOCTEXT("ToastSuccess", "저장했어요")},
    {EDrNoticeTone::Warning, LOCTEXT("ToastWarning", "연결이 불안정해요")}, {EDrNoticeTone::Danger, LOCTEXT("ToastDanger", "보내지 못했어요")}};

  // 토스트 견본(사라지지 않게 0초) + 직접 띄우기(화면 위 가운데 쌓개).
  UVerticalBox* ToastDemo = WidgetTree->ConstructWidget<UVerticalBox>();
  UWrapBox* Samples = MakeRow();
  for (const auto& [Tone, Message] : Tones) {
    UDrToast* Toast = WidgetTree->ConstructWidget<UDrToast>();
    Toast->SetContent(Message, Tone);
    Toast->Enter(0.f);
    Samples->AddChild(Toast);
  }
  if (UVerticalBoxSlot* S = ToastDemo->AddChildToVerticalBox(Samples)) S->SetPadding(FMargin(0, 0, 0, 16));
  UWrapBox* Pushers = MakeRow();
  for (const auto& [Tone, Message] : Tones) {
    UDrButton* Push = MakeButton(Message);
    const EDrNoticeTone PushTone = Tone;
    const FText PushMessage = Message;
    Push->OnClicked().AddWeakLambda(this, [this, PushTone, PushMessage] { if (Toasts) Toasts->Push(PushMessage, PushTone); });
    Pushers->AddChild(Push);
  }
  ToastDemo->AddChildToVerticalBox(Pushers);
  Add(Section(LOCTEXT("ToastTitle", "토스트(UDrToast, UDrToastStack) · 위는 견본, 버튼은 화면 위에 3초 띄움. 3개를 넘으면 오래된 것부터 물러남"), ToastDemo));

  // 스낵바: 견본(누르면 물러났다 다시 뜸) + 직접 띄우기(화면 아래 가운데).
  UVerticalBox* SnackDemo = WidgetTree->ConstructWidget<UVerticalBox>();
  SampleSnackbar = WidgetTree->ConstructWidget<UDrSnackbar>();
  SampleSnackbar->Show(LOCTEXT("SnackSample", "메일 1개를 보관했어요"), LOCTEXT("Undo", "되돌리기"), 0.f);
  SampleSnackbar->OnAction.AddUniqueDynamic(this, &ThisClass::HandleSampleSnackbar);
  if (UVerticalBoxSlot* S = SnackDemo->AddChildToVerticalBox(SampleSnackbar)) { S->SetHorizontalAlignment(HAlign_Left); S->SetPadding(FMargin(0, 0, 0, 16)); }
  UWrapBox* SnackButtons = MakeRow();
  UDrButton* Delete = MakeButton(LOCTEXT("SnackDelete", "삭제하기(되돌리기 4초)"));
  Delete->OnClicked().AddWeakLambda(this, [this] { if (Snackbar) Snackbar->Show(LOCTEXT("SnackDeleted", "메일 1개를 삭제했어요"), LOCTEXT("Undo", "되돌리기")); });
  SnackButtons->AddChild(Delete);
  UDrButton* Burst = MakeButton(LOCTEXT("SnackBurst", "연속 3개(차례대로)"));
  Burst->OnClicked().AddWeakLambda(this, [this] {
    if (!Snackbar) return;
    for (int32 I = 1; I <= 3; ++I) Snackbar->Show(FText::Format(LOCTEXT("SnackQueued", "{0}번째 알림(행동 없음, 2초)"), I), FText::GetEmpty(), 2.f);
  });
  SnackButtons->AddChild(Burst);
  SnackDemo->AddChildToVerticalBox(SnackButtons);
  Add(Section(LOCTEXT("SnackTitle", "스낵바(UDrSnackbar) · 한 번에 하나, 나머지는 차례. 마우스를 올리면 시간이 멈춤"), SnackDemo));

  Page->AddChildToVerticalBox(MakeText(LOCTEXT("NoticeNote", "확인 대화상자는 화면 층(UI 흐름)과 함께 만든다. 움직임 줄이기를 켜면 바로 나타나고 바로 사라진다."), TEXT("caption"), TEXT("ink-muted")));
  return Page;
}

UWidget* UDrGlassGallery::BuildInputPage() {
  UVerticalBox* Page = WidgetTree->ConstructWidget<UVerticalBox>();
  auto Add = [Page](UWidget* W) { if (UVerticalBoxSlot* S = Page->AddChildToVerticalBox(W)) S->SetPadding(FMargin(0, 0, 0, 16)); };
  auto MakeRow = [this] { UWrapBox* Row = WidgetTree->ConstructWidget<UWrapBox>(); Row->SetInnerSlotPadding(FVector2D(24, 16)); return Row; };
  auto Sized = [this](UWidget* W, float Width) { USizeBox* Box = WidgetTree->ConstructWidget<USizeBox>(); Box->SetWidthOverride(Width); Box->AddChild(W); return Box; };

  // 텍스트 필드: 안내·비밀번호·글자 수·오류·비활성. 닉네임은 Enter로 확정하면 2자 미만일 때 오류.
  UWrapBox* Fields = MakeRow();
  auto Field = [&](const FText& Label, const FText& Hint, const FText& Helper) {
    UDrTextField* F = WidgetTree->ConstructWidget<UDrTextField>();
    F->Label = Label; F->Hint = Hint; F->Helper = Helper;
    Fields->AddChild(Sized(F, 260.f));
    return F;
  };
  NicknameField = Field(LOCTEXT("Nickname", "닉네임"), LOCTEXT("NicknameHint", "2~12자"), LOCTEXT("NicknameHelper", "Enter로 확정(2자 미만이면 오류)"));
  NicknameField->MaxLength = 12;
  NicknameField->OnTextCommitted.AddUniqueDynamic(this, &ThisClass::HandleNicknameCommitted);
  Field(LOCTEXT("Password", "비밀번호"), LOCTEXT("PasswordHint", "8자 이상"), FText::GetEmpty())->bPassword = true;
  UDrTextField* Errored = Field(LOCTEXT("Email", "이메일"), FText::GetEmpty(), FText::GetEmpty());
  Errored->SetText(LOCTEXT("EmailValue", "player@"));
  Errored->SetError(LOCTEXT("EmailError", "이메일 형식이 아니에요"));
  Field(LOCTEXT("ServerName", "서버"), LOCTEXT("ServerHint", "자동 선택됨"), LOCTEXT("ServerHelper", "비활성"))->SetIsEnabled(false);
  Add(Section(LOCTEXT("FieldTitle", "텍스트 필드(UDrTextField) · 엔진 UEditableText + 라벨·안내·글자 수·오류"), Fields));

  // 드롭다운: 직접 열기(자리표시·고른 값) + 비활성 + 목록 견본(UDrDropdownMenu를 그대로 놓음).
  const TArray<FText> Regions = {LOCTEXT("Seoul", "서울"), LOCTEXT("Tokyo", "도쿄"), LOCTEXT("Singapore", "싱가포르"), LOCTEXT("Frankfurt", "프랑크푸르트")};
  UWrapBox* Dropdowns = MakeRow();
  for (const auto& [Selected, bEnabled] : TArray<TPair<int32, bool>>{{INDEX_NONE, true}, {1, true}, {2, false}}) {
    UDrDropdown* Dropdown = WidgetTree->ConstructWidget<UDrDropdown>();
    Dropdown->Placeholder = LOCTEXT("RegionPlaceholder", "지역 선택");
    Dropdown->SetOptions(Regions);
    Dropdown->SetSelectedIndex(Selected);
    Dropdown->SetIsEnabled(bEnabled);
    Dropdowns->AddChild(Sized(Dropdown, 200.f));
  }
  // 목록 견본은 아래 줄에 따로(같은 줄이면 드롭다운이 목록 높이만큼 늘어난다).
  UVerticalBox* DropdownDemo = WidgetTree->ConstructWidget<UVerticalBox>();
  if (UVerticalBoxSlot* S = DropdownDemo->AddChildToVerticalBox(Dropdowns)) S->SetPadding(FMargin(0, 0, 0, 16));
  UDrDropdownMenu* MenuSample = WidgetTree->ConstructWidget<UDrDropdownMenu>();
  MenuSample->SetOptions(Regions, 1, 200.f);
  if (UVerticalBoxSlot* S = DropdownDemo->AddChildToVerticalBox(MenuSample)) S->SetHorizontalAlignment(HAlign_Left);
  Add(Section(LOCTEXT("DropdownTitle", "드롭다운(UDrDropdown, UDrDropdownMenu, UDrDropdownItem) · 누르면 아래에 목록. 아래는 열린 목록 견본"), DropdownDemo));

  // 수평 선택기: 감쌈(끝에서 처음으로) / 멈춤(끝 화살표 흐림) / 비활성.
  UWrapBox* Selectors = MakeRow();
  struct FSelectorDemo { TArray<FText> Options; int32 Selected; bool bWrapAround, bEnabled; };
  const FSelectorDemo SelectorList[] = {
    {{LOCTEXT("Easy", "쉬움"), LOCTEXT("Normal", "보통"), LOCTEXT("Hard", "어려움"), LOCTEXT("Nightmare", "악몽")}, 1, true, true},
    {{LOCTEXT("QLow", "낮음"), LOCTEXT("QMid", "중간"), LOCTEXT("QHigh", "높음"), LOCTEXT("QEpic", "최고")}, 3, false, true},
    {{LOCTEXT("Off", "끔"), LOCTEXT("On", "켬")}, 0, true, false}};
  for (const FSelectorDemo& Demo : SelectorList) {
    UDrHorizontalSelector* Selector = WidgetTree->ConstructWidget<UDrHorizontalSelector>();
    Selector->bWrap = Demo.bWrapAround;
    Selector->SetOptions(Demo.Options);
    Selector->SetSelectedIndex(Demo.Selected);
    Selector->SetIsEnabled(Demo.bEnabled);
    Selectors->AddChild(Selector);
  }
  Add(Section(LOCTEXT("SelectorTitle", "수평 선택기(UDrHorizontalSelector) · 패드·키보드 좌우, 마우스는 왼쪽 절반 이전·오른쪽 절반 다음. 두 번째는 끝에서 멈춤"), Selectors));
  return Page;
}

void UDrGlassGallery::HandleNicknameCommitted(const FText& Text) {
  if (NicknameField && Text.ToString().TrimStartAndEnd().Len() < 2) NicknameField->SetError(LOCTEXT("NicknameError", "2자 이상 입력하세요"));
}

UWidget* UDrGlassGallery::BuildGamePage() {
  UVerticalBox* Page = WidgetTree->ConstructWidget<UVerticalBox>();
  auto Add = [Page](UWidget* W) { if (UVerticalBoxSlot* S = Page->AddChildToVerticalBox(W)) S->SetPadding(FMargin(0, 0, 0, 16)); };

  // 홀드 버튼: 변형·시간별. 다 차면 성공 표시.
  UWrapBox* Holds = WidgetTree->ConstructWidget<UWrapBox>();
  Holds->SetInnerSlotPadding(FVector2D(12, 12));
  struct FHoldDemo { FText Label; EDrButtonVariant Variant; float Seconds; FText Done; bool bEnabled; };
  const FHoldDemo HoldList[] = {
    {LOCTEXT("HoldDelete", "길게 눌러 삭제(1초)"), EDrButtonVariant::Destructive, 1.f, LOCTEXT("HoldDeleted", "✓ 삭제했어요"), true},
    {LOCTEXT("HoldBuy", "길게 눌러 구매(0.6초)"), EDrButtonVariant::Primary, .6f, LOCTEXT("HoldBought", "✓ 구매했어요"), true},
    {LOCTEXT("HoldSkip", "길게 눌러 건너뛰기(2초)"), EDrButtonVariant::Secondary, 2.f, LOCTEXT("HoldSkipped", "✓ 건너뛰었어요"), true},
    {LOCTEXT("HoldDisabled", "비활성"), EDrButtonVariant::Secondary, 1.f, FText::GetEmpty(), false}};
  for (const FHoldDemo& Demo : HoldList) {
    UDrHoldButton* Hold = WidgetTree->ConstructWidget<UDrHoldButton>();
    Hold->Label = Demo.Label;
    Hold->Variant = Demo.Variant;
    Hold->SetHoldSeconds(Demo.Seconds);
    Hold->SetIsEnabled(Demo.bEnabled);
    const FText Done = Demo.Done;
    Hold->OnClicked().AddWeakLambda(Hold, [Hold, Done] { Hold->ShowSuccess(Done); });
    Holds->AddChild(Hold);
  }
  Add(Section(LOCTEXT("HoldTitle", "홀드 버튼(UDrHoldButton) · 누르고 있으면 차오르고 다 차면 실행. 중간에 떼면 되감김(CommonUI 길게 누르기)"), Holds));

  // 파이 메뉴: 고르면 토스트, Esc·B는 취소 토스트.
  UHorizontalBox* RadialRow = WidgetTree->ConstructWidget<UHorizontalBox>();
  RadialDemo = WidgetTree->ConstructWidget<UDrRadialMenu>();
  RadialDemo->Hint = LOCTEXT("RadialHint", "방향을 고르세요");
  RadialDemo->SetItems({LOCTEXT("RAttack", "공격"), LOCTEXT("RDefend", "방어"), LOCTEXT("RPotion", "물약"), LOCTEXT("RMap", "지도"), LOCTEXT("RPing", "핑"), LOCTEXT("REmote", "감정")});
  RadialDemo->SetHighlighted(1);
  RadialDemo->OnItemSelected.AddUniqueDynamic(this, &ThisClass::HandleRadialSelected);
  RadialDemo->OnCancelled.AddUniqueDynamic(this, &ThisClass::HandleRadialCancelled);
  RadialRow->AddChildToHorizontalBox(RadialDemo);
  if (UHorizontalBoxSlot* S = RadialRow->AddChildToHorizontalBox(MakeText(LOCTEXT("RadialNote", "마우스: 가운데에서 본 방향으로 가리키고 왼쪽 버튼을 떼면 확정.\n패드: 스틱으로 가리키고 A로 확정, B로 취소.\n키보드: 메뉴를 한 번 누른 뒤 화살표로 옮기고 Enter, Esc로 취소."), TEXT("body"), TEXT("ink-muted"))))
  { S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(32, 0, 0, 0)); }
  Add(Section(LOCTEXT("RadialTitle", "파이 메뉴(UDrRadialMenu) · 방향으로 빨리 고르기. 강조가 칸 사이를 미끄러진다"), RadialRow));
  return Page;
}

void UDrGlassGallery::HandleRadialSelected(int32 Index) {
  if (Toasts && RadialDemo && RadialDemo->Items.IsValidIndex(Index))
    Toasts->Push(FText::Format(LOCTEXT("RadialPicked", "고름: {0}"), RadialDemo->Items[Index]), EDrNoticeTone::Success, 1.5f);
}

void UDrGlassGallery::ShowCarouselPage(int32 Page) {
  if (CarouselPages) CarouselPages->SetActiveWidgetIndex(Page);
  if (CarouselDots) CarouselDots->SetCurrent(Page);
}

void UDrGlassGallery::HandleRadialCancelled() {
  if (Toasts) Toasts->Push(LOCTEXT("RadialCancelled", "파이 메뉴 취소"), EDrNoticeTone::Info, 1.5f);
}

UWidget* UDrGlassGallery::BuildNavigationPage() {
  UVerticalBox* Page = WidgetTree->ConstructWidget<UVerticalBox>();
  auto Add = [Page](UWidget* W) { if (UVerticalBoxSlot* S = Page->AddChildToVerticalBox(W)) S->SetPadding(FMargin(0, 0, 0, 16)); };

  // 툴팁: 버튼에 마우스를 올리면 뜬다(엔진 툴팁 창). 아래는 같은 툴팁을 그대로 놓은 견본.
  UVerticalBox* TooltipDemo = WidgetTree->ConstructWidget<UVerticalBox>();
  UWrapBox* Hovers = WidgetTree->ConstructWidget<UWrapBox>();
  Hovers->SetInnerSlotPadding(FVector2D(12, 12));
  struct FTipDemo { FText Label, Title, Body; };
  const FTipDemo Tips[] = {
    {LOCTEXT("TipSave", "저장"), LOCTEXT("TipSaveTitle", "저장 (Ctrl+S)"), LOCTEXT("TipSaveBody", "지금 진행을 슬롯 1에 저장합니다.")},
    {LOCTEXT("TipShort", "설명만"), FText::GetEmpty(), LOCTEXT("TipShortBody", "제목 없이 한 줄 설명")},
    {LOCTEXT("TipLong", "긴 설명"), LOCTEXT("TipLongTitle", "표면 품질"), LOCTEXT("TipLongBody", "낮음으로 바꾸면 뒤 흐림을 끄고 불투명하게 칠합니다. 저사양 기기나 글이 많은 화면에서 글자 대비를 지키려고 씁니다.")}};
  UWrapBox* Samples = WidgetTree->ConstructWidget<UWrapBox>();
  Samples->SetInnerSlotPadding(FVector2D(16, 12));
  for (const FTipDemo& Tip : Tips) {
    UDrButton* Button = MakeButton(Tip.Label);
    UDrTooltip::Attach(this, Button, Tip.Title, Tip.Body);
    Hovers->AddChild(Button);
    UDrTooltip* Sample = WidgetTree->ConstructWidget<UDrTooltip>();
    Sample->SetContent(Tip.Title, Tip.Body);
    if (UWrapBoxSlot* S = Cast<UWrapBoxSlot>(Samples->AddChild(Sample))) S->SetVerticalAlignment(VAlign_Top);  // 줄 높이만큼 늘어나지 않게
  }
  if (UVerticalBoxSlot* S = TooltipDemo->AddChildToVerticalBox(Hovers)) S->SetPadding(FMargin(0, 0, 0, 16));
  TooltipDemo->AddChildToVerticalBox(Samples);
  Add(Section(LOCTEXT("TooltipTitle", "툴팁(UDrTooltip) · 버튼에 마우스를 올려 보기. 아래는 견본(제목+설명, 설명만, 긴 글 줄바꿈)"), TooltipDemo));

  // 목록 행: 스크롤 상자 + 버튼 묶음(하나만 선택) + 목록 밖 단독 행(비활성).
  UHorizontalBox* ListDemo = WidgetTree->ConstructWidget<UHorizontalBox>();
  UScrollBox* ListScroll = WidgetTree->ConstructWidget<UScrollBox>();
  UCommonButtonGroupBase* RowGroup = NewObject<UCommonButtonGroupBase>(this);
  RowGroup->SetSelectionRequired(true);
  RadioGroups.Add(RowGroup);  // 위젯이 아니라 따로 붙잡아 둔다
  struct FRowDemo { FText Title, Subtitle, Trailing; };
  const FRowDemo Rows[] = {
    {LOCTEXT("Slot1", "슬롯 1 · 북쪽 숲"), LOCTEXT("Slot1Sub", "레벨 12 · 플레이 3시간 20분"), LOCTEXT("Slot1Time", "방금")},
    {LOCTEXT("Slot2", "슬롯 2 · 폐광"), LOCTEXT("Slot2Sub", "레벨 9 · 플레이 2시간 5분"), LOCTEXT("Slot2Time", "어제")},
    {LOCTEXT("Slot3", "슬롯 3 · 성채"), LOCTEXT("Slot3Sub", "레벨 21 · 플레이 11시간"), LOCTEXT("Slot3Time", "3일 전")},
    {LOCTEXT("Slot4", "새 게임"), FText::GetEmpty(), FText::GetEmpty()}};
  for (const FRowDemo& Row : Rows) {
    UDrListRow* ListRow = WidgetTree->ConstructWidget<UDrListRow>();
    ListRow->SetContent(Row.Title, Row.Subtitle, Row.Trailing);
    ListRow->SetIsSelectable(true);
    RowGroup->AddWidget(ListRow);
    if (UScrollBoxSlot* S = Cast<UScrollBoxSlot>(ListScroll->AddChild(ListRow))) S->SetPadding(FMargin(0, 0, 0, 2));
  }
  RowGroup->SelectButtonAtIndex(0, false);
  USizeBox* ListSize = WidgetTree->ConstructWidget<USizeBox>();
  ListSize->SetWidthOverride(420.f);
  ListSize->AddChild(ListScroll);
  ListDemo->AddChildToHorizontalBox(ListSize);
  UDrListRow* Lone = WidgetTree->ConstructWidget<UDrListRow>();
  Lone->SetContent(LOCTEXT("LoneRow", "클라우드 저장"), LOCTEXT("LoneRowSub", "로그인 후 사용 가능(비활성)"), FText::GetEmpty());
  Lone->SetIsEnabled(false);
  USizeBox* LoneSize = WidgetTree->ConstructWidget<USizeBox>();
  LoneSize->SetWidthOverride(320.f);
  LoneSize->AddChild(Lone);
  if (UHorizontalBoxSlot* S = ListDemo->AddChildToHorizontalBox(LoneSize)) { S->SetVerticalAlignment(VAlign_Top); S->SetPadding(FMargin(32, 0, 0, 0)); }
  Add(Section(LOCTEXT("ListTitle", "목록 행(UDrListRow) · 호버 렌즈, 선택(accent-soft), 패드·화살표로 이동. 긴 목록은 UCommonListView 항목(위젯 블루프린트 부모)으로. 오른쪽은 비활성"), ListDemo));

  // 페이지 인디케이터: 넘기는 카드 + 이전·다음 버튼, 점을 눌러도 이동.
  UVerticalBox* Carousel = WidgetTree->ConstructWidget<UVerticalBox>();
  CarouselPages = WidgetTree->ConstructWidget<UWidgetSwitcher>();
  for (const FText& Body : {LOCTEXT("Card1", "1 · 환영합니다"), LOCTEXT("Card2", "2 · 조작법"), LOCTEXT("Card3", "3 · 저장과 불러오기"), LOCTEXT("Card4", "4 · 설정"), LOCTEXT("Card5", "5 · 시작하기")}) {
    UDrSurface* Card = WidgetTree->ConstructWidget<UDrSurface>();
    Card->FillToken = TEXT("hairline"); Card->FillStrongToken = NAME_None; Card->BackdropToken = NAME_None; Card->StrokeToken = NAME_None;
    Card->RadiusToken = TEXT("radius-lg"); Card->PaddingToken = TEXT("space-6");
    Card->SetContent(MakeText(Body, TEXT("title")));
    CarouselPages->AddChild(Card);
  }
  USizeBox* CardSize = WidgetTree->ConstructWidget<USizeBox>();
  CardSize->SetWidthOverride(420.f);
  CardSize->SetHeightOverride(96.f);
  CardSize->AddChild(CarouselPages);
  if (UVerticalBoxSlot* S = Carousel->AddChildToVerticalBox(CardSize)) { S->SetHorizontalAlignment(HAlign_Left); S->SetPadding(FMargin(0, 0, 0, 12)); }
  UHorizontalBox* Controls = WidgetTree->ConstructWidget<UHorizontalBox>();
  UDrButton* Prev = MakeButton(LOCTEXT("PrevPage", "이전"), EDrButtonVariant::Ghost);
  CarouselDots = WidgetTree->ConstructWidget<UDrPageIndicator>();
  CarouselDots->PageCount = 5;
  CarouselDots->OnPageClicked.AddUniqueDynamic(this, &ThisClass::ShowCarouselPage);
  UDrButton* NextButton = MakeButton(LOCTEXT("NextPage", "다음"), EDrButtonVariant::Ghost);
  Prev->OnClicked().AddWeakLambda(this, [this] { if (CarouselDots) ShowCarouselPage(FMath::Max(0, CarouselDots->Current - 1)); });
  NextButton->OnClicked().AddWeakLambda(this, [this] { if (CarouselDots) ShowCarouselPage(FMath::Min(CarouselDots->PageCount - 1, CarouselDots->Current + 1)); });
  Controls->AddChildToHorizontalBox(Prev);
  if (UHorizontalBoxSlot* S = Controls->AddChildToHorizontalBox(CarouselDots)) { S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(16, 0)); }
  Controls->AddChildToHorizontalBox(NextButton);
  Carousel->AddChildToVerticalBox(Controls);
  ShowCarouselPage(2);
  Add(Section(LOCTEXT("PageDotsTitle", "페이지 인디케이터(UDrPageIndicator) · 지금 점이 알약으로 늘어나 옆으로 미끄러짐. 점을 눌러도 이동"), Carousel));

  // 버튼 프롬프트: 키보드 줄·패드 줄(고정), 자동 줄(마지막 입력 장치를 따름). 상호작용은 길게 누르기 진행 60%.
  UVerticalBox* Prompts = WidgetTree->ConstructWidget<UVerticalBox>();
  struct FPromptDemo { FKey Keyboard, Gamepad; FText Label; float Hold; };
  const FPromptDemo PromptList[] = {
    {EKeys::Enter, EKeys::Gamepad_FaceButton_Bottom, LOCTEXT("PromptConfirm", "확인"), 0.f},
    {EKeys::Escape, EKeys::Gamepad_FaceButton_Right, LOCTEXT("PromptBack", "뒤로"), 0.f},
    {EKeys::E, EKeys::Gamepad_FaceButton_Left, LOCTEXT("PromptInteract", "길게 눌러 열기"), .6f},
    {EKeys::Tab, EKeys::Gamepad_RightShoulder, LOCTEXT("PromptNextTab", "다음 탭"), 0.f},
    {EKeys::SpaceBar, EKeys::Gamepad_LeftTrigger, LOCTEXT("PromptAim", "조준"), 0.f}};
  for (const auto& [DeviceMode, Caption] : TArray<TPair<EDrPromptDevice, FText>>{
         {EDrPromptDevice::Keyboard, LOCTEXT("PromptRowKeyboard", "키보드")}, {EDrPromptDevice::Gamepad, LOCTEXT("PromptRowGamepad", "패드")},
         {EDrPromptDevice::Auto, LOCTEXT("PromptRowAuto", "자동(마지막 입력)")}}) {
    UHorizontalBox* Line = WidgetTree->ConstructWidget<UHorizontalBox>();
    USizeBox* CaptionBox = WidgetTree->ConstructWidget<USizeBox>();
    CaptionBox->SetWidthOverride(120.f);
    CaptionBox->AddChild(MakeText(Caption, TEXT("caption"), TEXT("ink-muted")));
    if (UHorizontalBoxSlot* S = Line->AddChildToHorizontalBox(CaptionBox)) S->SetVerticalAlignment(VAlign_Center);
    for (const FPromptDemo& Demo : PromptList) {
      UDrButtonPrompt* Prompt = WidgetTree->ConstructWidget<UDrButtonPrompt>();
      Prompt->SetKeys(Demo.Keyboard, Demo.Gamepad, Demo.Label);
      Prompt->SetDevice(DeviceMode);
      Prompt->SetHoldProgress(Demo.Hold);
      if (UHorizontalBoxSlot* S = Line->AddChildToHorizontalBox(Prompt)) { S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(0, 0, 24, 0)); }
    }
    if (UVerticalBoxSlot* S = Prompts->AddChildToVerticalBox(Line)) S->SetPadding(FMargin(0, 0, 0, 10));
  }
  Add(Section(LOCTEXT("PromptTitle", "버튼 프롬프트(UDrButtonPrompt) · 키캡 칩(패드 얼굴 버튼은 원) + 행동 이름. 자동 줄은 키보드·패드를 쓰면 바뀜"), Prompts));
  return Page;
}

void UDrGlassGallery::HandleUndo(int32) {
  if (Toasts) Toasts->Push(LOCTEXT("Undone", "되돌렸어요"), EDrNoticeTone::Success);
}

void UDrGlassGallery::HandleSampleSnackbar(int32) {
  // 견본은 물러난 뒤 같은 내용으로 다시 뜬다(차례 줄에 들어가 있다가).
  if (SampleSnackbar) SampleSnackbar->Show(LOCTEXT("SnackSample", "메일 1개를 보관했어요"), LOCTEXT("Undo", "되돌리기"), 0.f);
}

void UDrGlassGallery::HandleSegmentDemo(int32 Index) {
  if (SegmentDemoValue && SegmentDemoOptions.IsValidIndex(Index))
    SegmentDemoValue->SetText(FText::Format(LOCTEXT("SegmentValue", "고른 값: {0}"), SegmentDemoOptions[Index]));
}

void UDrGlassGallery::HandleSliderDemo(float) {
  // 어느 슬라이더가 바뀌었는지 이벤트가 알려 주지 않으므로 전부 다시 쓴다(몇 개뿐).
  for (const auto& [Slider, Text] : SliderDemoValues) if (Slider && Text) Text->SetText(FText::AsPercent(Slider->GetValue()));
}

void UDrGlassGallery::HandleTransparency(float Value) {
  if (UDrUIThemeSubsystem* S = UDrUIThemeSubsystem::Get()) S->SetSurfaceTransparency(Value);
}

void UDrGlassGallery::HandleThemeChoice(int32 Index) {
  static const FName Ids[] = {TEXT("glass-light"), TEXT("glass-dark")};
  if (UDrUIThemeSubsystem* S = UDrUIThemeSubsystem::Get(); S && Index >= 0 && Index < UE_ARRAY_COUNT(Ids)) S->SetTheme(Ids[Index]);
}

void UDrGlassGallery::HandleQualityChoice(int32 Index) {
  static const EDrUISurfaceQuality Levels[] = {EDrUISurfaceQuality::High, EDrUISurfaceQuality::Medium, EDrUISurfaceQuality::Low};
  if (UDrUIThemeSubsystem* S = UDrUIThemeSubsystem::Get(); S && Index >= 0 && Index < UE_ARRAY_COUNT(Levels)) S->SetSurfaceQuality(Levels[Index]);
}

void UDrGlassGallery::Build() {
  UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>();
  WidgetTree->RootWidget = Root;
  if (UOverlaySlot* S = Root->AddChildToOverlay(CreateWidget<UDrGlassBackdrop>(this, UDrGlassBackdrop::StaticClass()))) { S->SetHorizontalAlignment(HAlign_Fill); S->SetVerticalAlignment(VAlign_Fill); }
  UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>();
  if (UOverlaySlot* S = Root->AddChildToOverlay(Column)) { S->SetHorizontalAlignment(HAlign_Fill); S->SetVerticalAlignment(VAlign_Fill); S->SetPadding(FMargin(32)); }

  // 위 조작 바(유리, 흐림 있음).
  UDrSurface* Bar = WidgetTree->ConstructWidget<UDrSurface>();
  Bar->RadiusToken = TEXT("radius-pill");
  Bar->PaddingToken = TEXT("space-3");
  UHorizontalBox* BarRow = WidgetTree->ConstructWidget<UHorizontalBox>();
  Bar->SetContent(BarRow);
  auto BarAdd = [BarRow](UWidget* W, bool bFill = false) {
    if (UHorizontalBoxSlot* S = BarRow->AddChildToHorizontalBox(W)) { S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(6, 0)); if (bFill) S->SetSize(FSlateChildSize(ESlateSizeRule::Fill)); }
  };
  BarAdd(MakeText(LOCTEXT("GalleryTitle", "DrUIKit 갤러리"), TEXT("heading")));
  BarAdd(WidgetTree->ConstructWidget<USpacer>(), true);
  ThemeChoice = WidgetTree->ConstructWidget<UDrSegmentedControl>();
  ThemeChoice->SetOptions({LOCTEXT("Light", "밝게"), LOCTEXT("Dark", "어둡게")});
  ThemeChoice->OnSelectionChanged.AddUniqueDynamic(this, &ThisClass::HandleThemeChoice);
  BarAdd(ThemeChoice);
  QualityChoice = WidgetTree->ConstructWidget<UDrSegmentedControl>();
  QualityChoice->SetOptions({LOCTEXT("High", "품질 높음"), LOCTEXT("Medium", "보통"), LOCTEXT("Low", "낮음")});
  QualityChoice->OnSelectionChanged.AddUniqueDynamic(this, &ThisClass::HandleQualityChoice);
  BarAdd(QualityChoice);
  // 표면 투명도(0 불투명 ~ .5 테마 그대로 ~ 1 가장 투명). 유리 바·카드가 함께 바뀐다.
  BarAdd(MakeText(LOCTEXT("Transparency", "투명도"), TEXT("label"), TEXT("ink-muted")));
  UDrSlider* Transparency = TransparencySlider = WidgetTree->ConstructWidget<UDrSlider>();
  Transparency->SetValue(UDrUIThemeSubsystem::Get() ? UDrUIThemeSubsystem::Get()->GetSurfaceTransparency() : .5f);
  Transparency->OnValueChanged.AddUniqueDynamic(this, &ThisClass::HandleTransparency);
  USizeBox* TransparencySize = WidgetTree->ConstructWidget<USizeBox>();
  TransparencySize->SetWidthOverride(140.f);
  TransparencySize->AddChild(Transparency);
  BarAdd(TransparencySize);
  MotionSwitch = WidgetTree->ConstructWidget<UDrSwitch>();
  MotionSwitch->Label = LOCTEXT("ReduceMotion", "움직임 줄이기");
  MotionSwitch->OnClicked().AddWeakLambda(this, [this] { if (UDrUIThemeSubsystem* S = UDrUIThemeSubsystem::Get()) S->SetReduceMotion(MotionSwitch->IsOn()); });
  BarAdd(MotionSwitch);
  if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(Bar)) S->SetPadding(FMargin(0, 0, 0, 16));

  // 탭(알약형 UDrTabList) + 페이지(UCommonAnimatedSwitcher).
  PageTabs = WidgetTree->ConstructWidget<UDrTabList>();
  if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(PageTabs)) { S->SetHorizontalAlignment(HAlign_Left); S->SetPadding(FMargin(0, 0, 0, 16)); }
  Pages = WidgetTree->ConstructWidget<UCommonAnimatedSwitcher>();
  if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(Pages)) S->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
  PageTabs->SetLinkedSwitcher(Pages);
  const TArray<TPair<FText, UWidget*>> PageList = {
    {LOCTEXT("PageFoundation", "기초: 글자·색·표면·아이콘"), BuildFoundationPage()},
    {LOCTEXT("PageButtons", "버튼: 4종 × 상태"), BuildButtonPage()},
    {LOCTEXT("PageSelection", "선택: 체크·라디오·스위치·슬라이더·세그먼트·탭"), BuildSelectionPage()},
    {LOCTEXT("PageStatus", "상태: 배지·진행·스켈레톤·스피너"), BuildStatusPage()},
    {LOCTEXT("PageNotice", "알림: 토스트·스낵바"), BuildNoticePage()},
    {LOCTEXT("PageInput", "입력: 텍스트 필드·드롭다운·수평 선택기"), BuildInputPage()},
    {LOCTEXT("PageGame", "게임용: 홀드 버튼·파이 메뉴"), BuildGamePage()},
    {LOCTEXT("PageNavigation", "탐색·표시: 툴팁·목록 행·페이지 점·프롬프트"), BuildNavigationPage()}};
  for (int32 I = 0; I < PageList.Num(); ++I) {
    UScrollBox* Scroll = WidgetTree->ConstructWidget<UScrollBox>();
    Scroll->AddChild(PageList[I].Value);
    Pages->AddChild(Scroll);
    PageTabs->AddTab(PageId(I), PageList[I].Key, Scroll);
  }

  // 떠 있는 층: 토스트는 위 가운데, 스낵바는 아래 가운데(갤러리 전체 위).
  Toasts = WidgetTree->ConstructWidget<UDrToastStack>();
  if (UOverlaySlot* S = Root->AddChildToOverlay(Toasts)) { S->SetHorizontalAlignment(HAlign_Center); S->SetVerticalAlignment(VAlign_Top); S->SetPadding(FMargin(0, 24, 0, 0)); }
  Snackbar = WidgetTree->ConstructWidget<UDrSnackbar>();
  Snackbar->OnAction.AddUniqueDynamic(this, &ThisClass::HandleUndo);
  if (UOverlaySlot* S = Root->AddChildToOverlay(Snackbar)) { S->SetHorizontalAlignment(HAlign_Center); S->SetVerticalAlignment(VAlign_Bottom); S->SetPadding(FMargin(0, 0, 0, 32)); }
}

int32 UDrGlassGallery::GetPageCount() const { return Pages ? Pages->GetNumWidgets() : 0; }

void UDrGlassGallery::ShowPage(int32 Index) {
  if (PageTabs && Index >= 0 && Index < GetPageCount()) PageTabs->SelectTabByID(PageId(Index), true);
}

TSharedRef<SWidget> UDrGlassGallery::RebuildWidget() {
  // 트리는 여기서 만든다(NativeOnInitialized는 이보다 먼저 불려 부품이 아직 없다). 첫 선택·캡처 준비도 만든 직후에.
  if (WidgetTree && !WidgetTree->RootWidget) {
    Build();
    ShowPage(0);
    if (ThemeChoice) ThemeChoice->SetSelectedIndex(0);
    if (QualityChoice) QualityChoice->SetSelectedIndex(0);
    if (FParse::Param(FCommandLine::Get(), TEXT("DrUIGalleryCapture")) && !CaptureTicker.IsValid()) {
      // 캡처는 몇십 프레임만 기다리므로 움직임(인디케이터·페이지 전환)을 끄고 찍는다. 스위치도 켜진 모습으로 맞춘다.
      if (UDrUIThemeSubsystem* S = UDrUIThemeSubsystem::Get()) S->SetReduceMotion(true);
      if (MotionSwitch) MotionSwitch->SetOn(true);
      if (Pages) Pages->SetDisableTransitionAnimation(true);
      CaptureTicker = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &ThisClass::CaptureStep));
    }
  }
  return Super::RebuildWidget();
}

void UDrGlassGallery::NativeDestruct() {
  FTSTicker::GetCoreTicker().RemoveTicker(CaptureTicker);
  Super::NativeDestruct();
}

bool UDrGlassGallery::CaptureStep(float DeltaTime) {
  // 테마(밝게·어둡게) × 페이지마다: 적용 → 30프레임 기다림(레이아웃·글꼴 안정) → 찍기.
  // 이어서 밝은 테마 첫 페이지를 투명도 0·1로 한 장씩(투명도 조절 확인). 다 찍으면 종료.
  // 시간이 아니라 프레임으로 센다: 화면 없이 렌더링(-RenderOffscreen)하면 틱 시간이 실제와 맞지 않았다.
  static const FName ThemeIds[] = {TEXT("glass-light"), TEXT("glass-dark")};
  static const float Transparencies[] = {0.f, 1.f};
  const int32 ThemeShots = UE_ARRAY_COUNT(ThemeIds) * GetPageCount();
  const int32 Total = ThemeShots + UE_ARRAY_COUNT(Transparencies);
  CaptureWait -= 1.f;
  if (CaptureWait > 0.f) return true;
  if (CaptureIndex >= Total) { FPlatformMisc::RequestExit(false, TEXT("DrUIGalleryCapture")); return false; }
  const bool bTransparencyShot = CaptureIndex >= ThemeShots;
  const int32 ThemeIndex = bTransparencyShot ? 0 : CaptureIndex / GetPageCount();
  const FName Theme = ThemeIds[ThemeIndex];
  const int32 Page = bTransparencyShot ? 0 : CaptureIndex % GetPageCount();
  const FString ShotName = bTransparencyShot ? FString::Printf(TEXT("DrUIGallery_transparency_%.0f"), Transparencies[CaptureIndex - ThemeShots])
                                             : FString::Printf(TEXT("DrUIGallery_%s_%d"), *Theme.ToString(), Page);
  if (!bCaptureShotPending) {
    if (ThemeChoice) ThemeChoice->SetSelectedIndex(ThemeIndex);  // HandleThemeChoice가 테마를 바꾼다
    if (bTransparencyShot) {
      const float Value = Transparencies[CaptureIndex - ThemeShots];
      if (TransparencySlider) TransparencySlider->SetValue(Value);  // SetValue는 OnValueChanged를 부르지 않는다
      HandleTransparency(Value);
    }
    ShowPage(Page);
    bCaptureShotPending = true;
    CaptureWait = 30.f;
    return true;
  }
  FScreenshotRequest::RequestScreenshot(FPaths::ScreenShotDir() / (ShotName + TEXT(".png")), true, false);
  bCaptureShotPending = false;
  ++CaptureIndex;
  CaptureWait = 10.f;
  return true;
}

#undef LOCTEXT_NAMESPACE
