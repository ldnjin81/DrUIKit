#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DrGlassBackdrop.generated.h"

class UBorder;
class UImage;

/** 유리 뒤에 비칠 바탕: ground 위에 가장자리가 부드럽게 사라지는 큰 색 덩어리 3개(backdrop-a/b/c). 낮은 투명도의 원을 겹쳐 만든다(텍스처·실시간 흐림 없음). */
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
  UPROPERTY(Transient) TArray<TObjectPtr<UBorder>> Blobs;
  FDelegateHandle ThemeHandle;
};
