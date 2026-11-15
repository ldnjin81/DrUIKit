#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/DrUIMotion.h"
#include "Widgets/DrButtonBase.h"
#include "Widgets/DrGlyph.h"
#include "DrInventory.generated.h"

class UDrBadge;
class UDrSurface;
class UDrText;
class USizeBox;
class UUniformGridPanel;
class UVerticalBox;

/** 희귀도. 칸 테두리·이름 색을 정한다(토큰은 UDrItemSlot::RarityToken). */
UENUM(BlueprintType)
enum class EDrItemRarity : uint8 { Common, Uncommon, Rare, Epic, Legendary };

/** 칸 하나의 내용. 아이콘은 게임이 정한다(코어는 에셋 없음): Icon 브러시가 없으면 Glyph 모양을 그린다. */
USTRUCT(BlueprintType)
struct DRUIKIT_API FDrItemData {
  GENERATED_BODY()
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Name;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") EDrGlyphShape Glyph = EDrGlyphShape::Dot;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FSlateBrush Icon;
  /** 2 이상이면 오른쪽 아래에 수량. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") int32 Count = 1;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") EDrItemRarity Rarity = EDrItemRarity::Common;
  /** 장착 중(왼쪽 위 E 표시). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") bool bEquipped = false;
  /** 새로 얻음(오른쪽 위 점, 포커스를 받으면 사라지고 OnSeen). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") bool bNew = false;
  /** 빈 칸(이름·아이콘 없음, 그래도 포커스는 받는다). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") bool bEmpty = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDrItemSlotEvent, int32, Index);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDrItemHintEvent, int32, Index, FName, HintId);

/**
 * 인벤토리 칸. 정사각 표면 + 아이콘(브러시 또는 표시 모양) + 희귀도 테두리·아래 띠 + 수량 + 장착 E + NEW 점.
 * 패드·키보드로 포커스가 오면 조금 커지고(움직임 줄이기면 그대로) 포커스 테두리. NEW는 포커스·호버를 받으면 지운다.
 */
UCLASS()
class DRUIKIT_API UDrItemSlot : public UDrButtonBase {
  GENERATED_BODY()
public:
  UDrItemSlot(const FObjectInitializer& Initializer);
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FDrItemData Item;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float SlotSize = 64.f;
  /** 칸 번호(격자가 넣는다). */
  int32 Index = INDEX_NONE;
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetItem(const FDrItemData& InItem);
  /** NEW 점이 지워질 때. */
  FSimpleMulticastDelegate OnSeen;
  /** 포커스를 받았을 때(격자가 칸 번호로 알린다). */
  FSimpleMulticastDelegate OnFocusedNative;
  /** 희귀도 → 색 토큰(보통 hairline, 고급 success, 희귀 accent, 영웅 ember, 전설 warning). */
  static FName RarityToken(EDrItemRarity Rarity);
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Surface;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<USizeBox> Frame;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrGlyph> IconGlyph;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<class UImage> IconImage;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> RarityBar;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> CountText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> EquippedMark;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrBadge> NewDot;
  virtual void BuildDefaultTree() override;
  virtual void ApplyVisualState() override;
  virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
  virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
private:
  FDrUISpring Grow;
};

/**
 * 인벤토리 칸 격자. Columns칸씩 줄을 채우고, 패드 십자키·스틱으로 칸을 옮긴다. 줄 끝에서 좌우로 넘기면 이웃 줄로 이어진다(bWrapRows).
 * 칸마다 SlotHints(행동 안내: A 사용, X 버리기 …)를 붙이면 포커스 칸의 행동이 행동 안내 바에 보이고, 그 키를 누르면 OnSlotHint.
 */
UCLASS()
class DRUIKIT_API UDrItemGrid : public UUserWidget {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI", meta = (ClampMin = 1)) int32 Columns = 6;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float SlotSize = 64.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float Gap = 8.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") bool bWrapRows = true;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") TArray<FDrActionHint> SlotHints;
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetItems(const TArray<FDrItemData>& Items);
  UFUNCTION(BlueprintCallable, Category = "DrUI") UDrItemSlot* GetSlot(int32 Index) const;
  UFUNCTION(BlueprintCallable, Category = "DrUI") int32 GetSlotCount() const { return Slots.Num(); }
  /** 칸 Index에서 Direction(-1 왼쪽 / +1 오른쪽)으로 넘길 칸(줄 끝 넘김 포함, 없으면 INDEX_NONE). */
  static int32 WrapNeighbor(int32 Index, int32 Direction, int32 Count, int32 Columns, bool bWrap);
  UPROPERTY(BlueprintAssignable, Category = "DrUI") FDrItemSlotEvent OnSlotFocused;
  UPROPERTY(BlueprintAssignable, Category = "DrUI") FDrItemSlotEvent OnSlotClicked;
  UPROPERTY(BlueprintAssignable, Category = "DrUI") FDrItemHintEvent OnSlotHint;
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UUniformGridPanel> Grid;
  virtual TSharedRef<SWidget> RebuildWidget() override;
private:
  UFUNCTION() void HandleSlotHint(FName HintId);
  void EnsureTree();
  UPROPERTY(Transient) TArray<TObjectPtr<UDrItemSlot>> Slots;
  TArray<FDrItemData> PendingItems;
};

/** 능력치 한 줄(현재 → 새 값). */
USTRUCT(BlueprintType)
struct DRUIKIT_API FDrStatRow {
  GENERATED_BODY()
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Label;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float Current = 0.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float New = 0.f;
  /** 거짓이면 작을수록 좋음(무게·쿨다운 등). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") bool bHigherIsBetter = true;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI", meta = (ClampMin = 0, ClampMax = 3)) int32 Decimals = 0;
};

/**
 * 능력치 비교(지금 장비 → 고른 장비). 줄마다 이름 · 현재 값 · ▲▼ · 새 값. 좋아지면 success, 나빠지면 danger, 같으면 흐리게.
 * 계산은 게임이 아니라 이 부품이 한다(플레이어가 빼기를 하지 않게).
 */
UCLASS()
class DRUIKIT_API UDrStatCompare : public UUserWidget {
  GENERATED_BODY()
public:
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetRows(const TArray<FDrStatRow>& InRows);
  /** +1 좋아짐, -1 나빠짐, 0 같음. */
  static int32 CompareDelta(const FDrStatRow& Row);
  void ApplyTheme();
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UVerticalBox> List;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeConstruct() override;
  virtual void NativeDestruct() override;
private:
  void Fill();
  TArray<FDrStatRow> Rows;
  FDelegateHandle ThemeHandle;
};
