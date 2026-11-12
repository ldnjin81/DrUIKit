#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CircularThrobber.h"
#include "Components/ProgressBar.h"
#include "Components/Widget.h"
#include "DrUIStatus.generated.h"

class SDrUIProgressRing;
class SDrUISkeleton;
class UDrUISurface;
class UDrUIText;
class USizeBox;

/** 상태 표시 부품(배지·진행 막대·진행 링·스켈레톤·스피너). 모두 토큰으로만 칠한다. */

UENUM(BlueprintType)
enum class EDrUIBadgeTone : uint8 {
  /** 강조색(accent). */
  Accent,
  /** 알림 점(ember, 디자인 시스템 '알림 점'). */
  Alert,
  Danger, Success, Warning,
  /** 진한 알약(chip). */
  Neutral
};

/** 배지(숫자·점). 숫자가 0이면 숨고, 최댓값을 넘으면 "99+"처럼 보인다. bDot이면 숫자 없이 점만. */
UCLASS()
class DRUIKIT_API UDrUIBadge : public UUserWidget {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") int32 Count = 0;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") int32 MaxCount = 99;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") bool bDot = false;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") EDrUIBadgeTone Tone = EDrUIBadgeTone::Alert;

  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetCount(int32 InCount);
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetDot(bool bInDot);
  /** 보일 글자. 0 이하면 빈 글자(숨김), MaxCount를 넘으면 "MaxCount+". */
  static FText FormatCount(int32 InCount, int32 InMaxCount);
  void ApplyTheme();
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<USizeBox> Frame;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrUISurface> Surface;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrUIText> CountText;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeOnInitialized() override;
  virtual void NativeDestruct() override;
private:
  FDelegateHandle ThemeHandle;
};

/** 진행 막대. 엔진 UProgressBar(채움 움직임·끝없는 진행 포함)에 둥근 트랙·채움을 토큰으로 칠한다. */
UCLASS()
class DRUIKIT_API UDrUIProgressBar : public UProgressBar {
  GENERATED_BODY()
public:
  UDrUIProgressBar();
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName TrackToken = TEXT("hairline");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName FillToken = TEXT("accent");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float Thickness = 6.f;
  void ApplyTheme();
protected:
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void SynchronizeProperties() override;
  virtual void ReleaseSlateResources(bool bReleaseChildren) override;
private:
  FDelegateHandle ThemeHandle;
};

namespace DrUIProgressRing {
/** 원호 위 점들. 시작·길이는 한 바퀴 비율(0 = 12시, 시계 방향). Segments는 한 바퀴 기준 조각 수. */
DRUIKIT_API TArray<FVector2f> ArcPoints(FVector2f Center, float Radius, float Start, float Sweep, int32 Segments = 64);
}

/** 진행 링(원형 진행률). 값이 바뀌면 스프링으로 따라가고, bIndeterminate면 짧은 호가 돈다. */
UCLASS()
class DRUIKIT_API UDrUIProgressRing : public UWidget {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI", meta = (ClampMin = 0, ClampMax = 1)) float Percent = 0.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") bool bIndeterminate = false;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float RingSize = 40.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float Thickness = 4.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName TrackToken = TEXT("hairline");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName FillToken = TEXT("accent");
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetPercent(float InPercent);
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetIndeterminate(bool bInIndeterminate);
  void ApplyTheme();
protected:
  virtual void SynchronizeProperties() override;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void ReleaseSlateResources(bool bReleaseChildren) override;
private:
  TSharedPtr<SDrUIProgressRing> Slate;
  FDelegateHandle ThemeHandle;
};

/** 스켈레톤(불러오는 중 자리 표시). 둥근 사각형이 천천히 밝아졌다 어두워진다. 움직임 줄이기면 멈춘다. */
UCLASS()
class DRUIKIT_API UDrUISkeleton : public UWidget {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FVector2D BlockSize = FVector2D(160.f, 12.f);
  /** radius-pill이면 원·알약. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName RadiusToken = TEXT("radius-xs");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName ColorToken = TEXT("ink-faint");
  /** 시간(초)에 따른 불투명도(.18 ~ .32, 1.2초 주기). */
  static float PulseAlpha(double Seconds);
  void ApplyTheme();
protected:
  virtual void SynchronizeProperties() override;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void ReleaseSlateResources(bool bReleaseChildren) override;
private:
  TSharedPtr<SDrUISkeleton> Slate;
  FDelegateHandle ThemeHandle;
};

/** 스피너. 엔진 UCircularThrobber를 색 토큰으로 칠하고 조각을 작게(겹쳐 덩어리로 보이지 않게) 맞춘다. */
UCLASS()
class DRUIKIT_API UDrUISpinner : public UCircularThrobber {
  GENERATED_BODY()
public:
  UDrUISpinner(const FObjectInitializer& Initializer);
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName ColorToken = TEXT("ink");
  /** 조각 지름(px). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float PieceSize = 3.f;
  void SetColorToken(FName Token) { ColorToken = Token; ApplyTheme(); }
  void ApplyTheme();
protected:
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void SynchronizeProperties() override;
  virtual void ReleaseSlateResources(bool bReleaseChildren) override;
private:
  FDelegateHandle ThemeHandle;
};
