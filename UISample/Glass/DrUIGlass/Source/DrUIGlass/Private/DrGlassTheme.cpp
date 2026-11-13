#include "DrGlassTheme.h"
#include "Dom/JsonObject.h"
#include "Interfaces/IPluginManager.h"
#include "Fonts/CompositeFont.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Theme/DrUITheme.h"

bool DrGlassTheme::ParseCssColor(const FString& InText, FLinearColor& Out) {
  const FString Text = InText.TrimStartAndEnd().ToLower();
  if (Text.StartsWith(TEXT("#"))) {
    FString Hex = Text.Mid(1);
    if (Hex.Len() == 3 || Hex.Len() == 4) { FString Long; for (TCHAR C : Hex) { Long.AppendChar(C); Long.AppendChar(C); } Hex = Long; }
    if (Hex.Len() == 6) Hex += TEXT("ff");
    if (Hex.Len() != 8) return false;
    Out = FLinearColor(FColor::FromHex(Hex.Left(6)));
    Out.A = FParse::HexNumber(*Hex.Right(2)) / 255.f;
    return true;
  }
  if (Text.StartsWith(TEXT("rgb"))) {
    const int32 Open = Text.Find(TEXT("(")), Close = Text.Find(TEXT(")"));
    if (Open < 0 || Close <= Open) return false;
    TArray<FString> Parts;
    Text.Mid(Open + 1, Close - Open - 1).ParseIntoArray(Parts, TEXT(","));
    if (Parts.Num() < 3) return false;
    const FColor Srgb(uint8(FCString::Atoi(*Parts[0])), uint8(FCString::Atoi(*Parts[1])), uint8(FCString::Atoi(*Parts[2])));
    Out = FLinearColor(Srgb);
    Out.A = Parts.Num() > 3 ? FCString::Atof(*Parts[3]) : 1.f;
    return true;
  }
  return false;
}

float DrGlassTheme::PxToFontSize(float Px) { return Px * .75f; }

static float LeadingNumber(const FString& Text) {
  // "16px", "blur(24px) saturate(160%)" → 첫 숫자.
  FString Digits;
  for (TCHAR C : Text) { if (FChar::IsDigit(C) || C == TEXT('.')) Digits.AppendChar(C); else if (!Digits.IsEmpty()) break; }
  return Digits.IsEmpty() ? 0.f : FCString::Atof(*Digits);
}

bool DrGlassTheme::ParseCssShadow(const FString& Text, FDrUIShadow& Out) {
  // 괄호 밖 쉼표로 겹을 나눈다(rgba(…, …) 안의 쉼표는 건너뜀).
  TArray<FString> Layers;
  FString Current;
  int32 Depth = 0;
  for (TCHAR C : Text) {
    if (C == TEXT('(')) ++Depth;
    else if (C == TEXT(')')) --Depth;
    if (C == TEXT(',') && Depth == 0) { Layers.Add(Current); Current.Reset(); }
    else Current.AppendChar(C);
  }
  Layers.Add(Current);
  for (FString Layer : Layers) {
    Layer = Layer.TrimStartAndEnd().ToLower();
    if (Layer.IsEmpty() || Layer.StartsWith(TEXT("inset"))) continue;
    // 길이들(x y 흐림 퍼짐) 뒤에 색.
    const int32 ColorAt = FMath::Max(Layer.Find(TEXT("rgb")), Layer.Find(TEXT("#")));
    if (ColorAt < 0) continue;
    TArray<FString> Lengths;
    Layer.Left(ColorAt).ParseIntoArrayWS(Lengths);
    TArray<float> Values;
    for (const FString& L : Lengths) Values.Add(FCString::Atof(*L));  // "16px" → 16, "-16px" → -16
    if (Values.Num() < 2 || !ParseCssColor(Layer.Mid(ColorAt), Out.Color)) continue;
    Out.Offset = FVector2D(Values[0], Values[1]);
    Out.Blur = Values.IsValidIndex(2) ? Values[2] : 0.f;
    Out.Spread = Values.IsValidIndex(3) ? Values[3] : 0.f;
    return true;
  }
  return false;
}

TArray<UDrUITheme*> DrGlassTheme::BuildThemes(const FString& Json, UObject* Outer, FString& OutError) {
  TArray<UDrUITheme*> Result;
  TSharedPtr<FJsonObject> Root;
  if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root) { OutError = TEXT("tokens.json을 읽을 수 없음"); return Result; }
  const TSharedPtr<FJsonObject>* Color;
  if (!Root->TryGetObjectField(TEXT("color"), Color)) { OutError = TEXT("color 항목 없음"); return Result; }
  TArray<FString> ThemeIds;
  for (const TSharedPtr<FJsonValue>& T : (*Color)->GetArrayField(TEXT("themes"))) ThemeIds.Add(T->AsObject()->GetStringField(TEXT("id")));
  for (const FString& Id : ThemeIds) {
    UDrUITheme* Theme = NewObject<UDrUITheme>(Outer ? Outer : GetTransientPackage(), *(TEXT("DrGlassTheme_") + Id));
    Theme->ThemeId = *(TEXT("glass-") + Id);
    // 색: 테마별 값, 없으면 첫 테마 값(디자인 시스템 규칙).
    for (const TSharedPtr<FJsonValue>& V : (*Color)->GetArrayField(TEXT("tokens"))) {
      const TSharedPtr<FJsonObject> Token = V->AsObject();
      FString Text;
      const TSharedPtr<FJsonObject>* PerTheme;
      if (Token->TryGetObjectField(TEXT("value"), PerTheme)) {
        if (!(*PerTheme)->TryGetStringField(Id, Text)) (*PerTheme)->TryGetStringField(ThemeIds[0], Text);
      } else Token->TryGetStringField(TEXT("value"), Text);
      FLinearColor Parsed;
      if (ParseCssColor(Text, Parsed)) Theme->Colors.Add(*Token->GetStringField(TEXT("name")), Parsed);
    }
    const TSharedPtr<FJsonObject>* Type;
    if (Root->TryGetObjectField(TEXT("type"), Type)) {
      for (const TSharedPtr<FJsonValue>& G : (*Type)->GetArrayField(TEXT("groups")))
        for (const TSharedPtr<FJsonValue>& S : G->AsObject()->GetArrayField(TEXT("styles"))) {
          const TSharedPtr<FJsonObject> StyleJson = S->AsObject();
          FDrUITextStyle Style;
          Style.Size = PxToFontSize(LeadingNumber(StyleJson->GetStringField(TEXT("fontSize"))));
          Style.Weight = int32(StyleJson->GetNumberField(TEXT("fontWeight")));
          FString Spacing;
          if (StyleJson->TryGetStringField(TEXT("letterSpacing"), Spacing)) Style.LetterSpacing = (Spacing.StartsWith(TEXT("-")) ? -1.f : 1.f) * LeadingNumber(Spacing);
          // 본문 글꼴은 테마의 RuntimeFont(Pretendard). 코드 글꼴(mono)은 엔진 기본.
          Style.bUseThemeFont = G->AsObject()->GetStringField(TEXT("family")) == TEXT("sans");
          Theme->TextStyles.Add(*StyleJson->GetStringField(TEXT("name")), Style);
        }
    }
    auto Lengths = [&Root](const TCHAR* Family, TMap<FName, float>& Into, float Scale) {
      const TSharedPtr<FJsonObject>* F;
      if (Root->TryGetObjectField(Family, F))
        for (const TSharedPtr<FJsonValue>& V : (*F)->GetArrayField(TEXT("tokens")))
          Into.Add(*V->AsObject()->GetStringField(TEXT("name")), LeadingNumber(V->AsObject()->GetStringField(TEXT("value"))) * Scale);
    };
    Lengths(TEXT("spacing"), Theme->Spacing, 1.f);
    Lengths(TEXT("radius"), Theme->Radius, 1.f);
    // CSS blur(r)의 r은 가우스 표준편차, SBackgroundBlur 강도도 표준편차(커널 = 3σ, 엔진 소스 확인)라 1:1.
    // saturate(160%)는 Slate 흐림에 없어 버린다.
    Lengths(TEXT("backdrop"), Theme->Backdrops, 1.f);
    // 그림자: 테마별 CSS box-shadow 문자열.
    const TSharedPtr<FJsonObject>* Shadows;
    if (Root->TryGetObjectField(TEXT("shadow"), Shadows))
      for (const TSharedPtr<FJsonValue>& V : (*Shadows)->GetArrayField(TEXT("tokens"))) {
        const TSharedPtr<FJsonObject> Token = V->AsObject();
        FString Text;
        const TSharedPtr<FJsonObject>* PerTheme;
        if (Token->TryGetObjectField(TEXT("value"), PerTheme)) { if (!(*PerTheme)->TryGetStringField(Id, Text)) (*PerTheme)->TryGetStringField(ThemeIds[0], Text); }
        else Token->TryGetStringField(TEXT("value"), Text);
        FDrUIShadow Shadow;
        if (ParseCssShadow(Text, Shadow)) Theme->Shadows.Add(*Token->GetStringField(TEXT("name")), Shadow);
      }
    // 투명도 조절은 유리 채움만 따른다(디자인 시스템: 유리 표면 = glass, 글이 많은 유리 = glass-strong).
    Theme->TranslucentTokens = {TEXT("glass"), TEXT("glass-strong")};
    Result.Add(Theme);
  }
  return Result;
}

TSharedPtr<const FCompositeFont> DrGlassTheme::MakePretendard() {
  // TTF 파일 경로로 만드는 복합 글꼴(엔진 기본 글꼴과 같은 방식, 에셋 없음). 서체 이름 = 굵기 이름.
  static TSharedPtr<const FCompositeFont> Shared;
  if (Shared) return Shared;
  const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("DrUIGlass"));
  if (!Plugin) return nullptr;
  TSharedRef<FStandaloneCompositeFont> Font = MakeShared<FStandaloneCompositeFont>();
  for (const TCHAR* Weight : {TEXT("Light"), TEXT("Regular"), TEXT("Medium"), TEXT("SemiBold"), TEXT("Bold")}) {
    const FString Path = Plugin->GetBaseDir() / TEXT("Fonts") / FString::Printf(TEXT("Pretendard-%s.ttf"), Weight);
    if (FPaths::FileExists(Path)) Font->DefaultTypeface.AppendFont(Weight, Path, EFontHinting::Default, EFontLoadingPolicy::LazyLoad);
  }
  if (Font->DefaultTypeface.Fonts.IsEmpty()) return nullptr;
  Shared = Font;
  return Shared;
}

TArray<UDrUITheme*> DrGlassTheme::LoadThemes(UObject* Outer, FString& OutError) {
  const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("DrUIGlass"));
  FString Json;
  if (!Plugin || !FFileHelper::LoadFileToString(Json, *(Plugin->GetBaseDir() / TEXT("Tokens/tokens.json")))) { OutError = TEXT("Tokens/tokens.json 없음"); return {}; }
  TArray<UDrUITheme*> Themes = BuildThemes(Json, Outer, OutError);
  const TSharedPtr<const FCompositeFont> Font = MakePretendard();
  if (!Font) OutError += TEXT(" Fonts/Pretendard-*.ttf 없음(엔진 기본 글꼴로 대체)");
  for (UDrUITheme* Theme : Themes) Theme->RuntimeFont = Font;
  return Themes;
}
