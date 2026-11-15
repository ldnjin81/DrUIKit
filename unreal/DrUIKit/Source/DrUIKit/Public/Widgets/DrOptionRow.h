#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/DrButtonBase.h"
#include "DrOptionRow.generated.h"

class UDrSurface;
class UDrText;
class USizeBox;

/**
 * 옵션 줄(설정 화면의 "밝기 ──●── 70%" 한 줄). 왼쪽 이름, 오른쪽 조작 부품(슬라이더·수평 선택기·스위치·수량 등 SetControl).
 * 좌우는 줄 안의 조작 부품이 쓴다(줄을 벗어나지 않음). 위아래는 NavTargets(목록이면 DrUIInput::ChainVertical)로 이웃 줄.
 * 포커스가 줄 안에 있으면 줄 전체를 강조하고(패드·키보드면 포커스 테두리), 행동 안내 바에 이 줄의 안내(기본: 좌우 조절)를 보인다.
 * 마우스는 조작 부품을 바로 누르고 끈다.
 */
UCLASS()
class DRUIKIT_API UDrOptionRow : public UUserWidget {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Label;
  /** 정보 패널 등에 보일 설명(이 부품은 그리지 않는다). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Description;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float ControlWidth = 280.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") TArray<FDrActionHint> ActionHints;
  FDrUINavTargets NavTargets;
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetLabel(const FText& InLabel);
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetControl(UWidget* Control);
  UWidget* GetControl() const { return PendingControl; }
  /** 포커스가 줄 안에 있는지(조작 부품 또는 그 안). */
  bool IsFocusWithin() const { return bFocusWithin; }
  /** 포커스가 든 옵션 줄(없으면 nullptr). */
  static UDrOptionRow* GetFocusedRow();
  /** 포커스가 줄 안으로 들어올 때. */
  FSimpleMulticastDelegate OnFocusedNative;
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Surface;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> LabelText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<USizeBox> ControlBox;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeConstruct() override;
  virtual void NativeDestruct() override;
  virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
  /** 줄 어디를 눌러도 조작 부품으로 포커스(마우스는 뎁스 없이 바로). */
  virtual FReply NativeOnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& Event) override;
  virtual FNavigationReply NativeOnNavigation(const FGeometry& Geometry, const FNavigationEvent& Event, const FNavigationReply& DefaultReply) override;
private:
  void ApplyTheme();
  UPROPERTY(Transient) TObjectPtr<UWidget> PendingControl;
  bool bFocusWithin = false, bHoveredNow = false, bRingNow = false;
  FDelegateHandle ThemeHandle;
};
