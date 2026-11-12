#include "Theme/DrUITheme.h"
#include "DrUIKit.h"
#include "Engine/Font.h"
#include "Styling/CoreStyle.h"

const FLinearColor UDrUITheme::MissingColor(1.f, 0.f, 1.f, 1.f);

void UDrUITheme::WarnMissing(const TCHAR* Kind, FName Token) const {
  if (!Warned.Contains(Token)) {
    Warned.Add(Token);
    UE_LOG(LogDrUIKit, Warning, TEXT("테마 %s에 %s 토큰 '%s'이(가) 없습니다"), *ThemeId.ToString(), Kind, *Token.ToString());
  }
}

FLinearColor UDrUITheme::GetColor(FName Token) const {
  if (const FLinearColor* Found = Colors.Find(Token)) return *Found;
  WarnMissing(TEXT("색"), Token);
  return MissingColor;
}

FDrUITextStyle UDrUITheme::GetTextStyle(FName Token) const {
  if (const FDrUITextStyle* Found = TextStyles.Find(Token)) return *Found;
  WarnMissing(TEXT("글자"), Token);
  return FDrUITextStyle();
}

float UDrUITheme::GetSpacing(FName Token) const {
  if (const float* Found = Spacing.Find(Token)) return *Found;
  WarnMissing(TEXT("간격"), Token);
  return 0.f;
}

float UDrUITheme::GetRadius(FName Token) const {
  if (const float* Found = Radius.Find(Token)) return *Found;
  WarnMissing(TEXT("모서리"), Token);
  return 0.f;
}

FDrUIShadow UDrUITheme::GetShadow(FName Token) const {
  if (const FDrUIShadow* Found = Shadows.Find(Token)) return *Found;
  WarnMissing(TEXT("그림자"), Token);
  return FDrUIShadow();
}

float UDrUITheme::GetBackdrop(FName Token) const {
  if (const float* Found = Backdrops.Find(Token)) return *Found;
  WarnMissing(TEXT("흐림"), Token);
  return 0.f;
}

FName UDrUITheme::PickTypeface(const UFont& Font, int32 Weight) {
  // CSS 굵기 → 서체 이름. 글꼴에 있는 이름 중 가장 가까운 굵기를 고른다(같은 거리면 더 굵은 쪽).
  static const TPair<int32, FName> Names[] = {{300, TEXT("Light")}, {400, TEXT("Regular")}, {500, TEXT("Medium")}, {600, TEXT("SemiBold")}, {700, TEXT("Bold")}};
  TSet<FName> Available;
  for (const FTypefaceEntry& Entry : Font.GetCompositeFont()->DefaultTypeface.Fonts) Available.Add(Entry.Name);
  FName Best = NAME_None;
  int32 BestDistance = MAX_int32;
  for (const auto& [Value, Name] : Names) {
    if (!Available.Contains(Name)) continue;
    const int32 Distance = FMath::Abs(Value - Weight) * 2 - (Value > Weight ? 1 : 0);
    if (Distance < BestDistance) { BestDistance = Distance; Best = Name; }
  }
  return Best.IsNone() && Font.GetCompositeFont()->DefaultTypeface.Fonts.Num() ? Font.GetCompositeFont()->DefaultTypeface.Fonts[0].Name : Best;
}

FSlateFontInfo UDrUITheme::MakeFont(FName TextToken) const {
  const FDrUITextStyle Style = GetTextStyle(TextToken);
  // 엔진 기본 글꼴(Roboto)의 서체: Light / Regular / Bold.
  const FName EngineTypeface = Style.Weight >= 600 ? TEXT("Bold") : Style.Weight <= 300 ? TEXT("Light") : TEXT("Regular");
  FSlateFontInfo Font = FCoreStyle::GetDefaultFontStyle(EngineTypeface, FMath::RoundToInt(Style.Size));
  if (const UFont* Custom = Cast<UFont>(Style.Font.LoadSynchronous())) {
    Font.FontObject = Custom;
    Font.TypefaceFontName = PickTypeface(*Custom, Style.Weight);
  }
  Font.LetterSpacing = FMath::RoundToInt(Style.LetterSpacing * 1000.f);
  return Font;
}

UDrUITheme* UDrUITheme::MakeDefault(UObject* Outer) {
  UDrUITheme* Theme = NewObject<UDrUITheme>(Outer ? Outer : GetTransientPackage(), TEXT("DrUIDefaultTheme"));
  Theme->ThemeId = TEXT("default");
  auto Gray = [](float V, float A = 1.f) { return FLinearColor(FColor(uint8(V * 255), uint8(V * 255), uint8(V * 255))).CopyWithNewOpacity(A); };
  Theme->Colors = {
    {DrUITokens::Ground, Gray(.93f)}, {DrUITokens::Glass, Gray(.98f)}, {DrUITokens::GlassStrong, Gray(1.f)},
    {DrUITokens::GlassStroke, Gray(.78f)}, {DrUITokens::Hairline, Gray(.85f)},
    {DrUITokens::Ink, Gray(.08f)}, {DrUITokens::InkMuted, Gray(.36f)}, {DrUITokens::InkFaint, Gray(.6f)},
    {DrUITokens::Accent, Gray(.2f)}, {DrUITokens::OnAccent, Gray(1.f)}, {DrUITokens::AccentSoft, Gray(.2f, .12f)},
    {DrUITokens::Chip, Gray(.1f)}, {DrUITokens::OnChip, Gray(1.f)},
    {DrUITokens::Success, FLinearColor(FColor(15, 122, 87))}, {DrUITokens::Warning, FLinearColor(FColor(154, 88, 0))},
    {DrUITokens::Danger, FLinearColor(FColor(194, 54, 43))}, {DrUITokens::Focus, Gray(.1f)}};
  auto Text = [](float Size, int32 Weight) { FDrUITextStyle S; S.Size = Size; S.Weight = Weight; return S; };
  Theme->TextStyles = {{TEXT("display"), Text(24, 700)}, {TEXT("title"), Text(16, 600)}, {TEXT("heading"), Text(12, 600)},
    {DrUITokens::Body, Text(11, 400)}, {DrUITokens::Label, Text(10, 500)}, {DrUITokens::Caption, Text(9, 400)}};
  Theme->Spacing = {{TEXT("space-1"), 4}, {DrUITokens::Space2, 8}, {DrUITokens::Space3, 12}, {DrUITokens::Space4, 16}, {TEXT("space-6"), 24}};
  Theme->Radius = {{DrUITokens::RadiusSm, 6}, {DrUITokens::RadiusMd, 8}, {DrUITokens::RadiusLg, 10}, {DrUITokens::RadiusXl, 12}, {DrUITokens::RadiusPill, 999}};
  Theme->Backdrops = {{DrUITokens::BackdropGlass, 0.f}};
  Theme->Shadows = {{DrUITokens::ShadowGlass, FDrUIShadow()}, {DrUITokens::ShadowFloat, FDrUIShadow()}};
  return Theme;
}
