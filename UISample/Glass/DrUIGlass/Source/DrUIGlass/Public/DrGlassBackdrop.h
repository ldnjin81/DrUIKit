#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DrGlassBackdrop.generated.h"

class UBorder;
class UImage;
class UScaleBox;

/**
 * 유리 뒤에 비칠 바탕: ground 위에 가장자리가 부드럽게 사라지는 큰 색 덩어리 3개(backdrop-a/b/c). 낮은 투명도의 원을 겹쳐 만든다(텍스처·실시간 흐림 없음).
 * 테마에 ground 이미지가 있으면(style.json images) 그 그림을 화면에 가득 채워(비율 유지, 넘치는 쪽 자름) 색 위에 깐다.
 */
UCLASS()
class DRUIGLASS_API UDrGlassBackdrop : public UUserWidget {
  GENERATED_BODY()
protected:
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeOnInitialized() override;
  virtual void NativeDestruct() override;
private:
  void ApplyTheme();
  UPROPERTY(Transient) TObjectPtr<UBorder> Ground;
  UPROPERTY(Transient) TObjectPtr<UScaleBox> GroundFit;
  UPROPERTY(Transient) TObjectPtr<UImage> GroundImage;
  UPROPERTY(Transient) TArray<TObjectPtr<UBorder>> Blobs;
  FDelegateHandle ThemeHandle;
};
