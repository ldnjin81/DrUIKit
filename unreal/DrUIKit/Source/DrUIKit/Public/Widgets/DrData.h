#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CommonNumericTextBlock.h"
#include "Components/Widget.h"
#include "DrData.generated.h"

class SDrSparkline;
class SDrDonut;
class UDrCountUp;
class UDrSurface;
class UDrText;

/** 데이터 표시(스파크라인·도넛·통계 카드). 값은 숫자 배열로 받고 색은 토큰. 축·눈금 없는 작은 그림(대시보드 카드용). */

/** 스파크라인(작은 꺾은선 + 아래 옅은 채움 + 마지막 점). 값 범위에 맞춰 위아래를 채운다. */
UCLASS()
class DRUIKIT_API UDrSparkline : public UWidget {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") TArray<float> Values;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName LineToken = TEXT("accent");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FVector2D LineSize = FVector2D(140.f, 36.f);
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetValues(const TArray<float>& InValues);
  /** 값들을 크기 Size 안 점으로(왼쪽→오른쪽, 큰 값이 위, 위아래 Pad 여백). */
  static TArray<FVector2f> ToPoints(const TArray<float>& InValues, FVector2f Size, float Pad = 3.f);
  void ApplyTheme();
protected:
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void SynchronizeProperties() override;
  virtual void ReleaseSlateResources(bool bReleaseChildren) override;
private:
  TSharedPtr<SDrSparkline> Slate;
  FDelegateHandle ThemeHandle;
};

/** 도넛(비율 원그래프). 조각 사이 틈, 조각 색은 토큰 목록을 차례로. 가운데에 큰 글과 작은 설명. */
UCLASS()
class DRUIKIT_API UDrDonut : public UWidget {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") TArray<float> Values;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") TArray<FName> ColorTokens = {TEXT("accent"), TEXT("success"), TEXT("warning"), TEXT("ember"), TEXT("ink-faint")};
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText CenterValue;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText CenterCaption;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float DonutSize = 120.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float Thickness = 14.f;
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetData(const TArray<float>& InValues, const FText& InCenterValue, const FText& InCenterCaption);
  /** 조각마다 (시작, 길이) 바퀴 비율. 음수는 0으로, 합이 0이면 빈 배열. */
  static TArray<FVector2f> Segments(const TArray<float>& InValues);
  void ApplyTheme();
protected:
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void SynchronizeProperties() override;
  virtual void ReleaseSlateResources(bool bReleaseChildren) override;
private:
  TSharedPtr<SDrDonut> Slate;
  FDelegateHandle ThemeHandle;
};

/** 통계 카드(라벨 + 큰 숫자(세어 올라감) + 변화량 ↑12%·↓3% + 스파크라인). 변화량은 색과 화살표를 함께(색만으로 구분하지 않음). */
UCLASS()
class DRUIKIT_API UDrStatCard : public UUserWidget {
  GENERATED_BODY()
public:
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetData(const FText& InLabel, float InValue, float InDeltaPercent, const TArray<float>& InHistory, ECommonNumericType InNumericType = ECommonNumericType::Number);
  /** 변화량 글("↑12%", "↓3.5%", 0이면 "—"). */
  static FText FormatDelta(float DeltaPercent);
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Surface;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> LabelText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrCountUp> ValueText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> DeltaText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSparkline> Spark;
  virtual TSharedRef<SWidget> RebuildWidget() override;
private:
  void EnsureTree();
};
