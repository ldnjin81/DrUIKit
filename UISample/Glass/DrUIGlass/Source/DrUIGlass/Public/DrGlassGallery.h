#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Containers/Ticker.h"
#include "Widgets/DrUIButton.h"
#include "DrGlassGallery.generated.h"

class UCommonAnimatedSwitcher;
class UCommonButtonGroupBase;
class UDrUISegmented;
class UDrUISwitch;
class UDrUITabList;
class UDrUIText;
class UVerticalBox;

/**
 * DrUIKit 갤러리(테스트 레벨 L_DrUIGallery가 띄운다). 위 조작 바(테마·표면 품질·움직임 줄이기) + 탭 + 페이지.
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
  UFUNCTION() void HandleSegmentDemo(int32 Index);
  UFUNCTION() void HandleThemeChoice(int32 Index);
  UFUNCTION() void HandleQualityChoice(int32 Index);
  UDrUIButton* MakeButton(const FText& Label, EDrUIButtonVariant Variant = EDrUIButtonVariant::Secondary);
  UDrUIText* MakeText(const FText& Text, FName Style, FName Color = TEXT("ink"));
  UWidget* Section(const FText& Title, UWidget* Body);
  bool CaptureStep(float DeltaTime);

  static FName PageId(int32 Index) { return FName(TEXT("Page"), Index + 1); }

  UPROPERTY(Transient) TObjectPtr<UCommonAnimatedSwitcher> Pages;
  UPROPERTY(Transient) TObjectPtr<UDrUITabList> PageTabs;
  UPROPERTY(Transient) TObjectPtr<UDrUISegmented> ThemeChoice;
  UPROPERTY(Transient) TObjectPtr<UDrUISegmented> QualityChoice;
  UPROPERTY(Transient) TObjectPtr<UDrUISwitch> MotionSwitch;
  /** 라디오 묶음(하나만 선택). 위젯이 아니라서 따로 붙잡아 둔다. */
  UPROPERTY(Transient) TArray<TObjectPtr<UCommonButtonGroupBase>> RadioGroups;
  UPROPERTY(Transient) TObjectPtr<UDrUIText> SegmentDemoValue;
  TArray<FText> SegmentDemoOptions;
  FTSTicker::FDelegateHandle CaptureTicker;
  int32 CaptureIndex = 0;
  /** 남은 대기 프레임 수. */
  float CaptureWait = 0.f;
  bool bCaptureShotPending = false;
};
