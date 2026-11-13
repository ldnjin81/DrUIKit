#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/DrUIMotion.h"
#include "DrRadialMenu.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDrRadialSelected, int32, Index);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDrRadialCancelled);

/**
 * 파이 메뉴(원형 메뉴: 무기·감정 표현·핑처럼 방향으로 빨리 고르기). 고리를 항목 수만큼 나눠 그리고, 가리킨 칸이 강조된다.
 * 마우스는 가운데에서 본 방향, 패드는 왼쪽·오른쪽 스틱 방향, 키보드는 화살표로 칸을 옮긴다.
 * 확정은 마우스 왼쪽 떼기·패드 A·Enter, 취소는 Esc·패드 B. 강조는 스프링으로 칸 사이를 미끄러진다.
 * 그림은 모두 NativePaint로 직접 그린다(칸·글·가운데 원). 0번 칸이 12시, 시계 방향.
 */
UCLASS()
class DRUIKIT_API UDrRadialMenu : public UUserWidget {
  GENERATED_BODY()
public:
  UDrRadialMenu(const FObjectInitializer& Initializer);
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") TArray<FText> Items;
  /** 가운데에 보이는 글(가리킨 칸이 없을 때). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Hint;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float Radius = 150.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float InnerRadius = 58.f;
  /** 스틱을 이만큼(0~1) 넘게 기울여야 방향으로 본다. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI", meta = (ClampMin = 0, ClampMax = 1)) float StickDeadZone = .4f;

  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetItems(const TArray<FText>& InItems);
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetHighlighted(int32 Index);
  UFUNCTION(BlueprintCallable, Category = "DrUI") int32 GetHighlighted() const { return Highlighted; }
  /** 가리킨 칸을 확정한다(없으면 아무것도 안 함). */
  UFUNCTION(BlueprintCallable, Category = "DrUI") void Confirm();

  /** 방향(화면 좌표: x 오른쪽, y 아래)이 가리키는 칸. 0번이 12시, 시계 방향. 길이가 0이거나 칸이 없으면 INDEX_NONE. */
  static int32 SectorAt(FVector2D Direction, int32 Count);
  /** From에서 To로 가는 가장 짧은 길로 맞춘 목표(칸 번호 단위, 고리를 넘어갈 수 있음). */
  static float NearestWrapped(float From, int32 To, int32 Count);

  UPROPERTY(BlueprintAssignable, Category = "DrUI") FDrRadialSelected OnItemSelected;
  UPROPERTY(BlueprintAssignable, Category = "DrUI") FDrRadialCancelled OnCancelled;

protected:
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
  virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
  virtual FReply NativeOnMouseMove(const FGeometry& Geometry, const FPointerEvent& Event) override;
  virtual FReply NativeOnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& Event) override;
  virtual FReply NativeOnMouseButtonUp(const FGeometry& Geometry, const FPointerEvent& Event) override;
  virtual void NativeOnMouseLeave(const FPointerEvent& Event) override;
  virtual FReply NativeOnAnalogValueChanged(const FGeometry& Geometry, const FAnalogInputEvent& Event) override;
  virtual FReply NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;
  virtual FNavigationReply NativeOnNavigation(const FGeometry& Geometry, const FNavigationEvent& Event, const FNavigationReply& DefaultReply) override;

private:
  void PointAt(FVector2D Direction);
  int32 Highlighted = INDEX_NONE;
  /** 강조 칸 위치(칸 번호 단위, 스프링). */
  FDrUISpring Sweep;
  /** 강조가 보이는 정도(0~1). */
  FDrUISpring Shown;
  FVector2D Stick = FVector2D::ZeroVector;
  bool bTicked = false;
};
