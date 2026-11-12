#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DrGlassBackdrop.generated.h"

class UBorder;

/** 유리 뒤에 비칠 바탕: ground 위에 색 원 3개(backdrop-a/b/c)를 놓고 화면 전체를 강하게 흐려 번진 색 바탕으로 만든다. */
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
