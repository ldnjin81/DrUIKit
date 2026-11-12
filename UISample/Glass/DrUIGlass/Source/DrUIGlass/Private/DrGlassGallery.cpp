#include "DrGlassGallery.h"
#include "Blueprint/WidgetTree.h"
#include "CommonAnimatedSwitcher.h"
#include "Groups/CommonButtonGroupBase.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WrapBox.h"
#include "DrGlassBackdrop.h"
#include "Engine/World.h"
#include "HAL/PlatformMisc.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "TimerManager.h"
#include "UnrealClient.h"
#include "Widgets/DrUIChoice.h"
#include "Widgets/DrUIGlyph.h"
#include "Widgets/DrUIIcon.h"
#include "Widgets/DrUISurface.h"
#include "Widgets/DrUITabList.h"
#include "Widgets/DrUIText.h"

#define LOCTEXT_NAMESPACE "DrGlassGallery"

TArray<UClass*> UDrGlassGallery::CoveredClasses() {
  return {UDrUIText::StaticClass(), UDrUIIcon::StaticClass(), UDrUISurface::StaticClass(), UDrUIButton::StaticClass(),
    UDrUIGlyph::StaticClass(), UDrUICheckBox::StaticClass(), UDrUIRadio::StaticClass(), UDrUISwitch::StaticClass(),
    UDrUITabButton::StaticClass(), UDrUITabList::StaticClass(), UDrUISegmented::StaticClass()};
}

UDrUIText* UDrGlassGallery::MakeText(const FText& Text, FName Style, FName Color) {
  UDrUIText* Widget = WidgetTree->ConstructWidget<UDrUIText>();
  Widget->TextStyle = Style;
  Widget->ColorToken = Color;
  Widget->SetText(Text);
  return Widget;
}

UDrUIButton* UDrGlassGallery::MakeButton(const FText& Label, EDrUIButtonVariant Variant) {
  UDrUIButton* Button = WidgetTree->ConstructWidget<UDrUIButton>();
  Button->Label = Label;
  Button->Variant = Variant;
  return Button;
}

UWidget* UDrGlassGallery::Section(const FText& Title, UWidget* Body) {
  UDrUISurface* Card = WidgetTree->ConstructWidget<UDrUISurface>();
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
  Add(Section(LOCTEXT("TypeTitle", "글자 스타일(UDrUIText)"), Type));

  UWrapBox* Swatches = WidgetTree->ConstructWidget<UWrapBox>();
  Swatches->SetInnerSlotPadding(FVector2D(12, 12));
  for (const TCHAR* Token : {TEXT("ground"), TEXT("glass"), TEXT("glass-strong"), TEXT("ink"), TEXT("ink-muted"), TEXT("accent"), TEXT("accent-soft"), TEXT("chip"), TEXT("ember"), TEXT("success"), TEXT("warning"), TEXT("danger")}) {
    UVerticalBox* Cell = WidgetTree->ConstructWidget<UVerticalBox>();
    UDrUISurface* Chip = WidgetTree->ConstructWidget<UDrUISurface>();
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
  Add(Section(LOCTEXT("ColorTitle", "색 토큰(UDrUISurface 채움)"), Swatches));

  UHorizontalBox* Surfaces = WidgetTree->ConstructWidget<UHorizontalBox>();
  for (const auto& [Fill, Caption] : TArray<TPair<FName, FText>>{{TEXT("glass"), LOCTEXT("Glass", "glass + 흐림(유리 패널)")}, {TEXT("glass-strong"), LOCTEXT("Strong", "glass-strong(글이 많은 곳)")}}) {
    UDrUISurface* Panel = WidgetTree->ConstructWidget<UDrUISurface>();
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
    UDrUIIcon* Icon = WidgetTree->ConstructWidget<UDrUIIcon>();
    Icon->ColorToken = Token;
    if (UHorizontalBoxSlot* S = Icons->AddChildToHorizontalBox(Icon)) S->SetPadding(FMargin(0, 0, 12, 0));
  }
  Icons->AddChildToHorizontalBox(MakeText(LOCTEXT("IconNote", "아이콘 모양(텍스처)은 게임이 지정한다. 여기서는 색 토큰만 확인."), TEXT("caption"), TEXT("ink-muted")));
  Add(Section(LOCTEXT("IconTitle", "아이콘(UDrUIIcon)"), Icons));
  return Page;
}

UWidget* UDrGlassGallery::BuildButtonPage() {
  UVerticalBox* Page = WidgetTree->ConstructWidget<UVerticalBox>();
  const TArray<TPair<EDrUIButtonVariant, FText>> Variants = {
    {EDrUIButtonVariant::Primary, LOCTEXT("Primary", "Primary")}, {EDrUIButtonVariant::Secondary, LOCTEXT("Secondary", "Secondary")},
    {EDrUIButtonVariant::Ghost, LOCTEXT("Ghost", "Ghost")}, {EDrUIButtonVariant::Destructive, LOCTEXT("Destructive", "Destructive")}};
  for (const auto& [Variant, Name] : Variants) {
    UWrapBox* Row = WidgetTree->ConstructWidget<UWrapBox>();
    Row->SetInnerSlotPadding(FVector2D(12, 12));
    // 1) 직접 눌러 보는 버튼: 누르면 로딩 1초 → 성공.
    UDrUIButton* Live = MakeButton(Variant == EDrUIButtonVariant::Destructive ? LOCTEXT("Delete", "삭제") : LOCTEXT("Save", "저장"), Variant);
    Live->OnClicked().AddWeakLambda(this, [this, Live] {
      Live->SetLoading(true);
      FTimerHandle Handle;
      if (UWorld* World = GetWorld()) World->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateWeakLambda(Live, [Live] { Live->ShowSuccess(LOCTEXT("Saved", "✓ 저장했어요")); }), 1.f, false);
    });
    Row->AddChild(Live);
    // 2) 상태 고정 견본(호버·눌림·포커스는 마우스·키보드로 직접 확인).
    UDrUIButton* Disabled = MakeButton(LOCTEXT("DisabledLabel", "비활성"), Variant); Disabled->SetIsEnabled(false); Row->AddChild(Disabled);
    UDrUIButton* Loading = MakeButton(LOCTEXT("LoadingLabel", "저장 중"), Variant); Row->AddChild(Loading);
    UDrUIButton* Error = MakeButton(LOCTEXT("ErrorLabel", "오류"), Variant); Row->AddChild(Error);
    UDrUIButton* Selected = MakeButton(LOCTEXT("SelectedLabel", "✓ 선택됨"), Variant); Selected->SetIsSelectable(true); Row->AddChild(Selected);
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
    UDrUICheckBox* Check = WidgetTree->ConstructWidget<UDrUICheckBox>();
    Check->Label = Label;
    Check->SetOn(bOn);
    Check->SetIsEnabled(bEnabled);
    Checks->AddChild(Check);
  }
  Add(Section(LOCTEXT("CheckTitle", "체크박스(UDrUICheckBox) · 켜면 체크를 그린다"), Checks));

  // 라디오: CommonUI 버튼 묶음이 하나만 선택을 맡는다.
  UWrapBox* Radios = MakeRow();
  UCommonButtonGroupBase* Group = NewObject<UCommonButtonGroupBase>(this);
  Group->SetSelectionRequired(true);
  RadioGroups.Add(Group);
  for (const FText& Label : {LOCTEXT("RadioEasy", "쉬움"), LOCTEXT("RadioNormal", "보통"), LOCTEXT("RadioHard", "어려움")}) {
    UDrUIRadio* Radio = WidgetTree->ConstructWidget<UDrUIRadio>();
    Radio->Label = Label;
    Group->AddWidget(Radio);
    Radios->AddChild(Radio);
  }
  Group->SelectButtonAtIndex(1, false);
  UDrUIRadio* RadioDisabled = WidgetTree->ConstructWidget<UDrUIRadio>();
  RadioDisabled->Label = LOCTEXT("RadioDisabled", "비활성");
  RadioDisabled->SetIsEnabled(false);
  Radios->AddChild(RadioDisabled);
  Add(Section(LOCTEXT("RadioTitle", "라디오(UDrUIRadio + UCommonButtonGroupBase) · 하나만 선택"), Radios));

  // 스위치.
  UWrapBox* Switches = MakeRow();
  for (const auto& [Label, bOn, bEnabled] : TArray<TTuple<FText, bool, bool>>{
         {LOCTEXT("SwitchLive", "진동"), false, true}, {LOCTEXT("SwitchOn", "자막"), true, true}, {LOCTEXT("SwitchDisabled", "비활성"), true, false}}) {
    UDrUISwitch* Switch = WidgetTree->ConstructWidget<UDrUISwitch>();
    Switch->Label = Label;
    Switch->SetOn(bOn);
    Switch->SetIsEnabled(bEnabled);
    Switches->AddChild(Switch);
  }
  Add(Section(LOCTEXT("SwitchTitle", "스위치(UDrUISwitch) · 손잡이가 스프링으로 미끄러진다"), Switches));

  // 세그먼트: 고른 값을 옆 글자로 보여 준다.
  UHorizontalBox* SegmentRow = WidgetTree->ConstructWidget<UHorizontalBox>();
  UDrUISegmented* Segmented = WidgetTree->ConstructWidget<UDrUISegmented>();
  const TArray<FText> Ranges = {LOCTEXT("Day", "일"), LOCTEXT("Week", "주"), LOCTEXT("Month", "월"), LOCTEXT("Year", "년")};
  Segmented->SetOptions(Ranges);
  USizeBox* SegmentSize = WidgetTree->ConstructWidget<USizeBox>();
  SegmentSize->SetWidthOverride(320.f);
  SegmentSize->AddChild(Segmented);
  if (UHorizontalBoxSlot* S = SegmentRow->AddChildToHorizontalBox(SegmentSize)) S->SetVerticalAlignment(VAlign_Center);
  UDrUIText* SegmentValue = MakeText(FText::GetEmpty(), TEXT("body"), TEXT("ink-muted"));
  if (UHorizontalBoxSlot* S = SegmentRow->AddChildToHorizontalBox(SegmentValue)) { S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(16, 0, 0, 0)); }
  Segmented->OnSelectionChanged.AddUniqueDynamic(this, &ThisClass::HandleSegmentDemo);
  SegmentDemoValue = SegmentValue;
  SegmentDemoOptions = Ranges;
  Segmented->SetSelectedIndex(1);
  Add(Section(LOCTEXT("SegmentTitle", "세그먼트(UDrUISegmented) · 값 하나 고르기"), SegmentRow));

  // 탭(밑줄형): 위 페이지 탭은 알약형. 밑줄형은 작은 스위처에 붙여 확인한다.
  UVerticalBox* TabDemo = WidgetTree->ConstructWidget<UVerticalBox>();
  UDrUITabList* Underline = WidgetTree->ConstructWidget<UDrUITabList>();
  Underline->Style = EDrUITabStyle::Underline;
  UCommonAnimatedSwitcher* TabPages = WidgetTree->ConstructWidget<UCommonAnimatedSwitcher>();
  Underline->SetLinkedSwitcher(TabPages);
  for (const auto& [Id, Label, Body] : TArray<TTuple<FName, FText, FText>>{
         {TEXT("Graphics"), LOCTEXT("TabGraphics", "그래픽"), LOCTEXT("TabGraphicsBody", "그래픽 설정 내용")},
         {TEXT("Sound"), LOCTEXT("TabSound", "소리"), LOCTEXT("TabSoundBody", "소리 설정 내용")},
         {TEXT("Controls"), LOCTEXT("TabControls", "조작"), LOCTEXT("TabControlsBody", "조작 설정 내용(패드는 탭 목록의 이전·다음 입력으로 넘긴다)")}}) {
    UDrUIText* Content = MakeText(Body, TEXT("body"), TEXT("ink-muted"));
    TabPages->AddChild(Content);
    Underline->AddTab(Id, Label, Content);
  }
  if (UVerticalBoxSlot* S = TabDemo->AddChildToVerticalBox(Underline)) { S->SetHorizontalAlignment(HAlign_Left); S->SetPadding(FMargin(0, 0, 0, 12)); }
  TabDemo->AddChildToVerticalBox(TabPages);
  Add(Section(LOCTEXT("TabTitle", "탭(UDrUITabList, UDrUITabButton) · 밑줄형. 위 페이지 탭이 알약형"), TabDemo));

  // 표시(UDrUIGlyph): 모든 모양.
  UWrapBox* Glyphs = MakeRow();
  for (const EDrUIGlyphShape Shape : {EDrUIGlyphShape::Check, EDrUIGlyphShape::Dot, EDrUIGlyphShape::Minus, EDrUIGlyphShape::Plus, EDrUIGlyphShape::Close,
         EDrUIGlyphShape::ChevronLeft, EDrUIGlyphShape::ChevronRight, EDrUIGlyphShape::ChevronDown}) {
    UDrUIGlyph* Glyph = WidgetTree->ConstructWidget<UDrUIGlyph>();
    Glyph->Shape = Shape;
    Glyph->GlyphSize = 24.f;
    Glyphs->AddChild(Glyph);
  }
  Add(Section(LOCTEXT("GlyphTitle", "표시(UDrUIGlyph) · 에셋 없이 선으로 그림"), Glyphs));
  return Page;
}

void UDrGlassGallery::HandleSegmentDemo(int32 Index) {
  if (SegmentDemoValue && SegmentDemoOptions.IsValidIndex(Index))
    SegmentDemoValue->SetText(FText::Format(LOCTEXT("SegmentValue", "고른 값: {0}"), SegmentDemoOptions[Index]));
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
  UDrUISurface* Bar = WidgetTree->ConstructWidget<UDrUISurface>();
  Bar->RadiusToken = TEXT("radius-pill");
  Bar->PaddingToken = TEXT("space-3");
  UHorizontalBox* BarRow = WidgetTree->ConstructWidget<UHorizontalBox>();
  Bar->SetContent(BarRow);
  auto BarAdd = [BarRow](UWidget* W, bool bFill = false) {
    if (UHorizontalBoxSlot* S = BarRow->AddChildToHorizontalBox(W)) { S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(6, 0)); if (bFill) S->SetSize(FSlateChildSize(ESlateSizeRule::Fill)); }
  };
  BarAdd(MakeText(LOCTEXT("GalleryTitle", "DrUIKit 갤러리"), TEXT("heading")));
  BarAdd(WidgetTree->ConstructWidget<USpacer>(), true);
  ThemeChoice = WidgetTree->ConstructWidget<UDrUISegmented>();
  ThemeChoice->SetOptions({LOCTEXT("Light", "밝게"), LOCTEXT("Dark", "어둡게")});
  ThemeChoice->OnSelectionChanged.AddUniqueDynamic(this, &ThisClass::HandleThemeChoice);
  BarAdd(ThemeChoice);
  QualityChoice = WidgetTree->ConstructWidget<UDrUISegmented>();
  QualityChoice->SetOptions({LOCTEXT("High", "품질 높음"), LOCTEXT("Medium", "보통"), LOCTEXT("Low", "낮음")});
  QualityChoice->OnSelectionChanged.AddUniqueDynamic(this, &ThisClass::HandleQualityChoice);
  BarAdd(QualityChoice);
  MotionSwitch = WidgetTree->ConstructWidget<UDrUISwitch>();
  MotionSwitch->Label = LOCTEXT("ReduceMotion", "움직임 줄이기");
  MotionSwitch->OnClicked().AddWeakLambda(this, [this] { if (UDrUIThemeSubsystem* S = UDrUIThemeSubsystem::Get()) S->SetReduceMotion(MotionSwitch->IsOn()); });
  BarAdd(MotionSwitch);
  if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(Bar)) S->SetPadding(FMargin(0, 0, 0, 16));

  // 탭(알약형 UDrUITabList) + 페이지(UCommonAnimatedSwitcher).
  PageTabs = WidgetTree->ConstructWidget<UDrUITabList>();
  if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(PageTabs)) { S->SetHorizontalAlignment(HAlign_Left); S->SetPadding(FMargin(0, 0, 0, 16)); }
  Pages = WidgetTree->ConstructWidget<UCommonAnimatedSwitcher>();
  if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(Pages)) S->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
  PageTabs->SetLinkedSwitcher(Pages);
  const TArray<TPair<FText, UWidget*>> PageList = {
    {LOCTEXT("PageFoundation", "기초: 글자·색·표면·아이콘"), BuildFoundationPage()},
    {LOCTEXT("PageButtons", "버튼: 4종 × 상태"), BuildButtonPage()},
    {LOCTEXT("PageSelection", "선택: 체크·라디오·스위치·세그먼트·탭"), BuildSelectionPage()}};
  for (int32 I = 0; I < PageList.Num(); ++I) {
    UScrollBox* Scroll = WidgetTree->ConstructWidget<UScrollBox>();
    Scroll->AddChild(PageList[I].Value);
    Pages->AddChild(Scroll);
    PageTabs->AddTab(PageId(I), PageList[I].Key, Scroll);
  }
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
  // 테마(밝게·어둡게) × 페이지마다: 적용 → 30프레임 기다림(레이아웃·글꼴 안정) → 찍기. 다 찍으면 종료.
  // 시간이 아니라 프레임으로 센다: 화면 없이 렌더링(-RenderOffscreen)하면 틱 시간이 실제와 맞지 않았다.
  static const FName ThemeIds[] = {TEXT("glass-light"), TEXT("glass-dark")};
  const int32 Total = UE_ARRAY_COUNT(ThemeIds) * GetPageCount();
  CaptureWait -= 1.f;
  if (CaptureWait > 0.f) return true;
  if (CaptureIndex >= Total) { FPlatformMisc::RequestExit(false, TEXT("DrUIGalleryCapture")); return false; }
  const FName Theme = ThemeIds[CaptureIndex / GetPageCount()];
  const int32 Page = CaptureIndex % GetPageCount();
  if (!bCaptureShotPending) {
    if (ThemeChoice) ThemeChoice->SetSelectedIndex(CaptureIndex / GetPageCount());  // HandleThemeChoice가 테마를 바꾼다
    ShowPage(Page);
    bCaptureShotPending = true;
    CaptureWait = 30.f;
    return true;
  }
  FScreenshotRequest::RequestScreenshot(FPaths::ScreenShotDir() / FString::Printf(TEXT("DrUIGallery_%s_%d.png"), *Theme.ToString(), Page), true, false);
  bCaptureShotPending = false;
  ++CaptureIndex;
  CaptureWait = 10.f;
  return true;
}

#undef LOCTEXT_NAMESPACE
