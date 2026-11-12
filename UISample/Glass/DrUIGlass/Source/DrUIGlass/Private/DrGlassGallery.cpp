#include "DrGlassGallery.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WidgetSwitcher.h"
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
#include "Widgets/DrUIIcon.h"
#include "Widgets/DrUISurface.h"
#include "Widgets/DrUIText.h"

#define LOCTEXT_NAMESPACE "DrGlassGallery"

TArray<UClass*> UDrGlassGallery::CoveredClasses() {
  return {UDrUIText::StaticClass(), UDrUIIcon::StaticClass(), UDrUISurface::StaticClass(), UDrUIButton::StaticClass()};
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

void UDrGlassGallery::SelectOnly(const TArray<TObjectPtr<UDrUIButton>>& Group, UDrUIButton* Selected) {
  for (UDrUIButton* Button : Group) if (Button) Button->SetIsSelected(Button == Selected, false);
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
  for (const auto& [Id, Label] : TArray<TPair<FName, FText>>{{TEXT("glass-light"), LOCTEXT("Light", "밝게")}, {TEXT("glass-dark"), LOCTEXT("Dark", "어둡게")}}) {
    UDrUIButton* Button = MakeButton(Label);
    Button->SetIsSelectable(true);
    Button->OnClicked().AddWeakLambda(this, [this, Button, Id] { if (UDrUIThemeSubsystem* S = UDrUIThemeSubsystem::Get()) S->SetTheme(Id); SelectOnly(ThemeButtons, Button); });
    ThemeButtons.Add(Button);
    BarAdd(Button);
  }
  for (const auto& [Quality, Label] : TArray<TPair<EDrUISurfaceQuality, FText>>{{EDrUISurfaceQuality::High, LOCTEXT("High", "품질 높음")}, {EDrUISurfaceQuality::Medium, LOCTEXT("Medium", "보통")}, {EDrUISurfaceQuality::Low, LOCTEXT("Low", "낮음")}}) {
    UDrUIButton* Button = MakeButton(Label, EDrUIButtonVariant::Ghost);
    Button->SetIsSelectable(true);
    Button->OnClicked().AddWeakLambda(this, [this, Button, Quality] { if (UDrUIThemeSubsystem* S = UDrUIThemeSubsystem::Get()) S->SetSurfaceQuality(Quality); SelectOnly(QualityButtons, Button); });
    QualityButtons.Add(Button);
    BarAdd(Button);
  }
  UDrUIButton* Motion = MakeButton(LOCTEXT("ReduceMotion", "움직임 줄이기"), EDrUIButtonVariant::Ghost);
  Motion->SetIsToggleable(true);
  Motion->OnClicked().AddWeakLambda(this, [Motion] { if (UDrUIThemeSubsystem* S = UDrUIThemeSubsystem::Get()) S->SetReduceMotion(Motion->GetSelected()); });
  BarAdd(Motion);
  if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(Bar)) S->SetPadding(FMargin(0, 0, 0, 16));

  // 탭 + 페이지.
  UHorizontalBox* Tabs = WidgetTree->ConstructWidget<UHorizontalBox>();
  if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(Tabs)) S->SetPadding(FMargin(0, 0, 0, 16));
  Pages = WidgetTree->ConstructWidget<UWidgetSwitcher>();
  if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(Pages)) S->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
  const TArray<TPair<FText, UWidget*>> PageList = {
    {LOCTEXT("PageFoundation", "기초: 글자·색·표면·아이콘"), BuildFoundationPage()},
    {LOCTEXT("PageButtons", "버튼: 4종 × 상태"), BuildButtonPage()}};
  for (int32 I = 0; I < PageList.Num(); ++I) {
    UScrollBox* Scroll = WidgetTree->ConstructWidget<UScrollBox>();
    Scroll->AddChild(PageList[I].Value);
    Pages->AddChild(Scroll);
    PageNames.Add(PageList[I].Key);
    UDrUIButton* Tab = MakeButton(PageList[I].Key);
    Tab->SetIsSelectable(true);
    Tab->OnClicked().AddWeakLambda(this, [this, I] { ShowPage(I); });
    TabButtons.Add(Tab);
    if (UHorizontalBoxSlot* S = Tabs->AddChildToHorizontalBox(Tab)) S->SetPadding(FMargin(0, 0, 8, 0));
  }
}

int32 UDrGlassGallery::GetPageCount() const { return Pages ? Pages->GetNumWidgets() : 0; }

void UDrGlassGallery::ShowPage(int32 Index) {
  if (!Pages || !TabButtons.IsValidIndex(Index)) return;
  Pages->SetActiveWidgetIndex(Index);
  SelectOnly(TabButtons, TabButtons[Index]);
}

TSharedRef<SWidget> UDrGlassGallery::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) Build();
  return Super::RebuildWidget();
}

void UDrGlassGallery::NativeOnInitialized() {
  Super::NativeOnInitialized();
  ShowPage(0);
  if (ThemeButtons.Num()) SelectOnly(ThemeButtons, ThemeButtons[0]);
  if (QualityButtons.Num()) SelectOnly(QualityButtons, QualityButtons[0]);
  if (FParse::Param(FCommandLine::Get(), TEXT("DrUIGalleryCapture")))
    CaptureTicker = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &ThisClass::CaptureStep));
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
    if (UDrUIThemeSubsystem* S = UDrUIThemeSubsystem::Get()) S->SetTheme(Theme);
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
