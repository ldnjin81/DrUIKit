#include "Theme/DrUIGlyphImages.h"
#include "DrUIKit.h"
#include "Dom/JsonObject.h"
#include "Engine/Texture2D.h"
#include "ImageUtils.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "UObject/StrongObjectPtr.h"

namespace {
struct FEntry {
  FString Path;
  int32 Order = 100;
  bool bTint = true;
  TStrongObjectPtr<UTexture2D> Texture;
  bool bTried = false;
};
/** "세트/키" → 이미지. 같은 키는 order가 작은 세트가 이긴다. */
TMap<FString, FEntry> GEntries;
TArray<FDrUIGlyphImageSetInfo> GSets;

FString EntryKey(FName SetName, const FKey& Key) { return SetName.ToString().ToLower() + TEXT("/") + Key.GetFName().ToString(); }

FName SetNameFor(EDrUIGamepadGlyphs Glyphs, const FKey& Key) {
  if (!Key.IsGamepadKey()) return TEXT("keyboard");
  switch (Glyphs) {
    case EDrUIGamepadGlyphs::PlayStation: return TEXT("playstation");
    case EDrUIGamepadGlyphs::Nintendo: return TEXT("nintendo");
    default: return TEXT("xbox");
  }
}
}

bool DrUIGlyphImages::Register(const FString& Json, const FString& BaseDir, const FString& PluginName, FDrUIGlyphImageSetInfo& OutInfo, FString& OutError) {
  TSharedPtr<FJsonObject> Root;
  if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root) { OutError = TEXT("glyphs.json을 읽을 수 없음"); return false; }
  FDrUIGlyphImageSetInfo Info;
  Info.Id = *Root->GetStringField(TEXT("id"));
  Info.Name = FText::FromString(Root->GetStringField(TEXT("name")));
  Info.Plugin = PluginName;
  Root->TryGetNumberField(TEXT("order"), Info.Order);
  Root->TryGetStringField(TEXT("license"), Info.License);
  Root->TryGetBoolField(TEXT("tint"), Info.bTint);
  const TSharedPtr<FJsonObject>* Sets;
  if (Info.Id.IsNone() || !Root->TryGetObjectField(TEXT("sets"), Sets)) { OutError = TEXT("glyphs.json에 id 또는 sets 없음"); return false; }
  TArray<FString> Missing;
  for (const auto& [SetName, SetValue] : (*Sets)->Values) {
    const TSharedPtr<FJsonObject>* Keys;
    if (!SetValue->TryGetObject(Keys)) continue;
    bool bSetTint = Info.bTint;
    (*Keys)->TryGetBoolField(TEXT("$tint"), bSetTint);
    for (const auto& [KeyName, PathValue] : (*Keys)->Values) {
      if (FString(KeyName).StartsWith(TEXT("$"))) continue;
      const FKey Key(*KeyName);
      const FString Path = BaseDir / PathValue->AsString();
      if (!Key.IsValid() || !FPaths::FileExists(Path)) { Missing.Add(FString(KeyName)); continue; }
      const FString Id = EntryKey(*SetName, Key);
      const FEntry* Existing = GEntries.Find(Id);
      if (Existing && Existing->Order <= Info.Order) continue;  // 먼저 온 더 우선인 세트가 있다
      FEntry Entry;
      Entry.Path = Path;
      Entry.Order = Info.Order;
      Entry.bTint = bSetTint;
      GEntries.Add(Id, MoveTemp(Entry));
      ++Info.ImageCount;
    }
  }
  if (!Missing.IsEmpty()) OutError = FString::Printf(TEXT("모르는 키이거나 파일 없음 %d개: %s"), Missing.Num(), *FString::Join(Missing, TEXT(", ")).Left(300));
  GSets.Add(Info);
  OutInfo = Info;
  return true;
}

void DrUIGlyphImages::LoadAll() {
  for (const TSharedRef<IPlugin>& Plugin : IPluginManager::Get().GetEnabledPlugins()) {
    const FString Dir = Plugin->GetBaseDir() / TEXT("Glyphs");
    FString Json;
    if (!FFileHelper::LoadFileToString(Json, *(Dir / TEXT("glyphs.json")))) continue;
    FDrUIGlyphImageSetInfo Info;
    FString Error;
    Register(Json, Dir, Plugin->GetName(), Info, Error);
    if (!Error.IsEmpty()) { UE_LOG(LogDrUIKit, Warning, TEXT("버튼 이미지 세트 %s: %s"), *Plugin->GetName(), *Error); }
    UE_LOG(LogDrUIKit, Log, TEXT("버튼 이미지 세트 %s: %d개"), *Info.Id.ToString(), Info.ImageCount);
  }
  GSets.Sort([](const FDrUIGlyphImageSetInfo& A, const FDrUIGlyphImageSetInfo& B) { return A.Order < B.Order; });
}

void DrUIGlyphImages::Clear() { GEntries.Reset(); GSets.Reset(); }

const TArray<FDrUIGlyphImageSetInfo>& DrUIGlyphImages::GetLoadedSets() { return GSets; }

FString DrUIGlyphImages::FindPath(FName SetName, const FKey& Key) {
  const FEntry* Entry = GEntries.Find(EntryKey(SetName, Key));
  return Entry ? Entry->Path : FString();
}

UTexture2D* DrUIGlyphImages::Find(EDrUIGamepadGlyphs Glyphs, const FKey& Key, bool* bOutTint, bool bBrand) {
  if (const UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get(); Subsystem && !Subsystem->IsUseGlyphImages()) return nullptr;
  const FName SetName = SetNameFor(Glyphs, Key);
  FEntry* Entry = bBrand ? GEntries.Find(EntryKey(*(SetName.ToString() + TEXT("_brand")), Key)) : nullptr;
  if (!Entry) Entry = GEntries.Find(EntryKey(SetName, Key));
  if (!Entry) return nullptr;
  if (bOutTint) *bOutTint = Entry->bTint;
  if (!Entry->bTried) {
    // 처음 쓸 때 PNG를 텍스처로(에셋 없이). 실패하면 다시 시도하지 않고 벡터로.
    Entry->bTried = true;
    if (UTexture2D* Texture = FImageUtils::ImportFileAsTexture2D(Entry->Path)) Entry->Texture.Reset(Texture);
    else UE_LOG(LogDrUIKit, Warning, TEXT("버튼 이미지를 읽을 수 없음: %s"), *Entry->Path);
  }
  return Entry->Texture.Get();
}
