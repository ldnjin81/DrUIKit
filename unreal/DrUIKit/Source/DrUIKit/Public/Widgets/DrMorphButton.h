#pragma once
#include "CoreMinimal.h"
#include "Widgets/DrButton.h"
#include "DrMorphButton.generated.h"

class UDrGlyph;
class USizeBox;

/**
 * 모핑 버튼(로딩 → 체크). 로딩이 시작되면 버튼이 동그랗게 줄며 스피너만 남고, 성공하면 원 안에 체크를 그린 뒤 다시 펴져 성공 문구를 보인다.
 * 오류면 바로 펴져 오류 모양. 상태·문구·성공 유지 시간은 UDrButton 그대로(SetLoading·ShowSuccess·SetError). 움직임 줄이기면 줄지 않는다.
 */
UCLASS()
class DRUIKIT_API UDrMorphButton : public UDrButton {
  GENERATED_BODY()
public:
  UDrMorphButton(const FObjectInitializer& Initializer);
  /** 성공 체크를 원 안에서 보이는 시간(초). 그 뒤 펴진다. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float CheckSeconds = .6f;
  /** 0 펴짐 ~ 1 원. */
  float GetMorphAmount() const { return Morph.Value; }
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<USizeBox> MorphBox;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrGlyph> Check;
  virtual void BuildDefaultTree() override;
  virtual void ApplyVisualState() override;
  virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
private:
  void ApplyMorph();
  FDrUISpring Morph, CheckDraw;
  float FullWidth = 0.f;
  float SuccessAge = -1.f;
  EDrUIState LastState = EDrUIState::Default;
};
