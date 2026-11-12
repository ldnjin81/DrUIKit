#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DrGlassBackdrop.generated.h"

class UBorder;
class UImage;

/** 유리 뒤에 비칠 바탕: ground 위에 가장자리가 부드럽게 사라지는 큰 색 덩어리 3개(backdrop-a/b/c). 실시간 흐림을 쓰지 않아 가볍다. */
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
  UPROPERTY(Transient) TArray<TObjectPtr<UImage>> Blobs;
  FDelegateHandle ThemeHandle;
};
