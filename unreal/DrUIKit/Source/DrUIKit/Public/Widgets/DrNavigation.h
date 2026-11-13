#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/DrUIMotion.h"
#include "Widgets/DrButtonBase.h"
#include "Widgets/DrGlyph.h"
#include "Widgets/DrTabList.h"
#include "DrNavigation.generated.h"

class UCommonButtonGroupBase;
class UDrBadge;
class UDrIconButton;
class UDrSurface;
class UDrText;
class UOverlay;
class USizeBox;
class UVerticalBox;
class UHorizontalBox;

/** 탐색 부품(사이드바·독·하단 탭바). 디자인 언어 GlassSidebar·Dock 명세를 따른다. */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDrNavEvent, FName, ItemId);

/** 사이드바 항목(표시 + 이름 + 개수 배지). 바탕은 칠하지 않는다(사이드바의 유리 렌즈가 깔린다). 레일이면 이름 대신 알림 점. */
UCLASS()
class DRUIKIT_API UDrSidebarItem : public UDrButtonBase {
  GENERATED_BODY()
public:
  UDrSidebarItem(const FObjectInitializer& Initializer);
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName ItemId;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Label;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") EDrGlyphShape Shape = EDrGlyphShape::Dot;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") int32 Count = 0;
  void SetContent(FName InId, const FText& InLabel, EDrGlyphShape InShape, int32 InCount);
  /** 0 레일 ~ 1 펼침(이름·배지가 이만큼 보인다). */
  void SetExpandAmount(float Amount);
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Surface;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrGlyph> Icon;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> LabelText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrBadge> CountBadge;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrBadge> AlertDot;
  virtual void BuildDefaultTree() override;
  virtual void ApplyVisualState() override;
private:
  float ExpandAmount = 1.f;
};

/**
 * 유리 사이드바. 아이콘 레일(60px)과 펼친 패널(250px)을 스프링으로 오간다. 가리킨 항목 아래로 유리 렌즈(glass-strong + shadow-float)가
 * 미끄러지고, 가리키지 않으면 활성 항목에 머문다(활성 항목은 강조색으로 채우지 않는다). 레일에서는 항목마다 툴팁, 개수는 ember 점.
 */
UCLASS()
class DRUIKIT_API UDrSidebar : public UUserWidget {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") bool bExpanded = true;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float RailWidth = 60.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float PanelWidth = 250.f;
  UFUNCTION(BlueprintCallable, Category = "DrUI") UDrSidebarItem* AddItem(FName ItemId, const FText& Label, EDrGlyphShape Shape, int32 Count = 0);
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetExpanded(bool bInExpanded);
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SelectItem(FName ItemId);
  UPROPERTY(BlueprintAssignable, Category = "DrUI") FDrNavEvent OnItemSelected;
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<USizeBox> Frame;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Panel;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrIconButton> ToggleButton;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UOverlay> Layers;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Lens;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UVerticalBox> List;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
private:
  void EnsureTree();
  void HandleItemClicked(UDrSidebarItem* Item);
  void ApplyWidth();
  UPROPERTY(Transient) TArray<TObjectPtr<UDrSidebarItem>> Items;
  UPROPERTY(Transient) TObjectPtr<UDrSidebarItem> Active;
  UPROPERTY(Transient) TObjectPtr<USizeBox> LensBox;
  UPROPERTY(Transient) TObjectPtr<UCommonButtonGroupBase> Group;
  FDrUISpring Width, LensY, LensHeight;
  bool bLensPlaced = false, bTicked = false;
};

/** 독 항목(아이콘 칩 + 활성 점 + 개수 배지). 크기는 독이 정한다. */
UCLASS()
class DRUIKIT_API UDrDockItem : public UDrButtonBase {
  GENERATED_BODY()
public:
  UDrDockItem(const FObjectInitializer& Initializer);
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName ItemId;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") EDrGlyphShape Shape = EDrGlyphShape::Dot;
  /** 칩 채움 색 토큰(앱 색). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName ColorToken = TEXT("accent");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") bool bRunning = false;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") int32 Count = 0;
  /** 확대 배율(독이 스프링으로 넣는다). 칩은 아래 가운데를 축으로 커지고, 자리 너비도 함께 늘어 이웃을 민다. */
  void SetMagnify(float Scale);
  bool IsPointed() const { return IsHovered() || bFocusedNow; }
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<USizeBox> TileBox;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UOverlay> TileLayer;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Tile;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrGlyph> Icon;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> RunningDot;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrBadge> CountBadge;
  virtual void BuildDefaultTree() override;
  virtual void ApplyVisualState() override;
};

/**
 * 독(떠 있는 아이콘 바). 가리킨 아이콘은 1.4배, 이웃은 1.15배로 스프링을 타고 커진다(패드·키보드 포커스도 같음). 활성 항목 아래에는 점.
 * 움직임 줄이기면 확대하지 않는다(확대는 장식). 항목 이름은 툴팁으로.
 */
UCLASS()
class DRUIKIT_API UDrDock : public UUserWidget {
  GENERATED_BODY()
public:
  UFUNCTION(BlueprintCallable, Category = "DrUI") UDrDockItem* AddItem(FName ItemId, const FText& Name, EDrGlyphShape Shape, FName ColorToken, bool bRunning = false, int32 Count = 0);
  UPROPERTY(BlueprintAssignable, Category = "DrUI") FDrNavEvent OnItemClicked;
  /** 가리킨 항목에서 Distance칸 떨어진 항목의 배율(0 → 1.4, 1 → 1.15, 그 밖 1). */
  static float MagnifyFor(int32 Distance);
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Bar;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UHorizontalBox> Row;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
private:
  void EnsureTree();
  UPROPERTY(Transient) TArray<TObjectPtr<UDrDockItem>> Items;
  TArray<FDrUISpring> Scales;
};

/** 하단 탭 버튼(표시 위, 이름 아래). 선택되면 진한 알약 위에서 on-chip. */
UCLASS()
class DRUIKIT_API UDrBottomTabButton : public UDrTabButton {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") EDrGlyphShape Shape = EDrGlyphShape::Dot;
  void SetShape(EDrGlyphShape InShape) { Shape = InShape; ApplyVisualState(); }
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrGlyph> Icon;
  virtual void BuildDefaultTree() override;
  virtual void ApplyVisualState() override;
};

/** 하단 탭바(모바일·태블릿형). UDrTabList의 알약 인디케이터를 그대로 쓰고 버튼만 표시+이름 세로형. 칸 너비는 같다. */
UCLASS()
class DRUIKIT_API UDrBottomTabBar : public UDrTabList {
  GENERATED_BODY()
public:
  UDrBottomTabBar(const FObjectInitializer& Initializer);
  UFUNCTION(BlueprintCallable, Category = "DrUI") bool AddIconTab(FName TabId, const FText& TabLabel, EDrGlyphShape Shape, UWidget* Content = nullptr);
protected:
  virtual void HandleTabCreation_Implementation(FName TabNameID, UCommonButtonBase* TabButton) override;
private:
  TMap<FName, EDrGlyphShape> PendingShapes;
};
