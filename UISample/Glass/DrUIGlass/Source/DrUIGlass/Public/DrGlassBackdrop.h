#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DrGlassBackdrop.generated.h"

class UBorder;

/** 유리 뒤에 비칠 바탕: ground 위에 크게 흐린 색 원 3개(backdrop-a/b/c). 원은 또렷하고, 위의 유리 표면이 흐리게 보이게 한다. */
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
