#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Containers/Ticker.h"
#include "Widgets/DrButton.h"
#include "DrGlassGallery.generated.h"

class UCommonAnimatedSwitcher;
class UCommonButtonGroupBase;
class UDrSegmentedControl;
class UDrSlider;
class UDrSnackbar;
class UDrTextField;
class UDrRadialMenu;
class UDrSwitch;
class UDrTabList;
class UDrToastStack;
class UDrText;
class UVerticalBox;

/**
 * DrUIKit 갤러리(테스트 레벨 L_DrUIGallery가 띄운다). 위 조작 바(테마·표면 품질·투명도·움직임 줄이기) + 탭 + 페이지.
 * 새 UI를 추가하면 같은 작업에서 페이지에 넣고 CoveredClasses()에도 올린다(DrUIGlass.Gallery.CoversAllKitWidgets가 검사).
 * -DrUIGalleryCapture로 실행하면 테마×페이지마다 화면을 찍고 끝낸다(Saved/Screenshots).
 */
UCLASS()
class DRUIGLASS_API UDrGlassGallery : public UUserWidget {
  GENERATED_BODY()
public:
  /** 갤러리가 보여 주는 DrUIKit 위젯 클래스. */
  static TArray<UClass*> CoveredClasses();
  int32 GetPageCount() const;
  void ShowPage(int32 Index);
protected:
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeDestruct() override;
private:
  void Build();
  UWidget* BuildFoundationPage();
  UWidget* BuildButtonPage();
  UWidget* BuildSelectionPage();
  UWidget* BuildStatusPage();
  UWidget* BuildNoticePage();
  UWidget* BuildInputPage();
  UWidget* BuildGamePage();
  UWidget* BuildNavigationPage();
  UWidget* BuildExtendedPage();
  UWidget* BuildNavigation2Page();
  UWidget* BuildExpressionPage();
  UFUNCTION() void HandleSegmentDemo(int32 Index);
  UFUNCTION() void HandleSliderDemo(float Value);
  UFUNCTION() void HandleTransparency(float Value);
  UFUNCTION() void HandleThemeChoice(int32 Index);
  UFUNCTION() void HandleQualityChoice(int32 Index);
  UFUNCTION() void HandleUndo(int32 RequestId);
  UFUNCTION() void HandleNicknameCommitted(const FText& Text);
  UFUNCTION() void HandleRadialSelected(int32 Index);
  UFUNCTION() void HandleRadialCancelled();
  UFUNCTION() void ShowCarouselPage(int32 Page);
  UFUNCTION() void HandlePageTabSelected(FName TabId);
  UFUNCTION() void HandleChipRemoved(class UDrChip* Chip);
  UFUNCTION() void HandleSampleSnackbar(int32 RequestId);
  UDrButton* MakeButton(const FText& Label, EDrButtonVariant Variant = EDrButtonVariant::Secondary);
  UDrText* MakeText(const FText& Text, FName Style, FName Color = TEXT("ink"));
  UWidget* Section(const FText& Title, UWidget* Body);
  bool CaptureStep(float DeltaTime);

  static FName PageId(int32 Index) { return FName(TEXT("Page"), Index + 1); }

  UPROPERTY(Transient) TObjectPtr<class UDrAnimatedSwitcher> Pages;
  /** 페이지 본문(탭을 고르면 순차 등장). */
  UPROPERTY(Transient) TArray<TObjectPtr<class UPanelWidget>> PageBodies;
  UPROPERTY(Transient) TObjectPtr<UDrTabList> PageTabs;
  UPROPERTY(Transient) TObjectPtr<UDrSegmentedControl> ThemeChoice;
  UPROPERTY(Transient) TObjectPtr<UDrSegmentedControl> QualityChoice;
  UPROPERTY(Transient) TObjectPtr<UDrSwitch> MotionSwitch;
  UPROPERTY(Transient) TObjectPtr<UDrSlider> TransparencySlider;
  /** 화면 위 가운데 토스트, 아래 가운데 스낵바(갤러리 전체 위에 뜬다). */
  UPROPERTY(Transient) TObjectPtr<UDrToastStack> Toasts;
  UPROPERTY(Transient) TObjectPtr<UDrSnackbar> Snackbar;
  UPROPERTY(Transient) TObjectPtr<UDrSnackbar> SampleSnackbar;
  UPROPERTY(Transient) TObjectPtr<class UDrBottomSheet> Sheet;
  UPROPERTY(Transient) TObjectPtr<class UDrCoachMark> Coach;
  UPROPERTY(Transient) TObjectPtr<class UDrPopover> PopoverDemo;
  UPROPERTY(Transient) TObjectPtr<class UDrConfirmDialog> CaptureDialog;
  UPROPERTY(Transient) TObjectPtr<UDrTextField> NicknameField;
  UPROPERTY(Transient) TObjectPtr<UDrRadialMenu> RadialDemo;
  UPROPERTY(Transient) TObjectPtr<class UWidgetSwitcher> CarouselPages;
  UPROPERTY(Transient) TObjectPtr<class UDrPageIndicator> CarouselDots;
  /** 라디오 묶음(하나만 선택). 위젯이 아니라서 따로 붙잡아 둔다. */
  UPROPERTY(Transient) TArray<TObjectPtr<UCommonButtonGroupBase>> RadioGroups;
  UPROPERTY(Transient) TObjectPtr<UDrText> SegmentDemoValue;
  TArray<FText> SegmentDemoOptions;
  UPROPERTY(Transient) TMap<TObjectPtr<UDrSlider>, TObjectPtr<UDrText>> SliderDemoValues;
  FTSTicker::FDelegateHandle CaptureTicker;
  int32 CaptureIndex = 0;
  /** 남은 대기 프레임 수. */
  float CaptureWait = 0.f;
  bool bCaptureShotPending = false;
  /** 지금 페이지를 끝까지 내린 두 번째 장을 찍는 중. */
  bool bCaptureScrolled = false;
};
