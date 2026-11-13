#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/DrUIMotion.h"
#include "Widgets/DrButtonBase.h"
#include "DrInput.generated.h"

class UDrGlyph;
class UDrSurface;
class UDrText;
class UEditableText;
class UMenuAnchor;
class UMultiLineEditableText;
class USizeBox;
class UVerticalBox;

/** 입력 부품(텍스트 필드·드롭다운·수평 선택기). */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDrTextEvent, const FText&, Text);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDrIndexEvent, int32, Index);

/**
 * 텍스트 필드. 엔진 UEditableText(입력·커서·IME·가상 키보드)를 라벨·입력 칸·도움말 줄로 감싼다.
 * 상태: 기본·호버·포커스(2px 링)·비활성(흐리게)·오류(빨간 테두리 + 오류 문구). MaxLength가 있으면 넘는 글자를 자르고 "3/20"을 보인다.
 */
UCLASS()
class DRUIKIT_API UDrTextField : public UUserWidget {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Label;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Hint;
  /** 오류가 없을 때 아래에 보이는 안내. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Helper;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") bool bPassword = false;
  /** 0이면 제한 없음. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI", meta = (ClampMin = 0)) int32 MaxLength = 0;

  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetText(const FText& InText);
  UFUNCTION(BlueprintCallable, Category = "DrUI") FText GetText() const;
  /** 오류 문구를 보인다. 빈 글이면 오류를 지운다. */
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetError(const FText& InError);
  bool HasError() const { return !Error.IsEmpty(); }
  /** MaxLength를 넘는 글자를 자른다(0이면 그대로). */
  static FText ClampLength(const FText& InText, int32 InMaxLength);

  UPROPERTY(BlueprintAssignable, Category = "DrUI") FDrTextEvent OnTextChanged;
  /** Enter로 확정하거나 포커스를 잃을 때. */
  UPROPERTY(BlueprintAssignable, Category = "DrUI") FDrTextEvent OnTextCommitted;

protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> LabelText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Field;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UEditableText> Input;
  /** 여러 줄이면 Input 대신 이것(UDrTextArea). */
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UMultiLineEditableText> MultiInput;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> HelperText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> CounterText;

  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeOnInitialized() override;
  virtual void NativeDestruct() override;
  virtual void NativeOnMouseEnter(const FGeometry& Geometry, const FPointerEvent& Event) override;
  virtual void NativeOnMouseLeave(const FPointerEvent& Event) override;
  virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
  virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;
  /** 활성 여부는 알림이 없어 틱에서 바뀐 것을 본다. */
  virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
  /** 상태대로 다시 칠한다(호버·포커스·오류·활성이 바뀔 때). */
  virtual void ApplyVisualState();
  /** 글꼴·색·커서·선택 영역을 테마대로(테마가 바뀔 때). 이어서 ApplyVisualState. */
  virtual void ApplyTheme();
  /** 참이면 기본 구성에서 여러 줄 입력을 만든다(UDrTextArea가 켬). */
  bool bMultiLine = false;
  /** 여러 줄일 때 최소 줄 수(입력 칸 최소 높이). */
  int32 MinLines = 1;

private:
  void BindInput();
  void SetInputText(const FText& InText);
  UWidget* GetInputWidget() const;
  UFUNCTION() void HandleInputChanged(const FText& InText);
  UFUNCTION() void HandleInputCommitted(const FText& InText, ETextCommit::Type Method);
  FText Error;
  /** 기본 구성 전에 SetText한 값(구성 직후 넣는다). */
  FText PendingText;
  bool bHovered = false, bFocused = false, bBound = false, bWasEnabled = true, bClamping = false;
  FDelegateHandle ThemeHandle;
};

/** 여러 줄 입력(메모·채팅·설명). 텍스트 필드와 같은 상태·라벨·글자 수에 엔진 UMultiLineEditableText(Enter는 줄바꿈, Ctrl+Enter 확정)를 쓴다. */
UCLASS()
class DRUIKIT_API UDrTextArea : public UDrTextField {
  GENERATED_BODY()
public:
  UDrTextArea(const FObjectInitializer& Initializer);
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI", meta = (ClampMin = 1)) int32 Lines = 4;
protected:
  virtual TSharedRef<SWidget> RebuildWidget() override;
};

/** 드롭다운 목록의 항목 한 줄(글 + 고른 항목 체크). */
UCLASS()
class DRUIKIT_API UDrDropdownItem : public UDrButtonBase {
  GENERATED_BODY()
public:
  UDrDropdownItem(const FObjectInitializer& Initializer);
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Label;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") bool bChecked = false;
  void SetOption(const FText& InLabel, bool bInChecked);
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Surface;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> LabelText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrGlyph> Check;
  virtual void BuildDefaultTree() override;
  virtual void ApplyVisualState() override;
};

/** 드롭다운이 여는 목록(유리 패널). 열리면 고른 항목에 포커스가 가고, Esc·패드 뒤로로 닫힌다. 갤러리처럼 따로 놓아 모양만 볼 수도 있다. */
UCLASS()
class DRUIKIT_API UDrDropdownMenu : public UUserWidget {
  GENERATED_BODY()
public:
  UDrDropdownMenu(const FObjectInitializer& Initializer);
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") TSubclassOf<UDrDropdownItem> OptionClass;
  void SetOptions(const TArray<FText>& InOptions, int32 InSelected, float InMinWidth = 0.f);
  /** 항목을 골랐을 때(번호). */
  TDelegate<void(int32)> OnPicked;
  /** Esc·뒤로로 닫아 달라고 할 때. */
  FSimpleDelegate OnDismiss;
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<USizeBox> Frame;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Surface;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UVerticalBox> List;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual FReply NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;
private:
  void EnsureTree();
  void Fill();
  TArray<FText> Options;
  int32 Selected = INDEX_NONE;
  float MinWidth = 0.f;
};

/**
 * 드롭다운. 누르면 엔진 UMenuAnchor가 아래에 UDrDropdownMenu를 띄운다(창 밖으로 나가지 않게 맞춤).
 * 화살표는 열리면 스프링으로 뒤집힌다. 키보드·패드로 열면 고른 항목부터 포커스.
 */
UCLASS()
class DRUIKIT_API UDrDropdown : public UDrButtonBase {
  GENERATED_BODY()
public:
  UDrDropdown(const FObjectInitializer& Initializer);
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") TArray<FText> Options;
  /** 아무것도 고르지 않았을 때 보이는 글. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Placeholder;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") TSubclassOf<UDrDropdownMenu> MenuClass;

  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetOptions(const TArray<FText>& InOptions);
  /** 범위 밖이면 고르지 않은 상태. bBroadcast면 OnSelectionChanged. */
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetSelectedIndex(int32 Index, bool bBroadcast = false);
  UFUNCTION(BlueprintCallable, Category = "DrUI") int32 GetSelectedIndex() const { return SelectedIndex; }
  bool IsMenuOpen() const;

  UPROPERTY(BlueprintAssignable, Category = "DrUI") FDrIndexEvent OnSelectionChanged;

protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UMenuAnchor> Anchor;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Surface;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> ValueText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrGlyph> Chevron;
  virtual void BuildDefaultTree() override;
  virtual void ApplyVisualState() override;
  virtual void NativeOnInitialized() override;
  virtual void NativeOnClicked() override;
  virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
private:
  void BindAnchor();
  UFUNCTION() UUserWidget* HandleGetMenu();
  UFUNCTION() void HandleMenuOpenChanged(bool bIsOpen);
  int32 SelectedIndex = INDEX_NONE;
  bool bOpen = false;
  FDrUISpring ChevronTurn;
};

/**
 * 수평 선택기(◀ 값 ▶). 게임 설정에서 흔한 값 고르기. 패드·키보드 좌우로 바꾸고, 마우스는 왼쪽 절반을 누르면 이전·오른쪽 절반은 다음.
 * CommonUI의 UCommonRotator와 같은 방식(좌우 탐색을 가로챔)이지만 상태 칠하기를 위해 UDrButtonBase를 상속한다.
 * 값이 바뀌면 글이 바뀐 방향으로 살짝 미끄러져 들어온다. bWrap이 거짓이면 끝에서 멈추고 그쪽 화살표가 흐려진다.
 */
UCLASS()
class DRUIKIT_API UDrHorizontalSelector : public UDrButtonBase {
  GENERATED_BODY()
public:
  UDrHorizontalSelector(const FObjectInitializer& Initializer);
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") TArray<FText> Options;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") bool bWrap = true;
  /** 값 칸 최소 너비(값이 바뀌어도 화살표가 움직이지 않게). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float MinValueWidth = 120.f;

  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetOptions(const TArray<FText>& InOptions);
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetSelectedIndex(int32 Index, bool bBroadcast = false);
  UFUNCTION(BlueprintCallable, Category = "DrUI") int32 GetSelectedIndex() const { return SelectedIndex; }
  /** Direction만큼(-1 이전, +1 다음) 옮긴다. 움직였으면 true. */
  UFUNCTION(BlueprintCallable, Category = "DrUI") bool Step(int32 Direction);
  /** 다음 번호. bInWrap이 거짓이면 끝에서 멈춘다. 항목이 없으면 INDEX_NONE. */
  static int32 StepIndex(int32 Current, int32 Direction, int32 Count, bool bInWrap);

  UPROPERTY(BlueprintAssignable, Category = "DrUI") FDrIndexEvent OnSelectionChanged;

protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Surface;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrGlyph> Prev;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrGlyph> Next;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> ValueText;
  virtual void BuildDefaultTree() override;
  virtual void ApplyVisualState() override;
  virtual void NativeOnClicked() override;
  virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
  virtual FNavigationReply NativeOnNavigation(const FGeometry& Geometry, const FNavigationEvent& Event, const FNavigationReply& DefaultReply) override;
private:
  int32 SelectedIndex = INDEX_NONE;
  /** 글이 들어오는 위치(px, 0이면 제자리). */
  FDrUISpring Slide;
};
