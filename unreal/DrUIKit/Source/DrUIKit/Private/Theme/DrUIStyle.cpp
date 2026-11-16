#include "Theme/DrUIStyle.h"
#include "DrUIKit.h"
#include "Dom/JsonObject.h"
#include "Fonts/CompositeFont.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Theme/DrUIKitSettings.h"
#include "Theme/DrUITheme.h"
#include "Theme/DrUIThemeSubsystem.h"

bool DrUIStyle::ParseCssColor(const FString& InText, FLinearColor& Out) {
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

float DrUIStyle::PxToFontSize(float Px) { return Px * .75f; }

static float LeadingNumber(const FString& Text) {
  // "16px", "blur(24px) saturate(160%)" → 첫 숫자.
  FString Digits;
  for (TCHAR C : Text) { if (FChar::IsDigit(C) || C == TEXT('.')) Digits.AppendChar(C); else if (!Digits.IsEmpty()) break; }
  return Digits.IsEmpty() ? 0.f : FCString::Atof(*Digits);
}

TArray<FDrUIShadow> DrUIStyle::ParseCssShadows(const FString& Text) {
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
  TArray<FDrUIShadow> Result;
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
    FDrUIShadow Shadow;
    if (Values.Num() < 2 || !ParseCssColor(Layer.Mid(ColorAt), Shadow.Color)) continue;
    Shadow.Offset = FVector2D(Values[0], Values[1]);
    Shadow.Blur = Values.IsValidIndex(2) ? Values[2] : 0.f;
    Shadow.Spread = Values.IsValidIndex(3) ? Values[3] : 0.f;
    Result.Add(Shadow);
  }
  return Result;
}

bool DrUIStyle::ParseCssShadow(const FString& Text, FDrUIShadow& Out) {
  const TArray<FDrUIShadow> Layers = ParseCssShadows(Text);
  if (Layers.IsEmpty()) return false;
  Out = Layers[0];
  return true;
}

TArray<UDrUITheme*> DrUIStyle::BuildThemes(const FString& Json, FName StyleId, UObject* Outer, FString& OutError) {
  TArray<UDrUITheme*> Result;
  TSharedPtr<FJsonObject> Root;
  if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root) { OutError = TEXT("tokens.json을 읽을 수 없음"); return Result; }
  const TSharedPtr<FJsonObject>* Color;
  if (!Root->TryGetObjectField(TEXT("color"), Color)) { OutError = TEXT("color 항목 없음"); return Result; }
  TArray<FString> ThemeIds;
  for (const TSharedPtr<FJsonValue>& T : (*Color)->GetArrayField(TEXT("themes"))) ThemeIds.Add(T->AsObject()->GetStringField(TEXT("id")));
  if (ThemeIds.IsEmpty()) { OutError = TEXT("color.themes가 비어 있음"); return Result; }
  // 토큰 값: 테마별 값({light, dark}), 없으면 첫 테마 값(디자인 시스템 규칙). 한 값이면 모든 테마 공통.
  auto ValueFor = [&ThemeIds](const TSharedPtr<FJsonObject>& Token, const FString& Id) {
    FString Text;
    const TSharedPtr<FJsonObject>* PerTheme;
    if (Token->TryGetObjectField(TEXT("value"), PerTheme)) { if (!(*PerTheme)->TryGetStringField(Id, Text)) (*PerTheme)->TryGetStringField(ThemeIds[0], Text); }
    else Token->TryGetStringField(TEXT("value"), Text);
    return Text;
  };
  for (const FString& Id : ThemeIds) {
    const FString ThemeId = StyleId.ToString() + TEXT("-") + Id;
    UObject* ThemeOuter = Outer ? Outer : GetTransientPackage();
    UDrUITheme* Theme = NewObject<UDrUITheme>(ThemeOuter, MakeUniqueObjectName(ThemeOuter, UDrUITheme::StaticClass(), *(TEXT("DrUITheme_") + ThemeId.Replace(TEXT("-"), TEXT("_")))));
    Theme->ThemeId = *ThemeId;
    Theme->StyleId = StyleId;
    for (const TSharedPtr<FJsonValue>& V : (*Color)->GetArrayField(TEXT("tokens"))) {
      FLinearColor Parsed;
      if (ParseCssColor(ValueFor(V->AsObject(), Id), Parsed)) Theme->Colors.Add(*V->AsObject()->GetStringField(TEXT("name")), Parsed);
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
          // 글꼴은 갈래(sans·mono)로 고른다. 스타일이 그 갈래 글꼴을 주지 않으면 sans는 RuntimeFont, 그 밖은 엔진 기본.
          Style.Family = *G->AsObject()->GetStringField(TEXT("family"));
          Theme->TextStyles.Add(*StyleJson->GetStringField(TEXT("name")), Style);
        }
    }
    auto Lengths = [&Root](const TCHAR* Family, TMap<FName, float>& Into) {
      const TSharedPtr<FJsonObject>* F;
      if (Root->TryGetObjectField(Family, F))
        for (const TSharedPtr<FJsonValue>& V : (*F)->GetArrayField(TEXT("tokens")))
          Into.Add(*V->AsObject()->GetStringField(TEXT("name")), LeadingNumber(V->AsObject()->GetStringField(TEXT("value"))));
    };
    Lengths(TEXT("spacing"), Theme->Spacing);
    Lengths(TEXT("radius"), Theme->Radius);
    // CSS blur(r)의 r은 가우스 표준편차, SBackgroundBlur 강도도 표준편차(커널 = 3σ, 엔진 소스 확인)라 1:1.
    // saturate(160%)는 Slate 흐림에 없어 버린다.
    Lengths(TEXT("backdrop"), Theme->Backdrops);
    // 그림자: 테마별 CSS box-shadow 문자열. 여러 겹이면 모두(ShadowLayers), 첫 겹은 Shadows.
    const TSharedPtr<FJsonObject>* Shadows;
    if (Root->TryGetObjectField(TEXT("shadow"), Shadows))
      for (const TSharedPtr<FJsonValue>& V : (*Shadows)->GetArrayField(TEXT("tokens"))) {
        const FName Name = *V->AsObject()->GetStringField(TEXT("name"));
        FDrUIShadowLayers Layers;
        Layers.Layers = ParseCssShadows(ValueFor(V->AsObject(), Id));
        if (Layers.Layers.IsEmpty()) continue;
        Theme->Shadows.Add(Name, Layers.Layers[0]);
        Theme->ShadowLayers.Add(Name, Layers);
      }
    Result.Add(Theme);
  }
  return Result;
}

namespace {
FString ResolveFontPath(const FString& Path, const FString& PluginDir) {
  if (Path.StartsWith(TEXT("{engine}"))) return FPaths::EngineContentDir() / Path.Mid(8);
  if (Path.StartsWith(TEXT("{plugin}"))) return PluginDir / Path.Mid(8);
  if (Path.StartsWith(TEXT("{plugin:"))) {
    const int32 End = Path.Find(TEXT("}"));
    const TSharedPtr<IPlugin> Other = End > 8 ? IPluginManager::Get().FindPlugin(Path.Mid(8, End - 8)) : nullptr;
    return Other ? Other->GetBaseDir() / Path.Mid(End + 1) : FString();
  }
  return Path;
}

/** 서체 이름 → TTF 경로 묶음을 서체에 더한다. 없는 파일은 OutMissing에. */
void AppendTypefaces(FTypeface& Typeface, const TSharedPtr<FJsonObject>& Files, const FString& PluginDir, TArray<FString>& OutMissing) {
  for (const auto& [Name, Value] : Files->Values) {
    const FString Path = ResolveFontPath(Value->AsString(), PluginDir);
    if (FPaths::FileExists(Path)) Typeface.AppendFont(*Name, Path, EFontHinting::Default, EFontLoadingPolicy::LazyLoad);
    else OutMissing.Add(Value->AsString());
  }
}

/** style.json fonts의 한 갈래로 복합 글꼴을 만든다(파일 경로 기반, 에셋 없음). 같은 스타일·갈래는 한 번만 만든다. */
TSharedPtr<const FCompositeFont> MakeFamilyFont(const FString& CacheKey, const TSharedPtr<FJsonObject>& Spec, const FString& PluginDir, TArray<FString>& OutMissing) {
  static TMap<FString, TSharedPtr<const FCompositeFont>> Cache;
  if (const TSharedPtr<const FCompositeFont>* Found = Cache.Find(CacheKey)) return *Found;
  const TSharedPtr<FJsonObject>* Typefaces;
  if (!Spec->TryGetObjectField(TEXT("typefaces"), Typefaces)) return nullptr;
  TSharedRef<FStandaloneCompositeFont> Font = MakeShared<FStandaloneCompositeFont>();
  AppendTypefaces(Font->DefaultTypeface, *Typefaces, PluginDir, OutMissing);
  if (Font->DefaultTypeface.Fonts.IsEmpty()) return nullptr;
  // 라틴 글꼴에 한글이 없으면 한국어 글자(한글 자모·호환 자모·음절, ₩, CJK 기호·전각 문자)만 다른 서체로 그린다.
  // 서체 이름이 같으면 같은 굵기가 쓰인다.
  const TSharedPtr<FJsonObject>* Hangul;
  if (Spec->TryGetObjectField(TEXT("hangul"), Hangul)) {
    FCompositeSubFont& Sub = Font->SubTypefaces.AddDefaulted_GetRef();
    static const TPair<int32, int32> Ranges[] = {{0x1100, 0x11FF}, {0x20A9, 0x20A9}, {0x3000, 0x303F}, {0x3130, 0x318F}, {0xAC00, 0xD7AF}, {0xFF00, 0xFFEF}};
    for (const TPair<int32, int32>& Range : Ranges)
      Sub.CharacterRanges.Add(FInt32Range(FInt32RangeBound::Inclusive(Range.Key), FInt32RangeBound::Inclusive(Range.Value)));
    AppendTypefaces(Sub.Typeface, *Hangul, PluginDir, OutMissing);
    if (Sub.Typeface.Fonts.IsEmpty()) Font->SubTypefaces.Pop();
  }
  // 그 밖에 글꼴에 없는 글자(일본어·중국어 등 다른 언어)는 엔진의 다국어 대체 글꼴로(빈 네모가 나오지 않게).
  const FString Fallback = FPaths::EngineContentDir() / TEXT("Slate/Fonts/DroidSansFallback.ttf");
  if (FPaths::FileExists(Fallback)) Font->FallbackTypeface.Typeface.AppendFont(TEXT("Regular"), Fallback, EFontHinting::Default, EFontLoadingPolicy::LazyLoad);
  Cache.Add(CacheKey, Font);
  return Font;
}

TArray<FDrUIStyleInfo> GLoadedStyles;

/** margin: 숫자 하나(네 변 같음) 또는 [왼, 위, 오른, 아래]. */
FMargin ReadMargin(const TSharedPtr<FJsonObject>& Spec) {
  double All = 0.;
  if (Spec->TryGetNumberField(TEXT("margin"), All)) return FMargin(float(All));
  const TArray<TSharedPtr<FJsonValue>>* Sides;
  if (Spec->TryGetArrayField(TEXT("margin"), Sides) && Sides->Num() == 4)
    return FMargin(float((*Sides)[0]->AsNumber()), float((*Sides)[1]->AsNumber()), float((*Sides)[2]->AsNumber()), float((*Sides)[3]->AsNumber()));
  return FMargin(0.f);
}
}

void DrUIStyle::ApplyImages(const TSharedPtr<FJsonObject>& Style, const FString& PluginDir, FName StyleId, const TArray<UDrUITheme*>& Themes, FString& OutError) {
  const TSharedPtr<FJsonObject>* ImagesJson;
  if (!Style || !Style->TryGetObjectField(TEXT("images"), ImagesJson)) return;
  TArray<FString> Missing;
  const FString Prefix = StyleId.ToString() + TEXT("-");
  for (UDrUITheme* Theme : Themes) {
    // {mode}는 tokens.json의 테마 id(light, dark …). 테마 id = "<스타일 id>-<테마 id>".
    const FString ThemeName = Theme->ThemeId.ToString();
    const FString Mode = ThemeName.StartsWith(Prefix) ? ThemeName.Mid(Prefix.Len()) : ThemeName;
    for (const auto& [Token, Value] : (*ImagesJson)->Values) {
      const TSharedPtr<FJsonObject> Spec = Value->AsObject();
      FString File;
      if (!Spec || !Spec->TryGetStringField(TEXT("file"), File)) continue;
      FDrUIImage Image;
      const FString Resolved = File.Replace(TEXT("{mode}"), *Mode);
      Image.Path = FPaths::ConvertRelativePathToFull(Resolved.StartsWith(TEXT("{")) ? ResolveFontPath(Resolved, PluginDir) : PluginDir / Resolved);
      if (!FPaths::FileExists(Image.Path)) { Missing.AddUnique(Image.Path); continue; }
      FString Fit;
      if (Spec->TryGetStringField(TEXT("fit"), Fit) && Fit == TEXT("cover")) Image.Fit = EDrUIImageFit::Cover;
      Image.Margin = ReadMargin(Spec);
      double Scale = 1.;
      if (Spec->TryGetNumberField(TEXT("scale"), Scale) && Scale > 0.) Image.Scale = float(Scale);
      FString Tint;
      if (Spec->TryGetStringField(TEXT("tint"), Tint)) ParseCssColor(Tint, Image.Tint);
      Theme->Images.Add(*Token, Image);
    }
  }
  if (!Missing.IsEmpty()) OutError += FString::Printf(TEXT(" 이미지 파일 없음(색으로 칠함): %s"), *FString::Join(Missing, TEXT(", ")));
}

TArray<UDrUITheme*> DrUIStyle::LoadPluginStyle(const FString& PluginName, UObject* Outer, FString& OutError, FDrUIStyleInfo* OutInfo) {
  const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(PluginName);
  if (!Plugin) { OutError = PluginName + TEXT(" 플러그인 없음"); return {}; }
  const FString Dir = Plugin->GetBaseDir();
  FString TokensJson, StyleJson;
  if (!FFileHelper::LoadFileToString(TokensJson, *(Dir / TEXT("Tokens/tokens.json")))) { OutError = PluginName + TEXT(": Tokens/tokens.json 없음"); return {}; }
  if (!FFileHelper::LoadFileToString(StyleJson, *(Dir / TEXT("Tokens/style.json")))) { OutError = PluginName + TEXT(": Tokens/style.json 없음"); return {}; }
  TSharedPtr<FJsonObject> Style;
  if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(StyleJson), Style) || !Style) { OutError = PluginName + TEXT(": style.json을 읽을 수 없음"); return {}; }
  FDrUIStyleInfo Info;
  Info.Id = *Style->GetStringField(TEXT("id"));
  Info.Name = FText::FromString(Style->GetStringField(TEXT("name")));
  Info.Plugin = PluginName;
  Style->TryGetNumberField(TEXT("order"), Info.Order);
  if (Info.Id.IsNone()) { OutError = PluginName + TEXT(": style.json에 id 없음"); return {}; }

  TArray<UDrUITheme*> Themes = BuildThemes(TokensJson, Info.Id, Outer, OutError);
  if (!OutError.IsEmpty()) OutError = PluginName + TEXT(": ") + OutError;
  TSet<FName> Translucent;
  const TArray<TSharedPtr<FJsonValue>>* TranslucentJson;
  if (Style->TryGetArrayField(TEXT("translucent"), TranslucentJson)) for (const TSharedPtr<FJsonValue>& V : *TranslucentJson) Translucent.Add(*V->AsString());
  double StrokeWidth = 0.;
  Style->TryGetNumberField(TEXT("strokeWidth"), StrokeWidth);
  TMap<FName, TSharedPtr<const FCompositeFont>> Fonts;
  TArray<FString> Missing;
  const TSharedPtr<FJsonObject>* FontsJson;
  if (Style->TryGetObjectField(TEXT("fonts"), FontsJson))
    for (const auto& [Family, Spec] : (*FontsJson)->Values)
      if (TSharedPtr<const FCompositeFont> Font = MakeFamilyFont(PluginName + TEXT("/") + Family, Spec->AsObject(), Dir, Missing)) Fonts.Add(*Family, Font);
  if (!Missing.IsEmpty()) OutError += FString::Printf(TEXT(" %s: 글꼴 파일 없음(엔진 기본으로 대체): %s"), *PluginName, *FString::Join(Missing, TEXT(", ")));
  for (UDrUITheme* Theme : Themes) {
    Theme->TranslucentTokens = Translucent;
    Theme->StrokeWidth = float(StrokeWidth);
    Theme->RuntimeFonts = Fonts;
    if (const TSharedPtr<const FCompositeFont>* Sans = Fonts.Find(TEXT("sans"))) Theme->RuntimeFont = *Sans;
    Info.ThemeIds.Add(Theme->ThemeId);
  }
  ApplyImages(Style, Dir, Info.Id, Themes, OutError);
  if (OutInfo) *OutInfo = Info;
  return Themes;
}

TArray<FString> DrUIStyle::FindStylePlugins() {
  TArray<TPair<int32, FString>> Found;
  for (const TSharedRef<IPlugin>& Plugin : IPluginManager::Get().GetEnabledPlugins()) {
    FString StyleJson;
    if (!FFileHelper::LoadFileToString(StyleJson, *(Plugin->GetBaseDir() / TEXT("Tokens/style.json")))) continue;
    TSharedPtr<FJsonObject> Style;
    int32 Order = 100;
    if (FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(StyleJson), Style) && Style) Style->TryGetNumberField(TEXT("order"), Order);
    Found.Add({Order, Plugin->GetName()});
  }
  Found.Sort([](const TPair<int32, FString>& A, const TPair<int32, FString>& B) { return A.Key != B.Key ? A.Key < B.Key : A.Value < B.Value; });
  TArray<FString> Names;
  for (const auto& [Order, Name] : Found) Names.Add(Name);
  return Names;
}

void DrUIStyle::LoadAllStyles(UDrUIThemeSubsystem& Subsystem) {
  GLoadedStyles.Reset();
  for (const FString& PluginName : FindStylePlugins()) {
    FString Error;
    FDrUIStyleInfo Info;
    const TArray<UDrUITheme*> Themes = LoadPluginStyle(PluginName, &Subsystem, Error, &Info);
    if (!Error.IsEmpty()) UE_LOG(LogDrUIKit, Error, TEXT("스타일 %s"), *Error);
    if (Themes.IsEmpty()) continue;
    for (UDrUITheme* Theme : Themes) Subsystem.AddTheme(Theme);
    GLoadedStyles.Add(Info);
  }
  // 설정의 시작 테마는 서브시스템이 뜰 때는 아직 없었다(스타일은 엔진이 뜬 뒤에 들어온다).
  if (const FName Start = GetDefault<UDrUIKitSettings>()->StartTheme; !Start.IsNone()) Subsystem.SetTheme(Start);
}

const TArray<FDrUIStyleInfo>& DrUIStyle::GetLoadedStyles() { return GLoadedStyles; }
