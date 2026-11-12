#pragma once
#include "CoreMinimal.h"
#include "Components/Image.h"
#include "DrUIIcon.generated.h"

/** 아이콘. 단색 이미지를 색 토큰으로 칠한다(SVG 대신 단색 텍스처·머티리얼). */
UCLASS()
class DRUIKIT_API UDrUIIcon : public UImage {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName ColorToken = TEXT("ink");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float IconSize = 20.f;
  void SetColorToken(FName Token) { ColorToken = Token; ApplyTheme(); }
  void ApplyTheme();
protected:
  virtual void SynchronizeProperties() override;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void ReleaseSlateResources(bool bReleaseChildren) override;
private:
  FDelegateHandle ThemeHandle;
};
