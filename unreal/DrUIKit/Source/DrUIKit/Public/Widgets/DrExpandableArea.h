#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/DrUIMotion.h"
#include "Widgets/DrButtonBase.h"
#include "DrExpandableArea.generated.h"

class UBorder;
class UDrGlyph;
class UDrSurface;
class UDrText;
class USizeBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDrExpansionEvent, bool, bExpanded);

/** 접히는 영역의 머리줄(제목 + 화살표). 누르면 영역이 열리고 닫힌다. 화살표는 열림 정도대로 돈다. */
UCLASS()
class DRUIKIT_API UDrExpanderHeader : public UDrButtonBase {
  GENERATED_BODY()
public:
  UDrExpanderHeader(const FObjectInitializer& Initializer);
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Title;
  void SetTitle(const FText& InTitle) { Title = InTitle; ApplyVisualState(); }
  /** 0 닫힘 ~ 1 열림(화살표 각도). */
  void SetOpenAmount(float Amount);
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Surface;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> TitleText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrGlyph> Chevron;
  virtual void BuildDefaultTree() override;
  virtual void ApplyVisualState() override;
};

/**
 * 접히는 영역(아코디언 한 칸). 엔진 UExpandableArea는 화살표가 이미지 브러시라 조합으로 만든다: 머리줄 버튼 + 높이가 스프링으로 열리는 본문.
 * 본문은 SetBody로 넣는다(열 때마다 내용 높이를 다시 재므로 내용이 바뀌어도 맞는다). 움직임 줄이기면 바로 열리고 닫힌다.
 */
UCLASS()
class DRUIKIT_API UDrExpandableArea : public UUserWidget {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Title;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") bool bExpanded = false;
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetExpanded(bool bInExpanded);
  UFUNCTION(BlueprintCallable, Category = "DrUI") void Toggle() { SetExpanded(!bExpanded); }
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetBody(UWidget* Content);
  UPROPERTY(BlueprintAssignable, Category = "DrUI") FDrExpansionEvent OnExpansionChanged;
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrExpanderHeader> Header;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<USizeBox> BodyBox;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UBorder> Body;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeOnInitialized() override;
  virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
private:
  void ApplyOpen();
  UPROPERTY(Transient) TObjectPtr<UWidget> PendingBody;
  FDrUISpring Open;
};
