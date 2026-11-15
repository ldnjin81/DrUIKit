#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/DrUIInput.h"
#include "Widgets/DrInventory.h"
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
 * 1뎁스에 포커스가 가면 2뎁스가 바뀌고, A로 2뎁스에 들어간다. 2뎁스에서 포커스를 옮기면 정보가 바뀌고, A는 장착, B는 1뎁스로.
 * 1뎁스에서 B면 닫는다. 내용은 샘플용으로 만들어 넣는다.
 */
UCLASS()
class DRUIGLASS_API UDrGlassConsoleMenu : public UUserWidget {
  GENERATED_BODY()
public:
  UFUNCTION(BlueprintCallable, Category = "DrUI") void Open();
  UFUNCTION(BlueprintCallable, Category = "DrUI") void Close();
  bool IsOpen() const { return bOpen; }
  FSimpleMulticastDelegate OnClosed;
protected:
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
  virtual FReply NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;
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
  UPROPERTY(Transient) TObjectPtr<UDrText> Depth1Title;
  UPROPERTY(Transient) TObjectPtr<UDrText> Depth2Title;
  UPROPERTY(Transient) TObjectPtr<UDrItemSlot> Preview;
  UPROPERTY(Transient) TObjectPtr<UDrText> InfoName;
  UPROPERTY(Transient) TObjectPtr<UDrText> InfoRarity;
  UPROPERTY(Transient) TObjectPtr<UDrText> InfoBody;
  UPROPERTY(Transient) TObjectPtr<UDrStatCompare> Compare;
  UPROPERTY(Transient) TObjectPtr<UDrActionBar> StatusBar;
  int32 MainIndex = 0, SubIndex = 0, Depth1Index = 0, InfoIndex = INDEX_NONE;
  TWeakObjectPtr<class UDrButtonBase> LastFocus;
  /** (메인·서브·1뎁스) → 2뎁스 항목들. 장착 표시를 기억한다. */
  TMap<FString, TArray<FEntry>> Data;
  FDrUIFocusMemory FocusMemory;
  bool bOpen = false;
};
