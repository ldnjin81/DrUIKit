#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DrStatCompare.generated.h"

class UVerticalBox;

/** 능력치 한 줄(현재 → 새 값). */
USTRUCT(BlueprintType)
struct DRUIKIT_API FDrStatRow {
  GENERATED_BODY()
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Label;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float Current = 0.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float New = 0.f;
  /** 거짓이면 작을수록 좋음(무게·쿨다운 등). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") bool bHigherIsBetter = true;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI", meta = (ClampMin = 0, ClampMax = 3)) int32 Decimals = 0;
};

/**
 * 능력치 비교(지금 장비 → 고른 장비). 줄마다 이름 · 현재 값 · ▲▼ · 새 값. 좋아지면 success, 나빠지면 danger, 같으면 흐리게.
 * 계산은 게임이 아니라 이 부품이 한다(플레이어가 빼기를 하지 않게).
 */
UCLASS()
class DRUIKIT_API UDrStatCompare : public UUserWidget {
  GENERATED_BODY()
public:
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetRows(const TArray<FDrStatRow>& InRows);
  /** +1 좋아짐, -1 나빠짐, 0 같음. */
  static int32 CompareDelta(const FDrStatRow& Row);
  void ApplyTheme();
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UVerticalBox> List;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeConstruct() override;
  virtual void NativeDestruct() override;
private:
  void Fill();
  TArray<FDrStatRow> Rows;
  FDelegateHandle ThemeHandle;
};
