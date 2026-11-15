#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/DrUIInput.h"
#include "Widgets/DrItemGrid.h"
#include "Widgets/DrItemSlot.h"
#include "Widgets/DrStatCompare.h"
#include "DrGlassConsoleMenu.generated.h"

class UDrActionBar;
class UDrItemSlot;
class UDrListRow;
class UDrStatCompare;
class UDrTabList;
class UDrText;
class UVerticalBox;

/**
 * 콘솔 게임 메뉴 샘플(전형적인 패드 메뉴 배치). 갤러리 위를 꽉 덮는 화면.
 *   메인 탭(LB·RB) / 서브 탭(LT·RT)
 *   1뎁스 목록 | 2뎁스 목록 | 선택된 정보(미리보기·설명·능력치 비교)
 *   상태 표시줄(골드·무게 + 행동 안내 바)
 * 패드·키보드: 1뎁스에 포커스가 가면 2뎁스가 바뀌고, A·오른쪽으로 2뎁스에 들어간다(그 1뎁스에서 마지막으로 있던 줄로).
 *   2뎁스에서 포커스를 옮기면 정보가 바뀌고, A는 장착, 왼쪽·B는 1뎁스로, 1뎁스에서 B면 닫는다. 위아래는 목록 안에서 돈다.
 *   방향 이동은 메뉴 밖으로 나가지 않고, 탭을 넘기면 1뎁스로 간다. 포커스가 사라지면(마우스로 빈 곳을 누른 뒤 등) 패드를 쓰는 순간 마지막 자리로.
 * '설정' 탭의 2뎁스는 옵션 줄(슬라이더·수평 선택기·스위치·수량): 좌우는 그 조작 부품이 쓰고 1뎁스로는 B로 돌아간다.
 * 마우스: 뎁스 없이 어디든 바로 누른다. 1뎁스를 누르면 2뎁스만 바뀌고(포커스를 옮기지 않음), 2뎁스는 올리기만 해도 정보가 바뀌며
 *   누르면 장착. 오른쪽 클릭·Esc는 닫기. 내용은 샘플용으로 만들어 넣는다.
 */
UCLASS()
class DRUIGLASS_API UDrGlassConsoleMenu : public UUserWidget {
  GENERATED_BODY()
public:
  UFUNCTION(BlueprintCallable, Category = "DrUI") void Open();
  UFUNCTION(BlueprintCallable, Category = "DrUI") void Close();
  bool IsOpen() const { return bOpen; }
  /** 둘러보기(합성 입력 검증)용: 줄 위젯. */
  UWidget* GetDepth1Row(int32 Index) const;
  UWidget* GetDepth2Item(int32 Index) const;
  UWidget* GetInfoText() const;
  FSimpleMulticastDelegate OnClosed;
protected:
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
  virtual FReply NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;
  virtual FReply NativeOnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& Event) override;
  /** 메뉴 안에서 처리되지 않고 올라온 방향 이동은 멈춘다(뒤의 갤러리 등 메뉴 밖으로 나가지 않게). */
  virtual FNavigationReply NativeOnNavigation(const FGeometry& Geometry, const FNavigationEvent& Event, const FNavigationReply& DefaultReply) override { return FNavigationReply::Stop(); }
private:
  struct FEntry { FDrItemData Item; FText Description; TArray<float> Stats; };
  void Build();
  UFUNCTION() void HandleMainTab(FName TabId);
  UFUNCTION() void HandleSubTab(FName TabId);
  UFUNCTION() void HandleRowHint(FName HintId);
  void FillSubTabs();
  void FillDepth1();
  void FillDepth2();
  void MarkDepth1(int32 Index);
  void FillOptions();
  /** 2뎁스로(이 1뎁스에서 마지막으로 있던 줄). */
  void EnterDepth2();
  void FocusDepth1();
  /** 2뎁스 줄(항목 줄 또는 옵션 줄) 중 포커스·호버가 든 번호. */
  int32 Depth2IndexOf(const UWidget* Widget) const;
  bool IsSettings() const { return MainIndex == 3; }
  void ShowInfo(int32 Index);
  void Equip(int32 Index);
  void UpdateStatus();
  TArray<FEntry>& Entries();
  FString Key() const;

  UPROPERTY(Transient) TObjectPtr<UDrTabList> MainTabs;
  UPROPERTY(Transient) TObjectPtr<UDrTabList> SubTabs;
  UPROPERTY(Transient) TObjectPtr<UVerticalBox> Depth1List;
  UPROPERTY(Transient) TObjectPtr<UVerticalBox> Depth2List;
  UPROPERTY(Transient) TArray<TObjectPtr<UDrListRow>> Depth1Rows;
  UPROPERTY(Transient) TArray<TObjectPtr<UDrListRow>> Depth2Rows;
  UPROPERTY(Transient) TArray<TObjectPtr<class UDrOptionRow>> Depth2Options;
  /** 2뎁스 줄들(항목 줄이나 옵션 줄, 위아래 이동 순서). */
  UPROPERTY(Transient) TArray<TObjectPtr<UWidget>> Depth2Items;
  UPROPERTY(Transient) TObjectPtr<UDrText> Depth1Title;
  UPROPERTY(Transient) TObjectPtr<UDrText> Depth2Title;
  UPROPERTY(Transient) TObjectPtr<UDrItemSlot> Preview;
  UPROPERTY(Transient) TObjectPtr<UDrText> InfoName;
  UPROPERTY(Transient) TObjectPtr<UDrText> InfoRarity;
  UPROPERTY(Transient) TObjectPtr<UDrText> InfoBody;
  UPROPERTY(Transient) TObjectPtr<UDrStatCompare> Compare;
  UPROPERTY(Transient) TObjectPtr<UDrText> CompareTitle;
  /** 옵션 줄의 라디오 묶음(위젯이 아니라 따로 붙잡아 둔다). */
  UPROPERTY(Transient) TArray<TObjectPtr<class UCommonButtonGroupBase>> RadioGroups;
  UPROPERTY(Transient) TObjectPtr<UDrActionBar> StatusBar;
  int32 MainIndex = 0, SubIndex = 0, Depth1Index = 0, InfoIndex = INDEX_NONE;
  /** (메인·서브·1뎁스) → 2뎁스 항목들. 장착 표시를 기억한다. */
  TMap<FString, TArray<FEntry>> Data;
  /** (메인·서브·1뎁스) → 2뎁스에서 마지막으로 있던 줄. */
  TMap<FString, int32> Depth2Focus;
  bool bInDepth2 = false;
  FDrUIFocusMemory FocusMemory;
  bool bOpen = false;
};
