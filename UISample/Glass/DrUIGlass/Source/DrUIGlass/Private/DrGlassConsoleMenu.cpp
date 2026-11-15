#include "DrGlassConsoleMenu.h"
#include "Blueprint/WidgetTree.h"
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
#include "Widgets/DrGamepadUX.h"
#include "Widgets/DrListRow.h"
#include "Widgets/DrScrollBox.h"
#include "Widgets/DrSurface.h"
#include "Widgets/DrTabList.h"
#include "Theme/DrUITheme.h"
#include "Widgets/DrText.h"

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
  MainTabs->SetTabNavigation(true, false);
  for (int32 I = 0; I < UE_ARRAY_COUNT(MainNames); ++I) MainTabs->AddTab(FName(TEXT("Main"), I + 1), FText::FromString(MainNames[I]));
  MainTabs->OnTabSelected.AddUniqueDynamic(this, &ThisClass::HandleMainTab);
  if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(MainTabs)) { S->SetHorizontalAlignment(HAlign_Center); S->SetPadding(FMargin(0, 0, 0, 14)); }
  SubTabs = WidgetTree->ConstructWidget<UDrTabList>();
  SubTabs->Style = EDrTabStyle::Underline;
  SubTabs->SetTabNavigation(false, true);
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
  Info->AddChildToVerticalBox(Text(LOCTEXT("CompareTitle", "장착 중인 것과 비교"), TEXT("overline"), DrUITokens::InkMuted));
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

void UDrGlassConsoleMenu::Open() {
  if (bOpen) return;
  bOpen = true;
  SetVisibility(ESlateVisibility::Visible);
  FocusMemory.Save();
  DrUIInput::PushModal(this);
  if (Depth1Rows.IsValidIndex(Depth1Index)) Depth1Rows[Depth1Index]->SetFocus();
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
}

void UDrGlassConsoleMenu::HandleSubTab(FName TabId) {
  SubIndex = FMath::Clamp(TabId.GetNumber() - 1, 0, 2);
  Depth1Index = 0;
  FillDepth1();
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
  Depth1Title->SetText(FText::FromString(SubNames[MainIndex][SubIndex]));
  const TArray<FString> Names = Depth1Names(MainIndex, SubIndex);
  for (int32 I = 0; I < Names.Num(); ++I) {
    UDrListRow* Row = WidgetTree->ConstructWidget<UDrListRow>();
    Row->SetContent(FText::FromString(Names[I]), FText::GetEmpty(), FText::AsNumber(UE_ARRAY_COUNT(Prefixes)));
    Row->SetIsSelectable(true);
    Row->ActionHints = {{OpenHint, EKeys::Gamepad_FaceButton_Bottom, EKeys::Enter, LOCTEXT("HintOpen", "열기")}};
    // A: 2뎁스로 들어간다(1뎁스는 고른 표시로 남는다).
    Row->OnClicked().AddWeakLambda(this, [this, I] {
      Depth1Index = I;
      MarkDepth1(I);
      if (!Depth2Rows.IsEmpty()) Depth2Rows[0]->SetFocus();
    });
    if (UVerticalBoxSlot* S = Depth1List->AddChildToVerticalBox(Row)) S->SetPadding(FMargin(0, 2));
    Depth1Rows.Add(Row);
  }
  MarkDepth1(Depth1Index);
  FillDepth2();
  // 탭을 넘기는 동안 포커스는 탭 버튼에 있다. 메뉴가 열려 있고 포커스가 사라진 목록에 있었다면 1뎁스 첫 줄로.
  if (bOpen && !LastFocus.IsValid() && Depth1Rows.IsValidIndex(Depth1Index)) Depth1Rows[Depth1Index]->SetFocus();
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
  const TArray<FString> Names = Depth1Names(MainIndex, SubIndex);
  Depth2Title->SetText(Names.IsValidIndex(Depth1Index) ? FText::FromString(Names[Depth1Index]) : FText::GetEmpty());
  TArray<FEntry>& List = Entries();
  for (int32 I = 0; I < List.Num(); ++I) {
    const FDrItemData& Item = List[I].Item;
    UDrListRow* Row = WidgetTree->ConstructWidget<UDrListRow>();
    Row->bShowChevron = false;
    const FText Trailing = Item.bEquipped ? LOCTEXT("Equipped", "장착 중") : Item.bNew ? LOCTEXT("New", "NEW") : Item.Count > 1 ? FText::Format(LOCTEXT("Count", "×{0}"), Item.Count) : FText::GetEmpty();
    Row->SetContent(Item.Name, FText::FromString(RarityNames[int32(Item.Rarity)]), Trailing);
    Row->ActionHints = {{EquipHint, EKeys::Gamepad_FaceButton_Bottom, EKeys::Enter, LOCTEXT("HintEquip", "장착")}, {DropHint, EKeys::Gamepad_FaceButton_Left, EKeys::R, LOCTEXT("HintDrop", "버리기")}};
    Row->OnClicked().AddWeakLambda(this, [this, I] { Equip(I); });
    Row->OnHintAction.AddUniqueDynamic(this, &ThisClass::HandleRowHint);
    if (UVerticalBoxSlot* S = Depth2List->AddChildToVerticalBox(Row)) S->SetPadding(FMargin(0, 2));
    Depth2Rows.Add(Row);
  }
  ShowInfo(0);
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
  FillDepth2();
  if (Depth2Rows.IsValidIndex(Index)) Depth2Rows[Index]->SetFocus();
  ShowInfo(Index);
  UpdateStatus();
}

void UDrGlassConsoleMenu::ShowInfo(int32 Index) {
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
  // 포커스가 1뎁스 줄로 오면 2뎁스를 바꾸고, 2뎁스 줄로 오면 정보를 바꾼다.
  UDrButtonBase* Focused = UDrButtonBase::GetHintFocus();
  if (Focused == LastFocus.Get()) return;
  LastFocus = Focused;
  if (const int32 Row = Depth1Rows.IndexOfByKey(Cast<UDrListRow>(Focused)); Row != INDEX_NONE && Row != Depth1Index) {
    Depth1Index = Row;
    MarkDepth1(Row);
    FillDepth2();
  } else if (const int32 Item = Depth2Rows.IndexOfByKey(Cast<UDrListRow>(Focused)); Item != INDEX_NONE) ShowInfo(Item);
}

FReply UDrGlassConsoleMenu::NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) {
  if (bOpen && DrUIInput::GetAction(Event) == EDrUIInputAction::Back) {
    // 2뎁스면 1뎁스로, 1뎁스(또는 탭)면 닫기.
    if (Depth2Rows.Contains(Cast<UDrListRow>(UDrButtonBase::GetHintFocus())) && Depth1Rows.IsValidIndex(Depth1Index)) {
      Depth1Rows[Depth1Index]->SetFocus();
      DrUIFeedback::Play(this, EDrUIFeedback::Back);
    } else Close();
    return FReply::Handled();
  }
  return Super::NativeOnKeyDown(Geometry, Event);
}

#undef LOCTEXT_NAMESPACE
