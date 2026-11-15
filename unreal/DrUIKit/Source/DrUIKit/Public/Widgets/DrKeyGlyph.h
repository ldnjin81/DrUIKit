#pragma once
#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Core/DrUIInput.h"
#include "InputCoreTypes.h"
#include "DrKeyGlyph.generated.h"

class SDrKeyGlyph;

/** 키 모양 갈래. 패드 버튼은 실제 패드처럼, 키보드는 키캡, 마우스는 마우스 모양으로 그린다. */
UENUM(BlueprintType)
enum class EDrKeyShape : uint8 {
  /** 얼굴 버튼(A·B·X·Y / ✕·○·□·△ / B·A·Y·X): 원. */
  Face,
  /** 숄더(LB·RB / L1·R1 / L·R): 넓은 알약. */
  Shoulder,
  /** 트리거(LT·RT / L2·R2 / ZL·ZR): 위가 둥근 칸. */
  Trigger,
  /** 스틱 누르기(LS·RS / L3·R3): 테 두른 원. */
  Stick,
  /** 스틱 기울기(왼쪽·오른쪽 스틱 가로·세로): 원 + 방향 화살. */
  StickAxis,
  /** D패드 방향: 십자에서 그 방향만 진하게. */
  DPad,
  /** 보기·메뉴(View·Menu / Create·Options / −·+). */
  Special,
  /** 키보드 키캡(글자만큼 넓어짐, 방향키는 화살표). */
  Keyboard,
  /** 마우스 버튼(누르는 쪽을 칠함). */
  Mouse
};

namespace DrKeyGlyph {
DRUIKIT_API EDrKeyShape ShapeFor(const FKey& Key);
}

/**
 * 키 모양 하나(에셋 없이 Slate로 그림). 패드 표기는 Xbox·PlayStation·Nintendo(자동이면 설정·지금 패드),
 * PlayStation 얼굴 버튼은 ✕○□△ 도형을 선으로 그린다. 색은 토큰(패드 몸통 chip, 글자 on-chip, 키캡 glass-strong·ink),
 * bBrandColors면 얼굴 버튼 글자·도형을 패드 고유색으로. 높이는 글자 크기 배율을 따른다.
 */
UCLASS()
class DRUIKIT_API UDrKeyGlyph : public UWidget {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FKey Key = EKeys::Gamepad_FaceButton_Bottom;
  /** 자동이면 설정·지금 쓰는 패드 종류. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") EDrGamepadGlyphs Glyphs = EDrGamepadGlyphs::Auto;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float Height = 24.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") bool bBrandColors = false;

  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetKey(FKey InKey);
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetGlyphs(EDrGamepadGlyphs InGlyphs);
  /** 지금 그리는 표기(자동을 풀어서). */
  EDrGamepadGlyphs GetResolvedGlyphs() const;
  void ApplyTheme();
protected:
  virtual void SynchronizeProperties() override;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void ReleaseSlateResources(bool bReleaseChildren) override;
private:
  TSharedPtr<SDrKeyGlyph> Slate;
  FDelegateHandle ThemeHandle;
};
