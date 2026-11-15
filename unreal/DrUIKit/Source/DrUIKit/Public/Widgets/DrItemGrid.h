#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/DrItemSlot.h"
#include "DrItemGrid.generated.h"

class UUniformGridPanel;

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
