#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/DrUIMotion.h"
#include "DrTooltip.generated.h"

class UDrSurface;
class UDrText;
class USizeBox;

/**
 * 툴팁(작은 유리 말풍선: 제목 한 줄 + 설명). 엔진 툴팁 창(UWidget::SetToolTip)에 그대로 넣어 쓰므로 위치·뜨기 지연·마우스 따라가기는 엔진이 맡고,
 * 여기서는 모양과 나타날 때 살짝 내려오며 밝아지는 움직임을 더한다. Attach로 아무 위젯에나 붙인다.
 * 화살표는 넣지 않았다: 엔진 툴팁은 커서 옆 어디에 뜰지 창 가장자리에 따라 바뀌어 가리킬 방향이 고정되지 않는다.
 */
UCLASS()
class DRUIKIT_API UDrTooltip : public UUserWidget {
  GENERATED_BODY()
public:
  /** 비우면 설명만. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Title;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Body;
  /** 이보다 길면 줄을 바꾼다(px). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float MaxWidth = 280.f;

  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetContent(const FText& InTitle, const FText& InBody);
  /** Target에 툴팁을 붙이고 만든 툴팁을 돌려준다. Owner는 툴팁을 만들 위젯(보통 Target이 들어 있는 UserWidget). */
  static UDrTooltip* Attach(UUserWidget* Owner, UWidget* Target, const FText& InTitle, const FText& InBody, TSubclassOf<UDrTooltip> TooltipClass = nullptr);

protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<USizeBox> Frame;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Surface;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> TitleText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> BodyText;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeOnInitialized() override;
  virtual void NativeConstruct() override;
  virtual void NativeDestruct() override;
  virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
  virtual void ApplyTheme();
private:
  void ApplyPresence();
  FDrUISpring Presence;
  FDelegateHandle ThemeHandle;
};
