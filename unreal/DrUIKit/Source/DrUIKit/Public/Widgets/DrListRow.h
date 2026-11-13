#pragma once
#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "CommonListView.h"
#include "Widgets/DrButtonBase.h"
#include "DrListRow.generated.h"

class UDrGlyph;
class UDrSurface;
class UDrText;

/** 목록 항목 데이터(제목·보조 글·오른쪽 글). UDrListView에 AddItem으로 넣는다. 게임 데이터 오브젝트를 바로 넣으려면 행을 상속해 NativeOnListItemObjectSet을 덮어쓴다. */
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
 * CommonUI 버튼이라 패드·키보드로 이동·선택되고, UCommonListView 안에서는 목록이 선택을 맡는다(버튼 선택과 자동으로 맞춰짐).
 * 목록 밖에서 단독으로도 쓴다(Title 등을 직접 채움).
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

/** DrUIKit 목록. CommonUI 목록에 행 기본값(UDrListRow), 투명 바탕, 행 간격만 정했다. 다른 행 클래스를 쓰려면 EntryWidgetClass를 바꾼다. */
UCLASS()
class DRUIKIT_API UDrListView : public UCommonListView {
  GENERATED_BODY()
public:
  UDrListView(const FObjectInitializer& Initializer);
};
