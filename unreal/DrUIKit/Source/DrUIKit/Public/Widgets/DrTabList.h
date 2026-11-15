#pragma once
#include "CoreMinimal.h"
#include "CommonTabListWidgetBase.h"
#include "Core/DrUIMotion.h"
#include "Widgets/DrButtonBase.h"
#include "DrTabList.generated.h"

class UDrButtonPrompt;
class UDrSurface;
class UDrText;
class UPanelWidget;
class USizeBox;

/** 탭 버튼. 바탕은 칠하지 않는다(선택 표시는 탭 목록의 인디케이터가 칠한다). */
UCLASS()
class DRUIKIT_API UDrTabButton : public UDrButtonBase {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Label;
  /** 선택됐을 때 글자색. 알약 인디케이터 위면 on-chip, 밑줄이면 ink. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName SelectedInk = TEXT("on-chip");
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetLabel(const FText& InLabel);
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Surface;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> LabelText;
  virtual void BuildDefaultTree() override;
  virtual void ApplyVisualState() override;
};

UENUM(BlueprintType)
enum class EDrTabStyle : uint8 {
  /** 유리 바 위에서 진한 알약(chip)이 선택 탭으로 미끄러진다. */
  Pill,
  /** 선택 탭 밑에 accent 밑줄이 미끄러진다. */
  Underline
};

/**
 * 탭 목록(디자인 시스템 '탭 인디케이터'). 탭 등록·선택·패드 좌우 이동·화면 전환 연결은 CommonUI 탭 목록이 맡고,
 * 여기서는 탭 바 모양과 선택 탭을 따라 미끄러지는 인디케이터(스프링)를 더한다.
 * 쓰는 법: SetLinkedSwitcher(UCommonAnimatedSwitcher) → AddTab(아이디, 이름, 내용). 내용은 미리 스위처에 넣어 둔다.
 * 게임패드: bShoulderNavigation이면 포커스가 어디 있든 LB·RB(키보드 Q·E)로 탭을 넘기고 양 끝에 그 키 모양을 붙인다.
 * 하위 탭은 bSubTabNavigation(LT·RT, Z·C). 패드로 넘기면 고른 탭 버튼으로 포커스가 간다(내용이 바뀌어도 포커스를 잃지 않게).
 */
UCLASS()
class DRUIKIT_API UDrTabList : public UCommonTabListWidgetBase {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") EDrTabStyle Style = EDrTabStyle::Pill;
  /** 탭 버튼 너비를 가장 긴 탭에 맞춰 똑같이 할지(세그먼트). 트리를 만들기 전에 정한다. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") bool bEqualWidth = false;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") TSubclassOf<UDrTabButton> TabButtonClass;
  /** LB·RB(Q·E)로 넘긴다(화면의 위 탭). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI|Gamepad") bool bShoulderNavigation = false;
  /** LT·RT(Z·C)로 넘긴다(하위 탭). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI|Gamepad") bool bSubTabNavigation = false;
  /** 넘기기 키 모양을 양 끝에 보인다. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI|Gamepad") bool bShowNavigationKeys = true;
  /** 끝에서 넘기면 처음으로. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI|Gamepad") bool bWrapNavigation = true;
  /** 탭 버튼에서 아래(탭이 내용 위에 있을 때)로 가면 연결된 스위처의 지금 내용 중 첫 조작 부품으로 들어간다.
      Slate 방향 이동은 버튼 바로 아래 띠만 찾아서, 아래에 부품이 없으면 탭에서 내려가지 못한다(전역 입력 처리기가 D패드·방향키 아래를 받는다). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI|Gamepad") bool bDownEntersContent = true;

  UDrTabList(const FObjectInitializer& Initializer);
  /** 탭 하나를 더한다. Content가 있으면 연결된 스위처에서 이 탭을 고를 때 보인다. */
  UFUNCTION(BlueprintCallable, Category = "DrUI") bool AddTab(FName TabId, const FText& TabLabel, UWidget* Content = nullptr);
  /** 인디케이터가 지금 있는 곳(트랙 안 왼쪽 x, 너비). 테스트·캡처 확인용. */
  FVector2D GetIndicatorSpan() const { return FVector2D(IndicatorX.Value, IndicatorWidth.Value); }
  /** 이웃 탭으로(Direction -1·+1). bFocusTab이면 고른 탭 버튼에 포커스. 넘겼으면 true. */
  UFUNCTION(BlueprintCallable, Category = "DrUI|Gamepad") bool SelectAdjacentTab(int32 Direction, bool bFocusTab);
  /** Button이 이 목록의 탭 버튼인지. */
  bool OwnsTabButton(const UWidget* Button) const;
  /** 연결된 스위처의 지금 내용 중 첫 조작 부품에 포커스(bDownEntersContent일 때 전역 입력 처리기가 부른다). 했으면 true. */
  bool FocusContent();
  /** 넘기기 플래그를 바꾼 뒤 양 끝 키 모양을 맞춘다. */
  UFUNCTION(BlueprintCallable, Category = "DrUI|Gamepad") void SetTabNavigation(bool bShoulder, bool bSubTab);

protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Track;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Indicator;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<USizeBox> IndicatorBox;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UPanelWidget> Row;  // 가로 상자, 같은 너비면 균등 격자
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrButtonPrompt> PrevKey;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrButtonPrompt> NextKey;

  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeOnInitialized() override;
  virtual void NativeConstruct() override;
  virtual void NativeDestruct() override;
  virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
  virtual void HandleTabCreation_Implementation(FName TabNameID, UCommonButtonBase* TabButton) override;
  virtual void HandleTabRemoval_Implementation(FName TabNameID, UCommonButtonBase* TabButton) override;
  /** 트랙·인디케이터를 Style대로 칠한다(테마가 바뀌어도 부른다). */
  virtual void ApplyTheme();

private:
  void EnsureTree();
  void UpdateIndicator(float DeltaTime);
  TMap<FName, FText> PendingLabels;
  FDrUISpring IndicatorX, IndicatorWidth;
  bool bIndicatorPlaced = false;
  FDelegateHandle ThemeHandle;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDrUISegmentedChanged, int32, Index);

/** 세그먼트(값 하나 고르기). 탭 목록과 같은 모양이지만 화면을 바꾸지 않고 번호만 알린다. 너비는 똑같이 나눈다. */
UCLASS()
class DRUIKIT_API UDrSegmentedControl : public UDrTabList {
  GENERATED_BODY()
public:
  UDrSegmentedControl(const FObjectInitializer& Initializer);
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetOptions(const TArray<FText>& Options);
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetSelectedIndex(int32 Index);
  UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DrUI") int32 GetSelectedIndex() const;
  UPROPERTY(BlueprintAssignable, Category = "DrUI") FDrUISegmentedChanged OnSelectionChanged;
protected:
  virtual void NativeOnInitialized() override;
  virtual void ApplyTheme() override;
private:
  UFUNCTION() void HandleTabSelected(FName TabId);
  static FName OptionId(int32 Index) { return FName(TEXT("Option"), Index + 1); }
};
