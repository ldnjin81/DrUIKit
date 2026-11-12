#pragma once
#include "CoreMinimal.h"
#include "CommonTextBlock.h"
#include "DrUIText.generated.h"

/** 텍스트. 글자 스타일 토큰(body, label …)과 색 토큰(ink …)으로 칠한다. */
UCLASS()
class DRUIKIT_API UDrUIText : public UCommonTextBlock {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName TextStyle = TEXT("body");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName ColorToken = TEXT("ink");
  void SetTokens(FName InTextStyle, FName InColorToken);
  void ApplyTheme();
protected:
  virtual void SynchronizeProperties() override;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void ReleaseSlateResources(bool bReleaseChildren) override;
private:
  FDelegateHandle ThemeHandle;
};
