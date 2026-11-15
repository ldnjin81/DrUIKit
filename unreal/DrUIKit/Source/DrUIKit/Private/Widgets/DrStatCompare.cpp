#include "Widgets/DrStatCompare.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrGlyph.h"
#include "Widgets/DrStatus.h"
#include "Widgets/DrSurface.h"
#include "Widgets/DrText.h"

int32 UDrStatCompare::CompareDelta(const FDrStatRow& Row) {
  const float Scale = FMath::Pow(10.f, float(Row.Decimals));
  const float Delta = FMath::RoundToFloat(Row.New * Scale) - FMath::RoundToFloat(Row.Current * Scale);  // 보이는 자릿수에서 같으면 같음
  if (Delta == 0.f) return 0;
  return (Delta > 0.f) == Row.bHigherIsBetter ? 1 : -1;
}

void UDrStatCompare::SetRows(const TArray<FDrStatRow>& InRows) { Rows = InRows; Fill(); }

TSharedRef<SWidget> UDrStatCompare::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) {
    List = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("List"));
    WidgetTree->RootWidget = List;
    Fill();
  }
  return Super::RebuildWidget();
}

void UDrStatCompare::NativeConstruct() {
  Super::NativeConstruct();
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get(); Subsystem && !ThemeHandle.IsValid())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
}

void UDrStatCompare::NativeDestruct() {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Super::NativeDestruct();
}

void UDrStatCompare::ApplyTheme() { Fill(); }

void UDrStatCompare::Fill() {
  if (!List || !WidgetTree) return;
  List->ClearChildren();
  auto Number = [](float Value, int32 Decimals) {
    FNumberFormattingOptions Options;
    Options.SetMinimumFractionalDigits(Decimals).SetMaximumFractionalDigits(Decimals);
    return FText::AsNumber(Value, &Options);
  };
  for (const FDrStatRow& Row : Rows) {
    const int32 Delta = CompareDelta(Row);
    const FName Tone = Delta > 0 ? DrUITokens::Success : Delta < 0 ? DrUITokens::Danger : DrUITokens::InkFaint;
    UHorizontalBox* Line = WidgetTree->ConstructWidget<UHorizontalBox>();
    auto Add = [Line](UWidget* W, bool bFill, float MinPad) {
      if (UHorizontalBoxSlot* S = Line->AddChildToHorizontalBox(W)) {
        S->SetVerticalAlignment(VAlign_Center);
        S->SetPadding(FMargin(MinPad, 0, 0, 0));
        if (bFill) S->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
      }
    };
    UDrText* Label = WidgetTree->ConstructWidget<UDrText>();
    Label->SetTokens(DrUITokens::Label, DrUITokens::InkMuted);
    Label->SetText(Row.Label);
    Add(Label, true, 0.f);
    UDrText* Current = WidgetTree->ConstructWidget<UDrText>();
    Current->SetTokens(DrUITokens::Body, DrUITokens::InkMuted);
    Current->SetText(Number(Row.Current, Row.Decimals));
    Add(Current, false, 12.f);
    UDrGlyph* Arrow = WidgetTree->ConstructWidget<UDrGlyph>();
    Arrow->GlyphSize = 14.f;
    Arrow->SetShape(Delta == 0 ? EDrGlyphShape::Minus : (Row.New > Row.Current ? EDrGlyphShape::ArrowUp : EDrGlyphShape::ArrowDown));
    Arrow->SetColorToken(Tone);
    Add(Arrow, false, 8.f);
    UDrText* New = WidgetTree->ConstructWidget<UDrText>();
    New->SetTokens(DrUITokens::Body, Delta == 0 ? DrUITokens::InkMuted : Tone);
    New->SetText(Number(Row.New, Row.Decimals));
    Add(New, false, 6.f);
    if (UVerticalBoxSlot* S = List->AddChildToVerticalBox(Line)) S->SetPadding(FMargin(0, 3));
  }
}
