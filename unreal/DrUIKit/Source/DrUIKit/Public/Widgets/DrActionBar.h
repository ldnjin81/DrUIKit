#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/DrButtonBase.h"
#include "DrActionBar.generated.h"

class UDrButtonPrompt;
class UDrSurface;
class UDrText;
class UHorizontalBox;

/**
 * 행동 안내 바(화면 아래 "Ⓐ 선택 Ⓑ 뒤로 LB/RB 탭" 줄). 왼쪽은 상태 글(골드·무게 등), 오른쪽은 지금 쓸 수 있는 버튼들.
 * 포커스가 든 DrUIKit 버튼의 ActionHints가 앞에, 화면 공통 BaseHints가 뒤에 온다. 키 모양은 입력 장치·패드 종류를 따라 바뀐다.
 */
UCLASS()
class DRUIKIT_API UDrActionBar : public UUserWidget {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") TArray<FDrActionHint> BaseHints;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") bool bShowFocusHints = true;
  /** 유리 바 위에 그린다(끄면 글과 키만). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") bool bSurface = true;
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetBaseHints(const TArray<FDrActionHint>& Hints);
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetStatusText(const FText& Text);
  /** 지금 보이는 안내(포커스 버튼 것 + 공통). 같은 키는 앞의 것만. */
  TArray<FDrActionHint> CollectHints() const;
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Surface;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> StatusLabel;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UHorizontalBox> Prompts;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
private:
  void Rebuild(const TArray<FDrActionHint>& Hints);
  FString ShownSignature;
  FText Status;
};
