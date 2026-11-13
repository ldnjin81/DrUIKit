#pragma once
#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Widgets/DrButtonBase.h"
#include "DrListRow.generated.h"

class UDrGlyph;
class UDrSurface;
class UDrText;

/** 목록 항목 데이터(제목·보조 글·오른쪽 글). UListView·UCommonListView에 AddItem으로 넣는다. 게임 데이터 오브젝트를 바로 넣으려면 행을 상속해 NativeOnListItemObjectSet을 덮어쓴다. */
UCLASS(BlueprintType)
class DRUIKIT_API UDrListItem : public UObject {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Title;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Subtitle;
  /** 오른쪽 끝 보조 글(시간·수량 등). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Trailing;
  static UDrListItem* Make(UObject* Outer, const FText& InTitle, const FText& InSubtitle = FText::GetEmpty(), const FText& InTrailing = FText::GetEmpty());
};

/**
 * 목록 행. 호버하면 옅은 렌즈(둥근 hairline 바탕)가 깔리고, 선택되면 accent-soft 바탕, 포커스는 2px 링.
 * CommonUI 버튼이라 패드·키보드로 이동·선택된다. 쓰는 법 두 가지:
 *  - 짧은 목록(저장 슬롯·설정): 스크롤 상자에 직접 넣고 UCommonButtonGroupBase로 하나만 선택(SetIsSelectable(true)).
 *  - 긴 목록(가상화): UCommonListView의 EntryWidgetClass로 쓴다. 단 엔진이 에디터에서는 항목이 위젯 블루프린트이기를 요구하므로
 *    이 클래스를 부모로 한 위젯 블루프린트를 만들어 지정한다(UListViewBase::RebuildWidget 검사, 덮어쓸 수 없음). 선택은 목록이 맡고 버튼 선택과 자동으로 맞춰진다.
 */
UCLASS()
class DRUIKIT_API UDrListRow : public UDrButtonBase, public IUserObjectListEntry {
  GENERATED_BODY()
public:
  UDrListRow(const FObjectInitializer& Initializer);
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Title;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Subtitle;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Trailing;
  /** 오른쪽 끝 화살표(눌러서 들어가는 행). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") bool bShowChevron = true;
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetContent(const FText& InTitle, const FText& InSubtitle, const FText& InTrailing);
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Surface;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> TitleText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> SubtitleText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> TrailingText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrGlyph> Chevron;
  virtual void BuildDefaultTree() override;
  virtual void ApplyVisualState() override;
  virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
};
