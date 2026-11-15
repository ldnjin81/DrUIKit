#pragma once
#include "CoreMinimal.h"
#include "Core/DrUIMotion.h"
#include "Widgets/DrButtonBase.h"
#include "Widgets/DrGlyph.h"
#include "DrItemSlot.generated.h"

class UDrBadge;
class UDrSurface;
class UDrText;
class UImage;
class USizeBox;

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
