#include "DrGlassConsoleMenu.h"
#include "Blueprint/WidgetTree.h"
#include "CommonButtonBase.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Core/DrUIFeedback.h"
#include "DrGlassBackdrop.h"
#include "Flow/DrUIFlowSubsystem.h"
#include "Groups/CommonButtonGroupBase.h"
#include "Theme/DrUITheme.h"
#include "Widgets/DrActionBar.h"
#include "Widgets/DrButton.h"
#include "Widgets/DrHoldButton.h"
#include "Widgets/DrInput.h"
#include "Widgets/DrListRow.h"
#include "Widgets/DrOptionRow.h"
#include "Widgets/DrQuantityStepper.h"
#include "Widgets/DrScrollBox.h"
#include "Widgets/DrSlider.h"
#include "Widgets/DrSurface.h"
#include "Widgets/DrTabList.h"
#include "Widgets/DrText.h"
#include "Widgets/DrToggle.h"

#define LOCTEXT_NAMESPACE "DrGlassConsoleMenu"

namespace {
const TCHAR* MainNames[] = {TEXT("장비"), TEXT("소지품"), TEXT("스킬"), TEXT("설정")};
const TCHAR* SubNames[4][3] = {{TEXT("무기"), TEXT("방어구"), TEXT("장신구")}, {TEXT("소모품"), TEXT("재료"), TEXT("퀘스트")},
                               {TEXT("전투"), TEXT("마법"), TEXT("생활")}, {TEXT("게임"), TEXT("화면"), TEXT("소리")}};
TArray<FString> Depth1Names(int32 Main, int32 Sub) {
  static const TArray<TArray<TArray<FString>>> Names = {
    {{TEXT("한손검"), TEXT("양손검"), TEXT("활"), TEXT("지팡이"), TEXT("단검")}, {TEXT("머리"), TEXT("몸통"), TEXT("손"), TEXT("다리"), TEXT("방패")}, {TEXT("반지"), TEXT("목걸이"), TEXT("귀걸이"), TEXT("허리띠")}},
    {{TEXT("회복약"), TEXT("마나약"), TEXT("음식"), TEXT("두루마리")}, {TEXT("광석"), TEXT("가죽"), TEXT("약초"), TEXT("보석")}, {TEXT("편지"), TEXT("열쇠"), TEXT("지도")}},
    {{TEXT("검술"), TEXT("방어술"), TEXT("궁술")}, {TEXT("불"), TEXT("얼음"), TEXT("번개"), TEXT("치유")}, {TEXT("채집"), TEXT("제작"), TEXT("요리")}},
    {{TEXT("난이도"), TEXT("자막"), TEXT("조작")}, {TEXT("밝기"), TEXT("해상도"), TEXT("글자 크기")}, {TEXT("음악"), TEXT("효과음"), TEXT("음성")}}};
  return Names[Main][Sub];
}
const TCHAR* RarityNames[] = {TEXT("보통"), TEXT("고급"), TEXT("희귀"), TEXT("영웅"), TEXT("전설")};
const TCHAR* Prefixes[] = {TEXT("낡은"), TEXT("튼튼한"), TEXT("정교한"), TEXT("빛나는"), TEXT("영웅의"), TEXT("전설의")};
const EDrItemRarity PrefixRarity[] = {EDrItemRarity::Common, EDrItemRarity::Common, EDrItemRarity::Uncommon, EDrItemRarity::Rare, EDrItemRarity::Epic, EDrItemRarity::Legendary};
const EDrGlyphShape MainGlyph[] = {EDrGlyphShape::Close, EDrGlyphShape::Dot, EDrGlyphShape::Alert, EDrGlyphShape::Check};
const FName EquipHint = TEXT("Equip"), DropHint = TEXT("Drop"), OpenHint = TEXT("Open");
}

FString UDrGlassConsoleMenu::Key() const { return FString::Printf(TEXT("%d/%d/%d"), MainIndex, SubIndex, Depth1Index); }

TArray<UDrGlassConsoleMenu::FEntry>& UDrGlassConsoleMenu::Entries() {
  // 샘플 내용: (메인·서브·1뎁스)마다 6개. 처음 것이 장착, 다섯 번째는 새로 얻음. 수치는 이름에서 정해진다(늘 같게).
  TArray<FEntry>& Found = Data.FindOrAdd(Key());
  if (!Found.IsEmpty()) return Found;
  const TArray<FString> Names = Depth1Names(MainIndex, SubIndex);
  const FString Base = Names.IsValidIndex(Depth1Index) ? Names[Depth1Index] : FString();
  for (int32 I = 0; I < UE_ARRAY_COUNT(Prefixes); ++I) {
    FEntry Entry;
    Entry.Item.Name = FText::FromString(FString::Printf(TEXT("%s %s"), Prefixes[I], *Base));
    Entry.Item.Rarity = PrefixRarity[I];
    Entry.Item.Glyph = MainGlyph[MainIndex];
    Entry.Item.Count = MainIndex == 1 ? 1 + (I * 7 + Depth1Index * 3) % 20 : 1;
    Entry.Item.bEquipped = I == 0;
    Entry.Item.bNew = I == 4;
    Entry.Description = FText::Format(LOCTEXT("Desc", "{0}. 샘플 설명이다 — 실제 게임은 데이터 테이블에서 읽어 온다."), Entry.Item.Name);
    const uint32 Hash = GetTypeHash(Base) + I * 2654435761u;
    Entry.Stats = {10.f + I * 6.f + Hash % 5, 4.f + (Hash >> 3) % 9 + I * 2.f, 3.f + (Hash >> 7) % 6 - I * .4f, 1.5f + I * .8f + ((Hash >> 11) % 10) * .1f};
    Found.Add(Entry);
  }
  return Found;
}

TSharedRef<SWidget> UDrGlassConsoleMenu::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) Build();
  return Super::RebuildWidget();
}

void UDrGlassConsoleMenu::Build() {
  UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>();
  WidgetTree->RootWidget = Root;
  if (UOverlaySlot* S = Root->AddChildToOverlay(CreateWidget<UDrGlassBackdrop>(this, UDrGlassBackdrop::StaticClass()))) { S->SetHorizontalAlignment(HAlign_Fill); S->SetVerticalAlignment(VAlign_Fill); }
  UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>();
  if (UOverlaySlot* S = Root->AddChildToOverlay(Column)) { S->SetHorizontalAlignment(HAlign_Fill); S->SetVerticalAlignment(VAlign_Fill); S->SetPadding(FMargin(40, 28)); }
  auto Text = [this](const FText& Value, FName Style, FName Color) {
    UDrText* T = WidgetTree->ConstructWidget<UDrText>();
    T->SetTokens(Style, Color);
    T->SetText(Value);
    return T;
  };
  auto Panel = [this](UWidget* Content) {
    UDrSurface* P = WidgetTree->ConstructWidget<UDrSurface>();
    P->RadiusToken = DrUITokens::RadiusXl;
    P->PaddingToken = TEXT("space-5");
    P->SetElevation(DrUITokens::ShadowGlass);
    P->SetHorizontalAlignment(HAlign_Fill);
    P->SetVerticalAlignment(VAlign_Fill);
    P->SetContent(Content);
    return P;
  };

  // 메인 탭(LB·RB)은 가운데, 서브 탭(LT·RT)은 그 아래 왼쪽.
  MainTabs = WidgetTree->ConstructWidget<UDrTabList>();
  MainTabs->SetPaging(EDrTabPaging::Bumpers);
  MainTabs->bFocusTabOnPaging = false;  // 넘긴 뒤 포커스는 메뉴가 1뎁스로 보낸다
  for (int32 I = 0; I < UE_ARRAY_COUNT(MainNames); ++I) MainTabs->AddTab(FName(TEXT("Main"), I + 1), FText::FromString(MainNames[I]));
  MainTabs->OnTabSelected.AddUniqueDynamic(this, &ThisClass::HandleMainTab);
  if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(MainTabs)) { S->SetHorizontalAlignment(HAlign_Center); S->SetPadding(FMargin(0, 0, 0, 14)); }
  SubTabs = WidgetTree->ConstructWidget<UDrTabList>();
  SubTabs->Style = EDrTabStyle::Underline;
  SubTabs->SetPaging(EDrTabPaging::Triggers);
  SubTabs->bFocusTabOnPaging = false;
  SubTabs->OnTabSelected.AddUniqueDynamic(this, &ThisClass::HandleSubTab);
  if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(SubTabs)) { S->SetHorizontalAlignment(HAlign_Left); S->SetPadding(FMargin(0, 0, 0, 18)); }

  // 1뎁스 | 2뎁스 | 선택된 정보.
  UHorizontalBox* Body = WidgetTree->ConstructWidget<UHorizontalBox>();
  if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(Body)) { S->SetSize(FSlateChildSize(ESlateSizeRule::Fill)); S->SetPadding(FMargin(0, 0, 0, 18)); }
  auto ListPanel = [&](TObjectPtr<UDrText>& Title, TObjectPtr<UVerticalBox>& List) {
    UVerticalBox* Inner = WidgetTree->ConstructWidget<UVerticalBox>();
    Title = Text(FText::GetEmpty(), TEXT("overline"), DrUITokens::InkMuted);
    if (UVerticalBoxSlot* S = Inner->AddChildToVerticalBox(Title)) S->SetPadding(FMargin(4, 0, 0, 10));
    UDrScrollBox* Scroll = WidgetTree->ConstructWidget<UDrScrollBox>();
    List = WidgetTree->ConstructWidget<UVerticalBox>();
    Scroll->AddChild(List);
    if (UVerticalBoxSlot* S = Inner->AddChildToVerticalBox(Scroll)) S->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    return Panel(Inner);
  };
  USizeBox* Depth1Box = WidgetTree->ConstructWidget<USizeBox>();
  Depth1Box->SetWidthOverride(280.f);
  Depth1Box->AddChild(ListPanel(Depth1Title, Depth1List));
  if (UHorizontalBoxSlot* S = Body->AddChildToHorizontalBox(Depth1Box)) S->SetPadding(FMargin(0, 0, 16, 0));
  if (UHorizontalBoxSlot* S = Body->AddChildToHorizontalBox(ListPanel(Depth2Title, Depth2List))) { S->SetSize(FSlateChildSize(ESlateSizeRule::Fill)); S->SetPadding(FMargin(0, 0, 16, 0)); }
  UVerticalBox* Info = WidgetTree->ConstructWidget<UVerticalBox>();
  Info->AddChildToVerticalBox(Text(LOCTEXT("InfoTitle", "선택된 정보"), TEXT("overline"), DrUITokens::InkMuted));
  Preview = WidgetTree->ConstructWidget<UDrItemSlot>();
  Preview->SlotSize = 96.f;
  Preview->SetIsFocusable(false);
  Preview->SetVisibility(ESlateVisibility::HitTestInvisible);
  if (UVerticalBoxSlot* S = Info->AddChildToVerticalBox(Preview)) { S->SetHorizontalAlignment(HAlign_Left); S->SetPadding(FMargin(0, 12)); }
  InfoName = Text(FText::GetEmpty(), TEXT("title"), DrUITokens::Ink);
  Info->AddChildToVerticalBox(InfoName);
  InfoRarity = Text(FText::GetEmpty(), DrUITokens::Caption, DrUITokens::InkMuted);
  if (UVerticalBoxSlot* S = Info->AddChildToVerticalBox(InfoRarity)) S->SetPadding(FMargin(0, 2, 0, 10));
  InfoBody = Text(FText::GetEmpty(), DrUITokens::Body, DrUITokens::InkMuted);
  InfoBody->SetAutoWrapText(true);
  if (UVerticalBoxSlot* S = Info->AddChildToVerticalBox(InfoBody)) S->SetPadding(FMargin(0, 0, 0, 14));
  CompareTitle = Text(LOCTEXT("CompareTitle", "장착 중인 것과 비교"), TEXT("overline"), DrUITokens::InkMuted);
  Info->AddChildToVerticalBox(CompareTitle);
  Compare = WidgetTree->ConstructWidget<UDrStatCompare>();
  if (UVerticalBoxSlot* S = Info->AddChildToVerticalBox(Compare)) S->SetPadding(FMargin(0, 6, 0, 0));
  USizeBox* InfoBox = WidgetTree->ConstructWidget<USizeBox>();
  InfoBox->SetWidthOverride(360.f);
  InfoBox->AddChild(Panel(Info));
  Body->AddChildToHorizontalBox(InfoBox);

  // 상태 표시줄: 왼쪽 상태 글, 오른쪽 행동 안내(포커스 행의 것 + 공통).
  StatusBar = WidgetTree->ConstructWidget<UDrActionBar>();
  StatusBar->SetBaseHints({
    {TEXT("Tab"), EKeys::Gamepad_LeftShoulder, EKeys::Q, LOCTEXT("HintMain", "메뉴")},
    {TEXT("Sub"), EKeys::Gamepad_LeftTrigger, EKeys::Z, LOCTEXT("HintSub", "분류")},
    {TEXT("Back"), EKeys::Gamepad_FaceButton_Right, EKeys::Escape, LOCTEXT("HintBack", "뒤로")}});
  Column->AddChildToVerticalBox(StatusBar);
  FillSubTabs();
}

UWidget* UDrGlassConsoleMenu::GetDepth1Row(int32 Index) const { return Depth1Rows.IsValidIndex(Index) ? Depth1Rows[Index].Get() : nullptr; }
UWidget* UDrGlassConsoleMenu::GetDepth2Item(int32 Index) const { return Depth2Items.IsValidIndex(Index) ? Depth2Items[Index].Get() : nullptr; }
UWidget* UDrGlassConsoleMenu::GetInfoText() const { return InfoBody; }

void UDrGlassConsoleMenu::Open() {
  if (bOpen) return;
  bOpen = true;
  SetVisibility(ESlateVisibility::Visible);
  FocusMemory.Save();
  DrUIInput::PushModal(this);
  FocusDepth1();
  DrUIFeedback::Play(this, EDrUIFeedback::Confirm);
}

void UDrGlassConsoleMenu::Close() {
  if (!bOpen) return;
  bOpen = false;
  SetVisibility(ESlateVisibility::Collapsed);
  DrUIInput::PopModal(this);
  FocusMemory.Restore();
  DrUIFeedback::Play(this, EDrUIFeedback::Back);
  OnClosed.Broadcast();
}

void UDrGlassConsoleMenu::HandleMainTab(FName TabId) {
  MainIndex = FMath::Clamp(TabId.GetNumber() - 1, 0, int32(UE_ARRAY_COUNT(MainNames)) - 1);
  FillSubTabs();
  // 패드·키보드로 넘겼으면 1뎁스 첫 줄로(마우스로 탭을 눌렀으면 포커스를 옮기지 않는다).
  if (bOpen && DrUIInput::GetLastInputKind() != EDrUIInputKind::Pointer) FocusDepth1();
}

void UDrGlassConsoleMenu::HandleSubTab(FName TabId) {
  SubIndex = FMath::Clamp(TabId.GetNumber() - 1, 0, 2);
  Depth1Index = 0;
  FillDepth1();
  if (bOpen && DrUIInput::GetLastInputKind() != EDrUIInputKind::Pointer) FocusDepth1();
}

void UDrGlassConsoleMenu::FillSubTabs() {
  if (!SubTabs) return;
  SubTabs->RemoveAllTabs();
  for (int32 I = 0; I < 3; ++I) SubTabs->AddTab(FName(TEXT("Sub"), I + 1), FText::FromString(SubNames[MainIndex][I]));
  SubIndex = 0;
  SubTabs->SelectTabByID(FName(TEXT("Sub"), 1));
  Depth1Index = 0;
  FillDepth1();
}

void UDrGlassConsoleMenu::FillDepth1() {
  if (!Depth1List) return;
  Depth1List->ClearChildren();
  Depth1Rows.Reset();
  bInDepth2 = false;
  Depth1Title->SetText(FText::FromString(SubNames[MainIndex][SubIndex]));
  const TArray<FString> Names = Depth1Names(MainIndex, SubIndex);
  TArray<UWidget*> Chain;
  for (int32 I = 0; I < Names.Num(); ++I) {
    UDrListRow* Row = WidgetTree->ConstructWidget<UDrListRow>();
    Row->SetContent(FText::FromString(Names[I]), FText::GetEmpty(), IsSettings() ? FText::GetEmpty() : FText::AsNumber(UE_ARRAY_COUNT(Prefixes)));
    Row->SetIsSelectable(true);
    Row->ActionHints = {{OpenHint, EKeys::Gamepad_FaceButton_Bottom, EKeys::Enter, LOCTEXT("HintOpen", "열기")}};
    Row->OnClicked().AddWeakLambda(this, [this, I] {
      // 마우스: 2뎁스만 바꾸고 포커스는 그대로(뎁스 없이 2뎁스를 바로 누르면 된다). 패드·키보드: 2뎁스로 들어간다.
      if (I != Depth1Index) { Depth1Index = I; MarkDepth1(I); FillDepth2(); }
      if (DrUIInput::GetLastInputKind() != EDrUIInputKind::Pointer) EnterDepth2();
    });
    // 오른쪽은 2뎁스(그 1뎁스에서 마지막으로 있던 줄), 왼쪽은 멈춤. 위아래는 목록 안에서 돈다.
    Row->NavTargets.Set(EUINavigation::Right, [this]() -> UWidget* {
      const int32 Index = FMath::Clamp(Depth2Focus.FindRef(Key()), 0, FMath::Max(Depth2Items.Num() - 1, 0));
      if (Depth2Items.IsValidIndex(Index)) bInDepth2 = true;
      return Depth2Items.IsValidIndex(Index) ? Depth2Items[Index].Get() : Depth1Rows.IsValidIndex(Depth1Index) ? Depth1Rows[Depth1Index].Get() : nullptr;
    });
    Row->NavTargets.Set(EUINavigation::Left, [Row]() -> UWidget* { return Row; });
    if (UVerticalBoxSlot* S = Depth1List->AddChildToVerticalBox(Row)) S->SetPadding(FMargin(0, 2));
    Depth1Rows.Add(Row);
    Chain.Add(Row);
  }
  DrUIInput::ChainVertical(Chain, true);
  MarkDepth1(Depth1Index);
  FillDepth2();
  UpdateStatus();
}

void UDrGlassConsoleMenu::MarkDepth1(int32 Index) {
  // 1뎁스 고른 표시(2뎁스에 들어가도 어느 분류인지 보이게). CommonUI는 토글 버튼만 코드로 선택을 풀 수 있어 잠깐 토글로 바꿔 푼다
  // (늘 토글이면 고른 줄을 다시 눌렀을 때 선택이 풀린다).
  for (int32 J = 0; J < Depth1Rows.Num(); ++J) {
    UDrListRow* Row = Depth1Rows[J];
    if (J == Index) { Row->SetIsSelected(true, false); continue; }
    Row->SetIsToggleable(true);
    Row->SetIsSelected(false, false);
    Row->SetIsToggleable(false);
  }
}

void UDrGlassConsoleMenu::FillDepth2() {
  if (!Depth2List) return;
  Depth2List->ClearChildren();
  Depth2Rows.Reset();
  Depth2Options.Reset();
  Depth2Items.Reset();
  InfoIndex = INDEX_NONE;
  const TArray<FString> Names = Depth1Names(MainIndex, SubIndex);
  Depth2Title->SetText(Names.IsValidIndex(Depth1Index) ? FText::FromString(Names[Depth1Index]) : FText::GetEmpty());
  if (IsSettings()) { FillOptions(); return; }
  TArray<FEntry>& List = Entries();
  TArray<UWidget*> Chain;
  for (int32 I = 0; I < List.Num(); ++I) {
    const FDrItemData& Item = List[I].Item;
    UDrListRow* Row = WidgetTree->ConstructWidget<UDrListRow>();
    Row->bShowChevron = false;
    const FText Trailing = Item.bEquipped ? LOCTEXT("Equipped", "장착 중") : Item.bNew ? LOCTEXT("New", "NEW") : Item.Count > 1 ? FText::Format(LOCTEXT("Count", "×{0}"), Item.Count) : FText::GetEmpty();
    Row->SetContent(Item.Name, FText::FromString(RarityNames[int32(Item.Rarity)]), Trailing);
    Row->ActionHints = {{EquipHint, EKeys::Gamepad_FaceButton_Bottom, EKeys::Enter, LOCTEXT("HintEquip", "장착")}, {DropHint, EKeys::Gamepad_FaceButton_Left, EKeys::R, LOCTEXT("HintDrop", "버리기")}};
    Row->OnClicked().AddWeakLambda(this, [this, I] { Equip(I); });
    Row->OnHintAction.AddUniqueDynamic(this, &ThisClass::HandleRowHint);
    // 왼쪽은 1뎁스(고른 분류), 오른쪽은 멈춤(정보 칸에는 조작이 없다).
    Row->NavTargets.Set(EUINavigation::Left, [this]() -> UWidget* { bInDepth2 = false; return Depth1Rows.IsValidIndex(Depth1Index) ? Depth1Rows[Depth1Index].Get() : nullptr; });
    Row->NavTargets.Set(EUINavigation::Right, [Row]() -> UWidget* { return Row; });
    if (UVerticalBoxSlot* S = Depth2List->AddChildToVerticalBox(Row)) S->SetPadding(FMargin(0, 2));
    Depth2Rows.Add(Row);
    Depth2Items.Add(Row);
    Chain.Add(Row);
  }
  DrUIInput::ChainVertical(Chain, true);
  ShowInfo(Depth2Focus.FindRef(Key()));
}

void UDrGlassConsoleMenu::FillOptions() {
  // '설정' 2뎁스: 좌우로 조작하는 부품이 든 옵션 줄. 좌우는 부품 몫이라 1뎁스로는 B로 돌아간다.
  const TArray<FString> Names = Depth1Names(MainIndex, SubIndex);
  const FString Base = Names.IsValidIndex(Depth1Index) ? Names[Depth1Index] : FString();
  TArray<UWidget*> Chain;
  auto AddRow = [&](const FText& Label, const FText& Description, UWidget* Control) {
    UDrOptionRow* Row = WidgetTree->ConstructWidget<UDrOptionRow>();
    Row->Label = Label;
    Row->Description = Description;
    Row->SetControl(Control);
    const int32 Index = Depth2Options.Num();
    Row->OnFocusedNative.AddWeakLambda(this, [this, Index] { ShowInfo(Index); });
    if (UVerticalBoxSlot* S = Depth2List->AddChildToVerticalBox(Row)) S->SetPadding(FMargin(0, 2));
    Depth2Options.Add(Row);
    Depth2Items.Add(Row);
    Chain.Add(Row);
  };
  // 여러 종류를 한 목록에 둬서 패드·키보드·마우스로 같이 시험한다. 목록이 길어 스크롤도 포커스를 따라간다.
  const FText BaseText = FText::FromString(Base);
  UDrHorizontalSelector* Level = WidgetTree->ConstructWidget<UDrHorizontalSelector>();
  Level->SetOptions({LOCTEXT("OptLow", "낮음"), LOCTEXT("OptMid", "보통"), LOCTEXT("OptHigh", "높음"), LOCTEXT("OptMax", "최고")});
  Level->SetSelectedIndex(1);
  AddRow(FText::Format(LOCTEXT("OptLevel", "{0} 단계"), BaseText), LOCTEXT("OptLevelDesc", "수평 선택기: 좌우로 값을 고른다(끝에서 처음으로 돈다). 마우스는 왼쪽·오른쪽 절반을 누른다."), Level);
  UDrSlider* Amount = WidgetTree->ConstructWidget<UDrSlider>();
  Amount->SetValue(.7f);
  AddRow(FText::Format(LOCTEXT("OptAmount", "{0} 세기"), BaseText), LOCTEXT("OptAmountDesc", "슬라이더: 좌우 한 칸 5%. 패드는 잠그지 않고 바로 조절, 마우스는 끈다."), Amount);
  UDrSegmentedControl* Quality = WidgetTree->ConstructWidget<UDrSegmentedControl>();
  Quality->SetOptions({LOCTEXT("OptQLow", "낮음"), LOCTEXT("OptQMid", "보통"), LOCTEXT("OptQHigh", "높음")});
  Quality->SetSelectedIndex(1);
  AddRow(LOCTEXT("OptQuality", "품질"), LOCTEXT("OptQualityDesc", "세그먼트: 좌우로 칸을 옮기고 A로 고른다. 줄 끝에서는 멈춘다(줄 밖으로 나가지 않음)."), Quality);
  UDrSwitch* Toggle = WidgetTree->ConstructWidget<UDrSwitch>();
  Toggle->SetOn(true);
  AddRow(FText::Format(LOCTEXT("OptOn", "{0} 사용"), BaseText), LOCTEXT("OptOnDesc", "스위치: A로 켜고 끈다."), Toggle);
  UDrCheckBox* Check = WidgetTree->ConstructWidget<UDrCheckBox>();
  Check->Label = LOCTEXT("OptCheckLabel", "배경 어둡게");
  AddRow(LOCTEXT("OptCheck", "자막 배경"), LOCTEXT("OptCheckDesc", "체크박스: A로 켜고 끈다."), Check);
  UHorizontalBox* Radios = WidgetTree->ConstructWidget<UHorizontalBox>();
  UCommonButtonGroupBase* Group = NewObject<UCommonButtonGroupBase>(this);
  Group->SetSelectionRequired(true);
  RadioGroups.Add(Group);
  for (const FText& Label : {LOCTEXT("OptRadioOff", "끔"), LOCTEXT("OptRadioRG", "적록"), LOCTEXT("OptRadioBY", "청황")}) {
    UDrRadio* Radio = WidgetTree->ConstructWidget<UDrRadio>();
    Radio->Label = Label;
    Group->AddWidget(Radio);
    if (UHorizontalBoxSlot* S = Radios->AddChildToHorizontalBox(Radio)) S->SetPadding(FMargin(0, 0, 14, 0));
  }
  Group->SelectButtonAtIndex(0, false);
  AddRow(LOCTEXT("OptRadio", "색맹 보정"), LOCTEXT("OptRadioDesc", "라디오: 좌우로 옮기고 A로 고른다(하나만)."), Radios);
  UDrQuantityStepper* Repeat = WidgetTree->ConstructWidget<UDrQuantityStepper>();
  Repeat->MaxValue = 10;
  Repeat->SetValue(3);
  AddRow(LOCTEXT("OptRepeat", "반복 횟수"), LOCTEXT("OptRepeatDesc", "수량: 좌우로 −·+ 사이를 옮기고 A로 바꾼다(길게 누르면 빨라짐)."), Repeat);
  UDrDropdown* Language = WidgetTree->ConstructWidget<UDrDropdown>();
  Language->SetOptions({LOCTEXT("OptKo", "한국어"), LOCTEXT("OptEn", "English"), LOCTEXT("OptJa", "日本語"), LOCTEXT("OptZh", "中文")});
  Language->SetSelectedIndex(0);
  AddRow(LOCTEXT("OptLanguage", "언어"), LOCTEXT("OptLanguageDesc", "드롭다운: A로 펼치고 위아래로 골라 A, B로 닫으면 이 줄로 돌아온다(덮개 층·포커스 복원 시험)."), Language);
  UDrTextField* Name = WidgetTree->ConstructWidget<UDrTextField>();
  Name->Hint = LOCTEXT("OptNameHint", "이름 입력");
  Name->MaxLength = 12;
  AddRow(LOCTEXT("OptName", "캐릭터 이름"), LOCTEXT("OptNameDesc", "텍스트 입력: 키보드로 바로 쓰고, 패드는 포커스가 오면 입력(콘솔은 가상 키보드). 글 입력 중에는 Q·E 같은 단축키를 가로채지 않는다."), Name);
  UDrHoldButton* Reset = WidgetTree->ConstructWidget<UDrHoldButton>();
  Reset->Label = LOCTEXT("OptResetLabel", "길게 눌러 초기화");
  Reset->Variant = EDrButtonVariant::Destructive;
  Reset->SetHoldSeconds(1.f);
  AddRow(LOCTEXT("OptReset", "기본값"), LOCTEXT("OptResetDesc", "길게 누르기: A를 누른 채 채워지면 실행, 떼면 되감긴다."), Reset);
  UDrButton* Dialog = WidgetTree->ConstructWidget<UDrButton>();
  Dialog->Label = LOCTEXT("OptDialogLabel", "확인 대화상자 열기");
  Dialog->Variant = EDrButtonVariant::Secondary;
  Dialog->OnClicked().AddWeakLambda(this, [this] {
    if (UDrUIFlowSubsystem* Flow = UDrUIFlowSubsystem::Get(this))
      Flow->ShowConfirm(LOCTEXT("OptDialogTitle", "설정을 저장할까요?"), LOCTEXT("OptDialogBody", "닫으면 연 줄로 포커스가 돌아온다."), LOCTEXT("OptDialogSave", "저장"), FText::GetEmpty(), false);
  });
  AddRow(LOCTEXT("OptDialog", "저장"), LOCTEXT("OptDialogDesc", "버튼 → 확인 대화상자(덮개 안에서 또 덮개): 대화상자 밖으로 포커스가 나가지 않고, 닫으면 이 버튼으로 돌아온다."), Dialog);
  DrUIInput::ChainVertical(Chain, true);
  ShowInfo(Depth2Focus.FindRef(Key()));
}

void UDrGlassConsoleMenu::EnterDepth2() {
  const int32 Index = FMath::Clamp(Depth2Focus.FindRef(Key()), 0, FMath::Max(Depth2Items.Num() - 1, 0));
  if (!Depth2Items.IsValidIndex(Index)) return;
  bInDepth2 = true;
  DrUIInput::FocusWidget(Depth2Items[Index]);
}

void UDrGlassConsoleMenu::FocusDepth1() {
  bInDepth2 = false;
  if (Depth1Rows.IsValidIndex(Depth1Index)) Depth1Rows[Depth1Index]->SetFocus();
}

int32 UDrGlassConsoleMenu::Depth2IndexOf(const UWidget* Widget) const {
  if (!Widget) return INDEX_NONE;
  for (int32 I = 0; I < Depth2Items.Num(); ++I) {
    if (Depth2Items[I] == Widget) return I;
    if (const UDrOptionRow* Row = Cast<UDrOptionRow>(Depth2Items[I]); Row && Row == Widget) return I;
  }
  return INDEX_NONE;
}

void UDrGlassConsoleMenu::HandleRowHint(FName HintId) {
  const int32 Index = Depth2Rows.IndexOfByKey(Cast<UDrListRow>(UDrButtonBase::GetHintFocus()));
  if (Index == INDEX_NONE) return;
  if (HintId == EquipHint) Equip(Index);
  else if (HintId == DropHint) {
    // 장착 중인 것은 버릴 수 없다(오류 반응).
    TArray<FEntry>& List = Entries();
    if (!List.IsValidIndex(Index) || List[Index].Item.bEquipped) { DrUIFeedback::Play(this, EDrUIFeedback::Error); return; }
    List.RemoveAt(Index);
    FillDepth2();
    if (Depth2Rows.Num()) Depth2Rows[FMath::Min(Index, Depth2Rows.Num() - 1)]->SetFocus();
  }
}

void UDrGlassConsoleMenu::Equip(int32 Index) {
  TArray<FEntry>& List = Entries();
  if (!List.IsValidIndex(Index)) return;
  for (int32 I = 0; I < List.Num(); ++I) List[I].Item.bEquipped = I == Index;
  List[Index].Item.bNew = false;
  const bool bPointer = DrUIInput::GetLastInputKind() == EDrUIInputKind::Pointer;
  FillDepth2();
  // 다시 만든 줄로 포커스를 돌려준다(마우스면 포커스 없이 정보만).
  if (!bPointer && Depth2Rows.IsValidIndex(Index)) Depth2Rows[Index]->SetFocus();
  ShowInfo(Index);
  UpdateStatus();
}

void UDrGlassConsoleMenu::ShowInfo(int32 Index) {
  const bool bOption = IsSettings();
  Preview->SetVisibility(bOption ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
  for (UWidget* W : {static_cast<UWidget*>(InfoRarity.Get()), static_cast<UWidget*>(CompareTitle.Get()), static_cast<UWidget*>(Compare.Get())})
    if (W) W->SetVisibility(bOption ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
  if (bOption) {
    if (!Depth2Options.IsValidIndex(Index)) return;
    InfoIndex = Index;
    InfoName->SetTokens(TEXT("title"), DrUITokens::Ink);
    InfoName->SetText(Depth2Options[Index]->Label);
    InfoBody->SetText(Depth2Options[Index]->Description);
    return;
  }
  TArray<FEntry>& List = Entries();
  if (!List.IsValidIndex(Index)) return;
  InfoIndex = Index;
  const FEntry& Entry = List[Index];
  Preview->SetItem(Entry.Item);
  InfoName->SetText(Entry.Item.Name);
  InfoName->SetTokens(TEXT("title"), Entry.Item.Rarity == EDrItemRarity::Common ? DrUITokens::Ink : UDrItemSlot::RarityToken(Entry.Item.Rarity));
  InfoRarity->SetText(FText::FromString(RarityNames[int32(Entry.Item.Rarity)]));
  InfoBody->SetText(Entry.Description);
  const FEntry* Equipped = List.FindByPredicate([](const FEntry& E) { return E.Item.bEquipped; });
  const TArray<float>& Now = Equipped ? Equipped->Stats : Entry.Stats;
  Compare->SetRows({
    {LOCTEXT("Attack", "공격력"), Now[0], Entry.Stats[0], true, 0},
    {LOCTEXT("Defense", "방어력"), Now[1], Entry.Stats[1], true, 0},
    {LOCTEXT("Weight", "무게"), Now[2], Entry.Stats[2], false, 1},
    {LOCTEXT("Crit", "치명타 %"), Now[3], Entry.Stats[3], true, 1}});
}

void UDrGlassConsoleMenu::UpdateStatus() {
  int32 Equipped = 0;
  for (const auto& [Key, List] : Data) for (const FEntry& E : List) Equipped += E.Item.bEquipped ? 1 : 0;
  if (StatusBar) StatusBar->SetStatusText(FText::Format(LOCTEXT("Status", "골드 12,400   ·   장착 {0}개   ·   {1} › {2}"), Equipped, FText::FromString(MainNames[MainIndex]), FText::FromString(SubNames[MainIndex][SubIndex])));
}

void UDrGlassConsoleMenu::NativeTick(const FGeometry& Geometry, float DeltaTime) {
  Super::NativeTick(Geometry, DeltaTime);
  if (!bOpen) return;
  const EDrUIInputKind Kind = DrUIInput::GetLastInputKind();
  // 패드·키보드를 쓰는데 포커스가 메뉴 안에 없으면(마우스로 빈 곳을 누른 뒤 등) 마지막 자리로 되돌린다.
  // 메뉴 위에 덮개(드롭다운 목록·확인 대화상자)가 떠 있으면 포커스는 그쪽이 맞으므로 건드리지 않는다.
  if (Kind != EDrUIInputKind::Pointer && !HasFocusedDescendants() && DrUIInput::GetTopModal() == this) {
    if (bInDepth2 && !Depth2Items.IsEmpty()) EnterDepth2();
    else FocusDepth1();
  }
  // 포커스가 1뎁스 줄로 오면 2뎁스를 바꾸고(패드·키보드만: 마우스는 눌러야 바뀜), 2뎁스 줄로 오면 정보를 바꾸고 자리를 기억한다.
  UDrButtonBase* Focused = UDrButtonBase::GetHintFocus();
  const UDrOptionRow* FocusedRow = UDrOptionRow::GetFocusedRow();
  const int32 Row1 = Depth1Rows.IndexOfByKey(Cast<UDrListRow>(Focused));
  const int32 Row2 = FocusedRow ? Depth2IndexOf(FocusedRow) : Depth2IndexOf(Focused);
  if (Row1 != INDEX_NONE) {
    bInDepth2 = false;
    if (Row1 != Depth1Index && Kind != EDrUIInputKind::Pointer) { Depth1Index = Row1; MarkDepth1(Row1); FillDepth2(); }
  } else if (Row2 != INDEX_NONE) {
    bInDepth2 = true;
    Depth2Focus.Add(Key(), Row2);
    if (Row2 != InfoIndex && Kind != EDrUIInputKind::Pointer) ShowInfo(Row2);
  }
  // 마우스: 2뎁스 줄에 올리기만 해도 정보가 바뀐다(뎁스를 거치지 않고 바로 본다). 본 줄을 기억해 패드로 바꾸면 거기서 이어 간다.
  if (Kind == EDrUIInputKind::Pointer)
    for (int32 I = 0; I < Depth2Items.Num(); ++I)
      if (Depth2Items[I] && Depth2Items[I]->IsHovered() && I != InfoIndex) {
        ShowInfo(I);
        Depth2Focus.Add(Key(), I);
        bInDepth2 = true;
        break;
      }
}

FReply UDrGlassConsoleMenu::NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) {
  if (bOpen && DrUIInput::GetAction(Event) == EDrUIInputAction::Back) {
    // 2뎁스면 1뎁스로, 1뎁스(또는 탭)면 닫기.
    if (bInDepth2 && Depth1Rows.IsValidIndex(Depth1Index)) {
      FocusDepth1();
      DrUIFeedback::Play(this, EDrUIFeedback::Back);
    } else Close();
    return FReply::Handled();
  }
  return Super::NativeOnKeyDown(Geometry, Event);
}

FReply UDrGlassConsoleMenu::NativeOnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& Event) {
  // 마우스는 뎁스가 없다: 오른쪽 클릭은 바로 닫기.
  if (bOpen && Event.GetEffectingButton() == EKeys::RightMouseButton) { Close(); return FReply::Handled(); }
  return Super::NativeOnMouseButtonDown(Geometry, Event);
}

#undef LOCTEXT_NAMESPACE
