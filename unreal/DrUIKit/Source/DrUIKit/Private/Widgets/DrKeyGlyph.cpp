#include "Widgets/DrKeyGlyph.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "CommonInputSubsystem.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/DrawElements.h"
#include "Rendering/SlateRenderer.h"
#include "Engine/Texture2D.h"
#include "Theme/DrUIGlyphImages.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/SLeafWidget.h"

EDrKeyShape DrKeyGlyph::ShapeFor(const FKey& Key) {
  if (Key == EKeys::Gamepad_FaceButton_Bottom || Key == EKeys::Gamepad_FaceButton_Right || Key == EKeys::Gamepad_FaceButton_Left || Key == EKeys::Gamepad_FaceButton_Top) return EDrKeyShape::Face;
  if (Key == EKeys::Gamepad_LeftShoulder || Key == EKeys::Gamepad_RightShoulder) return EDrKeyShape::Shoulder;
  if (Key == EKeys::Gamepad_LeftTrigger || Key == EKeys::Gamepad_RightTrigger || Key == EKeys::Gamepad_LeftTriggerAxis || Key == EKeys::Gamepad_RightTriggerAxis) return EDrKeyShape::Trigger;
  if (Key == EKeys::Gamepad_LeftThumbstick || Key == EKeys::Gamepad_RightThumbstick) return EDrKeyShape::Stick;
  if (Key == EKeys::Gamepad_LeftX || Key == EKeys::Gamepad_LeftY || Key == EKeys::Gamepad_RightX || Key == EKeys::Gamepad_RightY || Key == EKeys::Gamepad_Left2D || Key == EKeys::Gamepad_Right2D) return EDrKeyShape::StickAxis;
  if (Key == EKeys::Gamepad_DPad_Up || Key == EKeys::Gamepad_DPad_Down || Key == EKeys::Gamepad_DPad_Left || Key == EKeys::Gamepad_DPad_Right) return EDrKeyShape::DPad;
  if (Key == EKeys::Gamepad_Special_Left || Key == EKeys::Gamepad_Special_Right) return EDrKeyShape::Special;
  if (Key.IsMouseButton()) return EDrKeyShape::Mouse;
  return EDrKeyShape::Keyboard;
}

class SDrKeyGlyph : public SLeafWidget {
public:
  SLATE_BEGIN_ARGS(SDrKeyGlyph) {}
  SLATE_END_ARGS()
  void Construct(const FArguments&) {
    Round = FSlateRoundedBoxBrush(FLinearColor::White, 999.f);
    Round.OutlineSettings.RoundingType = ESlateBrushRoundingType::HalfHeightRadius;
    SetVisibility(EVisibility::HitTestInvisible);
  }

  FKey Key;
  EDrUIGamepadGlyphs Glyphs = EDrUIGamepadGlyphs::Xbox;
  float H = 24.f;
  bool bBrand = false;
  FLinearColor Body, Ink, Dim, CapFill, CapEdge, CapInk;
  FSlateFontInfo Font;
  /** 이미지 세트의 그림(있으면 벡터 대신 그대로 그린다, 색은 바꾸지 않음). */
  TStrongObjectPtr<UTexture2D> Image;
  bool bTintImage = true;
  FSlateBrush ImageBrush;

  void Refresh() {
    Shape = DrKeyGlyph::ShapeFor(Key);
    if (Shape == EDrKeyShape::StickAxis) {
      const bool bRight = Key == EKeys::Gamepad_RightX || Key == EKeys::Gamepad_RightY || Key == EKeys::Gamepad_Right2D;
      Label = FText::FromString(bRight ? TEXT("R") : TEXT("L"));
    } else Label = Shape == EDrKeyShape::Face && Glyphs == EDrUIGamepadGlyphs::PlayStation ? FText::GetEmpty() : DrUIInput::GlyphName(Key, Glyphs);
    Font.Size = FMath::Max(6, FMath::RoundToInt(H * (Shape == EDrKeyShape::Face || Shape == EDrKeyShape::Stick ? .38f : .34f)));
    Small = FSlateRoundedBoxBrush(FLinearColor::White, FMath::Max(2.f, H * .14f));
    Big = FSlateRoundedBoxBrush(FLinearColor::White, H * .42f);
    Invalidate(EInvalidateWidgetReason::Layout);
  }

  float TextWidth() const {
    if (Label.IsEmpty()) return 0.f;
    return FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(Label, Font).X;
  }

  virtual FVector2D ComputeDesiredSize(float) const override {
    if (Image) {
      const float Aspect = Image->GetSizeY() > 0 ? float(Image->GetSizeX()) / Image->GetSizeY() : 1.f;
      return FVector2D(H * Aspect, H);
    }
    const float W = TextWidth();
    switch (Shape) {
      case EDrKeyShape::Shoulder: return FVector2D(FMath::Max(H * 1.5f, W + H * .7f), H);
      case EDrKeyShape::Trigger: return FVector2D(FMath::Max(H * 1.3f, W + H * .6f), H);
      case EDrKeyShape::Keyboard: return FVector2D(IsArrow() ? H : FMath::Max(H, W + H * .55f), H);
      case EDrKeyShape::Special: return FVector2D(Glyphs == EDrUIGamepadGlyphs::PlayStation ? H * 1.4f : H, H);
      case EDrKeyShape::Mouse: return FVector2D(H * .75f, H);
      default: return FVector2D(H);
    }
  }

  virtual int32 OnPaint(const FPaintArgs&, const FGeometry& G, const FSlateRect&, FSlateWindowElementList& Out, int32 Layer, const FWidgetStyle& Style, bool) const override {
    const FLinearColor Tint = Style.GetColorAndOpacityTint();
    const FVector2f S = G.GetLocalSize();
    const float T = FMath::Max(1.f, H / 12.f);  // 선 굵기
    auto Box = [&](const FSlateBrush& Brush, FVector2f Pos, FVector2f Size, const FLinearColor& Color, int32 At) {
      FSlateDrawElement::MakeBox(Out, At, G.ToPaintGeometry(Size, FSlateLayoutTransform(Pos)), &Brush, ESlateDrawEffect::None, Color * Tint);
    };
    auto Line = [&](TArray<FVector2f> Points, const FLinearColor& Color, float Width, int32 At) {
      FSlateDrawElement::MakeLines(Out, At, G.ToPaintGeometry(), MoveTemp(Points), ESlateDrawEffect::None, Color * Tint, true, Width);
    };
    auto Text = [&](const FLinearColor& Color, FVector2f Center, int32 At) {
      if (Label.IsEmpty()) return;
      const FVector2f Size = FVector2f(FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(Label, Font));
      FSlateDrawElement::MakeText(Out, At, G.ToPaintGeometry(Size, FSlateLayoutTransform(Center - Size * .5f)), Label, Font, ESlateDrawEffect::None, Color * Tint);
    };
    auto Circle = [](FVector2f C, float R, int32 Segments = 28) {
      TArray<FVector2f> P;
      for (int32 I = 0; I <= Segments; ++I) { const float A = 2.f * PI * I / Segments; P.Add(C + R * FVector2f(FMath::Cos(A), FMath::Sin(A))); }
      return P;
    };
    if (Image) {
      const bool bCap = Shape == EDrKeyShape::Keyboard || Shape == EDrKeyShape::Mouse;
      const FLinearColor Color = bTintImage ? (bCap ? CapInk : Body) : FLinearColor::White;
      FSlateDrawElement::MakeBox(Out, Layer, G.ToPaintGeometry(), &ImageBrush, ESlateDrawEffect::None, Color * Tint);
      return Layer;
    }
    const FVector2f C = S * .5f;
    const float R = FMath::Min(S.X, S.Y) * .5f;
    switch (Shape) {
      case EDrKeyShape::Face: {
        Box(Round, FVector2f(C.X - R, C.Y - R), FVector2f(2.f * R), Body, Layer);
        const FLinearColor Mark = bBrand ? BrandColor() : Ink;
        if (Glyphs != EDrUIGamepadGlyphs::PlayStation) { Text(Mark, C, Layer + 1); break; }
        // PlayStation 도형: ✕(아래) ○(오른쪽) □(왼쪽) △(위).
        const float M = R * .46f;
        if (Key == EKeys::Gamepad_FaceButton_Bottom) { Line({C - FVector2f(M), C + FVector2f(M)}, Mark, T * 1.2f, Layer + 1); Line({C + FVector2f(M, -M), C + FVector2f(-M, M)}, Mark, T * 1.2f, Layer + 1); }
        else if (Key == EKeys::Gamepad_FaceButton_Right) Line(Circle(C, M), Mark, T * 1.2f, Layer + 1);
        else if (Key == EKeys::Gamepad_FaceButton_Left) Line({C + FVector2f(-M, -M), C + FVector2f(M, -M), C + FVector2f(M, M), C + FVector2f(-M, M), C + FVector2f(-M, -M)}, Mark, T * 1.2f, Layer + 1);
        else Line({C + FVector2f(0.f, -M * 1.05f), C + FVector2f(M * 1.05f, M * .75f), C + FVector2f(-M * 1.05f, M * .75f), C + FVector2f(0.f, -M * 1.05f)}, Mark, T * 1.2f, Layer + 1);
        break;
      }
      case EDrKeyShape::Shoulder:
        Box(Round, FVector2f(0.f, S.Y * .14f), FVector2f(S.X, S.Y * .72f), Body, Layer);
        Text(Ink, C, Layer + 1);
        break;
      case EDrKeyShape::Trigger:
        // 위는 크게 둥글고 아래는 거의 각진 칸: 둥근 칸 + 아래 절반을 작은 반경으로 덮는다(같은 색).
        Box(Big, FVector2f::ZeroVector, S, Body, Layer);
        Box(Small, FVector2f(0.f, S.Y * .5f), FVector2f(S.X, S.Y * .5f), Body, Layer);
        Text(Ink, C + FVector2f(0.f, S.Y * .06f), Layer + 1);
        break;
      case EDrKeyShape::Stick:
        Box(Round, FVector2f(C.X - R, C.Y - R), FVector2f(2.f * R), Body, Layer);
        Line(Circle(C, R * .74f), Ink.CopyWithNewOpacity(Ink.A * .45f), T, Layer + 1);
        Text(Ink, C, Layer + 1);
        break;
      case EDrKeyShape::StickAxis: {
        Box(Round, FVector2f(C.X - R, C.Y - R), FVector2f(2.f * R), Body, Layer);
        Text(Ink, C, Layer + 1);  // L 또는 R(Refresh에서 정함)
        // 기울이는 방향의 작은 화살(가로면 좌우, 세로면 위아래, 2D면 넷 다).
        const bool bX = Key == EKeys::Gamepad_LeftX || Key == EKeys::Gamepad_RightX || Key == EKeys::Gamepad_Left2D || Key == EKeys::Gamepad_Right2D;
        const bool bY = Key == EKeys::Gamepad_LeftY || Key == EKeys::Gamepad_RightY || Key == EKeys::Gamepad_Left2D || Key == EKeys::Gamepad_Right2D;
        const float A = R * .78f, B = R * .2f;
        if (bX) { Line({C + FVector2f(-A + B, -B), C + FVector2f(-A, 0.f), C + FVector2f(-A + B, B)}, Ink, T, Layer + 1); Line({C + FVector2f(A - B, -B), C + FVector2f(A, 0.f), C + FVector2f(A - B, B)}, Ink, T, Layer + 1); }
        if (bY) { Line({C + FVector2f(-B, -A + B), C + FVector2f(0.f, -A), C + FVector2f(B, -A + B)}, Ink, T, Layer + 1); Line({C + FVector2f(-B, A - B), C + FVector2f(0.f, A), C + FVector2f(B, A - B)}, Ink, T, Layer + 1); }
        break;
      }
      case EDrKeyShape::DPad: {
        const float W = S.Y * .36f, Half = (S.Y - W) * .5f;
        Box(Small, FVector2f(C.X - W * .5f, 0.f), FVector2f(W, S.Y), Dim, Layer);
        Box(Small, FVector2f(C.X - S.Y * .5f, C.Y - W * .5f), FVector2f(S.Y, W), Dim, Layer);
        FVector2f Pos, Size(W, Half + W * .5f);
        if (Key == EKeys::Gamepad_DPad_Up) Pos = FVector2f(C.X - W * .5f, 0.f);
        else if (Key == EKeys::Gamepad_DPad_Down) Pos = FVector2f(C.X - W * .5f, C.Y - W * .5f);
        else { Size = FVector2f(Half + W * .5f, W); Pos = Key == EKeys::Gamepad_DPad_Left ? FVector2f(C.X - S.Y * .5f, C.Y - W * .5f) : FVector2f(C.X - W * .5f, C.Y - W * .5f); }
        Box(Small, Pos, Size, Body, Layer + 1);
        break;
      }
      case EDrKeyShape::Special: {
        const bool bMenu = Key == EKeys::Gamepad_Special_Right;
        if (Glyphs == EDrUIGamepadGlyphs::Nintendo) {  // −·+ 동그라미
          Box(Round, FVector2f(C.X - R, C.Y - R), FVector2f(2.f * R), Body, Layer);
          const float M = R * .42f;
          Line({C - FVector2f(M, 0.f), C + FVector2f(M, 0.f)}, Ink, T * 1.2f, Layer + 1);
          if (bMenu) Line({C - FVector2f(0.f, M), C + FVector2f(0.f, M)}, Ink, T * 1.2f, Layer + 1);
          break;
        }
        if (Glyphs == EDrUIGamepadGlyphs::PlayStation) Box(Round, FVector2f(0.f, S.Y * .18f), FVector2f(S.X, S.Y * .64f), Body, Layer);
        else Box(Round, FVector2f(C.X - R, C.Y - R), FVector2f(2.f * R), Body, Layer);
        const float M = R * .4f;
        if (bMenu) {  // 메뉴·옵션: 가로줄 셋
          for (float Y : {-M * .7f, 0.f, M * .7f}) Line({C + FVector2f(-M, Y), C + FVector2f(M, Y)}, Ink, T, Layer + 1);
        } else if (Glyphs == EDrUIGamepadGlyphs::PlayStation) {  // 만들기: 세로줄 셋
          for (float X : {-M * .7f, 0.f, M * .7f}) Line({C + FVector2f(X, -M * .7f), C + FVector2f(X, M * .7f)}, Ink, T, Layer + 1);
        } else {  // 보기: 겹친 네모 둘
          const float Q = M * .9f;
          Line({C + FVector2f(-Q, -Q * .8f), C + FVector2f(Q * .4f, -Q * .8f), C + FVector2f(Q * .4f, Q * .3f), C + FVector2f(-Q, Q * .3f), C + FVector2f(-Q, -Q * .8f)}, Ink, T, Layer + 1);
          Line({C + FVector2f(-Q * .4f, -Q * .2f), C + FVector2f(Q, -Q * .2f), C + FVector2f(Q, Q * .9f), C + FVector2f(-Q * .4f, Q * .9f), C + FVector2f(-Q * .4f, -Q * .2f)}, Ink, T, Layer + 1);
        }
        break;
      }
      case EDrKeyShape::Keyboard: {
        // 키캡: 아래 두께(가장자리 색) + 윗면(채움, 1px 테두리 안).
        Box(Small, FVector2f::ZeroVector, S, CapEdge, Layer);
        Box(Small, FVector2f(1.f), FVector2f(S.X - 2.f, S.Y - 2.f - FMath::Max(1.f, H / 12.f)), CapFill, Layer + 1);
        const FVector2f Face = C - FVector2f(0.f, FMath::Max(1.f, H / 12.f) * .5f);
        if (IsArrow()) {
          const float M = H * .2f;
          const FVector2f Dir = Key == EKeys::Up ? FVector2f(0.f, -1.f) : Key == EKeys::Down ? FVector2f(0.f, 1.f) : Key == EKeys::Left ? FVector2f(-1.f, 0.f) : FVector2f(1.f, 0.f);
          const FVector2f Side(-Dir.Y, Dir.X);
          Line({Face - Dir * M, Face + Dir * M}, CapInk, T, Layer + 2);
          Line({Face + Dir * M - Dir * M * .6f + Side * M * .6f, Face + Dir * M, Face + Dir * M - Dir * M * .6f - Side * M * .6f}, CapInk, T, Layer + 2);
        } else Text(CapInk, Face, Layer + 2);
        break;
      }
      case EDrKeyShape::Mouse: {
        // 마우스 몸통(세로 알약) + 위쪽 가운데 선, 누르는 쪽(왼·오른·가운데)을 칠한다.
        Box(Round, FVector2f::ZeroVector, S, CapEdge, Layer);
        Box(Round, FVector2f(1.f), S - FVector2f(2.f), CapFill, Layer + 1);
        const float Top = S.Y * .42f;
        Line({FVector2f(C.X, 2.f), FVector2f(C.X, Top)}, CapEdge, T * .8f, Layer + 2);
        Line({FVector2f(2.f, Top), FVector2f(S.X - 2.f, Top)}, CapEdge, T * .8f, Layer + 2);
        if (Key == EKeys::MiddleMouseButton || Key == EKeys::MouseScrollUp || Key == EKeys::MouseScrollDown) Box(Round, FVector2f(C.X - S.X * .12f, S.Y * .12f), FVector2f(S.X * .24f, S.Y * .24f), Body, Layer + 3);
        else {
          const bool bLeft = Key == EKeys::LeftMouseButton;
          Box(Small, FVector2f(bLeft ? S.X * .14f : C.X + 1.f, S.Y * .1f), FVector2f(S.X * .36f - 1.f, Top - S.Y * .12f), Body, Layer + 3);
        }
        break;
      }
    }
    return Layer + 3;
  }

private:
  EDrKeyShape Shape = EDrKeyShape::Face;
  FText Label;
  FSlateRoundedBoxBrush Round = FSlateRoundedBoxBrush(FLinearColor::White, 999.f);
  FSlateRoundedBoxBrush Small = FSlateRoundedBoxBrush(FLinearColor::White, 3.f);
  FSlateRoundedBoxBrush Big = FSlateRoundedBoxBrush(FLinearColor::White, 10.f);

  bool IsArrow() const { return Key == EKeys::Up || Key == EKeys::Down || Key == EKeys::Left || Key == EKeys::Right; }

  FLinearColor BrandColor() const {
    // 얼굴 버튼 고유색(Xbox 글자색, PlayStation 도형색). 닌텐도는 단색.
    const bool bPS = Glyphs == EDrUIGamepadGlyphs::PlayStation;
    if (Glyphs == EDrUIGamepadGlyphs::Nintendo) return Ink;
    if (Key == EKeys::Gamepad_FaceButton_Bottom) return FLinearColor(bPS ? FColor(0x7f, 0xa8, 0xe0) : FColor(0x6c, 0xc2, 0x4a));
    if (Key == EKeys::Gamepad_FaceButton_Right) return FLinearColor(bPS ? FColor(0xec, 0x6a, 0x74) : FColor(0xe5, 0x4b, 0x3f));
    if (Key == EKeys::Gamepad_FaceButton_Left) return FLinearColor(bPS ? FColor(0xd9, 0x85, 0xbb) : FColor(0x3f, 0x8f, 0xe0));
    return FLinearColor(bPS ? FColor(0x4f, 0xc4, 0xa8) : FColor(0xf2, 0xc2, 0x30));
  }
};

void UDrKeyGlyph::SetKey(FKey InKey) { Key = InKey; ApplyTheme(); }
void UDrKeyGlyph::SetGlyphs(EDrUIGamepadGlyphs InGlyphs) { Glyphs = InGlyphs; ApplyTheme(); }

EDrUIGamepadGlyphs UDrKeyGlyph::GetResolvedGlyphs() const { return Glyphs == EDrUIGamepadGlyphs::Auto ? DrUIInput::ResolveGlyphs(this) : Glyphs; }

void UDrKeyGlyph::ApplyTheme() {
  if (!Slate) return;
  const UDrUITheme& Theme = UDrUIThemeSubsystem::Current();
  const UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  Slate->Key = Key;
  Slate->Glyphs = GetResolvedGlyphs();
  Slate->H = Height * (Subsystem ? Subsystem->GetTextScale() : 1.f);
  Slate->bBrand = bBrandColors;
  Slate->Body = Theme.GetColor(DrUITokens::Chip);
  Slate->Ink = Theme.GetColor(DrUITokens::OnChip);
  const FLinearColor Faint = Theme.GetColor(DrUITokens::InkFaint);
  Slate->Dim = Faint.CopyWithNewOpacity(Faint.A * .45f);
  Slate->CapFill = Theme.GetColor(DrUITokens::GlassStrong).CopyWithNewOpacity(1.f);
  Slate->CapEdge = Faint;
  Slate->CapInk = Theme.GetColor(DrUITokens::Ink);
  Slate->Font = Theme.MakeFont(DrUITokens::Label);
  // 이미지 세트에 이 키가 있으면 이미지로(표기·키보드에 맞는 세트). 없으면 벡터.
  UTexture2D* Texture = DrUIGlyphImages::Find(Slate->Glyphs, Key, &Slate->bTintImage, bBrandColors);
  Slate->Image.Reset(Texture);
  if (Texture) {
    Slate->ImageBrush = FSlateBrush();
    Slate->ImageBrush.SetResourceObject(Texture);
    Slate->ImageBrush.ImageSize = FVector2D(Texture->GetSizeX(), Texture->GetSizeY());
    Slate->ImageBrush.DrawAs = ESlateBrushDrawType::Image;
  }
  Slate->Refresh();
  Slate->Invalidate(EInvalidateWidgetReason::Paint);
}

void UDrKeyGlyph::SynchronizeProperties() { Super::SynchronizeProperties(); ApplyTheme(); }

TSharedRef<SWidget> UDrKeyGlyph::RebuildWidget() {
  SAssignNew(Slate, SDrKeyGlyph);
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get(); Subsystem && !ThemeHandle.IsValid())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
  ApplyTheme();
  return Slate.ToSharedRef();
}

void UDrKeyGlyph::ReleaseSlateResources(bool bReleaseChildren) {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Slate.Reset();
  Super::ReleaseSlateResources(bReleaseChildren);
}
