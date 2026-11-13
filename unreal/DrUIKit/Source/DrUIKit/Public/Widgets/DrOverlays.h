#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "Core/DrUIMotion.h"
#include "Widgets/DrNotice.h"
#include "DrOverlays.generated.h"

class SDrCoachScrim;
class UBorder;
class UDrButton;
class UDrIconButton;
class UMenuAnchor;
class UOverlay;
class UVerticalBox;

/**
 * 떠 있는·덮는 부품(배너·바텀시트·코치마크·팝오버). 화면 층(UDrUIFlowSubsystem)이 생기기 전까지는 게임이 화면 맨 위 오버레이에
 * 꽉 차게 놓아 쓴다(배너·팝오버는 제자리). 모두 움직임 줄이기면 바로 나타나고 바로 사라진다.
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDrOverlayEvent);

/** 배너(화면 안 한 줄 알림: 연결 끊김·업데이트 있음). 왼쪽 색 띠 + 표시 + 제목·설명 + 행동 버튼 + 닫기. 닫으면 높이가 접히며 사라진다. */
UCLASS()
class DRUIKIT_API UDrBanner : public UUserWidget {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Title;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Message;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") EDrNoticeTone Tone = EDrNoticeTone::Info;
  /** 비우면 행동 버튼 없음. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText ActionLabel;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") bool bDismissible = true;
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetContent(const FText& InTitle, const FText& InMessage, EDrNoticeTone InTone, const FText& InActionLabel);
  UFUNCTION(BlueprintCallable, Category = "DrUI") void Show();
  UFUNCTION(BlueprintCallable, Category = "DrUI") void Dismiss();
  UPROPERTY(BlueprintAssignable, Category = "DrUI") FDrOverlayEvent OnAction;
  UPROPERTY(BlueprintAssignable, Category = "DrUI") FDrOverlayEvent OnDismissed;
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<USizeBox> Frame;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Surface;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Stripe;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrGlyph> Icon;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> TitleText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> MessageText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrButton> ActionButton;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrIconButton> CloseButton;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeOnInitialized() override;
  virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
  virtual void ApplyTheme();
private:
  void ApplyPresence();
  FDrUISpring Presence;
  float FullHeight = 0.f;
  FDelegateHandle ThemeHandle;
};

/**
 * 바텀시트(아래에서 올라오는 판: 상세·선택지·공유). 뒤는 scrim으로 덮고, 판 밖을 누르거나 Esc·패드 B·손잡이를 아래로 끌면 닫힌다.
 * 화면 맨 위 오버레이에 꽉 차게 둔다. 내용은 SetBody.
 */
UCLASS()
class DRUIKIT_API UDrBottomSheet : public UUserWidget {
  GENERATED_BODY()
public:
  UDrBottomSheet(const FObjectInitializer& Initializer);
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Title;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float SheetWidth = 560.f;
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetBody(UWidget* Content);
  UFUNCTION(BlueprintCallable, Category = "DrUI") void Open();
  UFUNCTION(BlueprintCallable, Category = "DrUI") void Close();
  bool IsOpen() const { return bOpen; }
  UPROPERTY(BlueprintAssignable, Category = "DrUI") FDrOverlayEvent OnClosed;
  /** 끈 거리(px)와 판 높이로 닫을지(높이의 25% 넘게 끌면 닫음). */
  static bool ShouldDismissDrag(float DragY, float SheetHeight) { return SheetHeight > 0.f && DragY > SheetHeight * .25f; }
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UBorder> Scrim;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Sheet;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> TitleText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UBorder> Body;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
  virtual FReply NativeOnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& Event) override;
  virtual FReply NativeOnMouseMove(const FGeometry& Geometry, const FPointerEvent& Event) override;
  virtual FReply NativeOnMouseButtonUp(const FGeometry& Geometry, const FPointerEvent& Event) override;
  virtual FReply NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;
private:
  void ApplyPresence();
  UPROPERTY(Transient) TObjectPtr<UWidget> PendingBody;
  FDrUISpring Presence;
  bool bOpen = false, bDragging = false;
  float DragStartY = 0.f, DragY = 0.f;
  FDelegateHandle ThemeHandle;
};

/** 코치마크의 막(구멍 뚫린 scrim + 구멍 둘레 강조 링). 코치마크가 쓴다. */
UCLASS()
class DRUIKIT_API UDrCoachScrim : public UWidget {
  GENERATED_BODY()
public:
  /** 구멍(이 위젯 로컬 좌표: 왼쪽 위, 크기)과 모서리 반경. */
  void SetHole(FVector2D Position, FVector2D Size, float Radius);
  void ApplyTheme();
protected:
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void SynchronizeProperties() override;
  virtual void ReleaseSlateResources(bool bReleaseChildren) override;
private:
  TSharedPtr<SDrCoachScrim> Slate;
  FDelegateHandle ThemeHandle;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDrCoachEvent, bool, bSkipped);

/**
 * 코치마크(튜토리얼 강조). 화면을 어둡게 덮고 지금 단계의 위젯만 둥근 구멍으로 보이게 한 뒤, 옆에 설명 말풍선(단계 n/N, 제목, 설명, 건너뛰기·다음)을 띄운다.
 * 단계가 바뀌면 구멍이 스프링으로 다음 위젯으로 옮겨 간다. 화면 맨 위 오버레이에 꽉 차게 둔다.
 */
UCLASS()
class DRUIKIT_API UDrCoachMark : public UUserWidget {
  GENERATED_BODY()
public:
  UFUNCTION(BlueprintCallable, Category = "DrUI") void AddStep(UWidget* Target, const FText& InTitle, const FText& InBody);
  UFUNCTION(BlueprintCallable, Category = "DrUI") void Start();
  UFUNCTION(BlueprintCallable, Category = "DrUI") void Next();
  UFUNCTION(BlueprintCallable, Category = "DrUI") void Finish(bool bSkipped);
  bool IsRunning() const { return StepIndex != INDEX_NONE; }
  UPROPERTY(BlueprintAssignable, Category = "DrUI") FDrCoachEvent OnFinished;
  /** 말풍선 왼쪽 위(구멍 아래에 둘 자리가 없으면 위로, 가로는 화면 안으로). */
  static FVector2D PlaceBubble(FVector2D HolePos, FVector2D HoleSize, FVector2D BubbleSize, FVector2D Screen, float Gap = 12.f);
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrCoachScrim> Scrim;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Bubble;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> StepText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> TitleText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> BodyText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrButton> NextButton;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrButton> SkipButton;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
  virtual FReply NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;
private:
  struct FStep { TWeakObjectPtr<UWidget> Target; FText Title, Body; };
  void ShowStep();
  TArray<FStep> Steps;
  int32 StepIndex = INDEX_NONE;
  FDrUISpring HoleX, HoleY, HoleW, HoleH;
  bool bHolePlaced = false;
};

/** 팝오버의 판(유리 + 약한 흐림 + 떠 있는 그림자). 팝오버가 만든다. */
UCLASS()
class DRUIKIT_API UDrPopoverPanel : public UUserWidget {
  GENERATED_BODY()
public:
  UDrPopoverPanel(const FObjectInitializer& Initializer);
  void SetBody(UWidget* Content);
  FSimpleDelegate OnDismiss;
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Surface;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual FReply NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;
private:
  UPROPERTY(Transient) TObjectPtr<UWidget> PendingBody;
};

/**
 * 팝오버(누른 자리 옆에 뜨는 작은 판: 설명·빠른 설정·공유). 엔진 UMenuAnchor가 위치(창 안으로 맞춤)·바깥 누르면 닫기를 맡는다.
 * SetTrigger로 누를 위젯을 넣으면(CommonUI 버튼이면 누르면 열고 닫음) 그 옆에 SetBody 내용이 유리 판으로 뜬다.
 */
UCLASS()
class DRUIKIT_API UDrPopover : public UUserWidget {
  GENERATED_BODY()
public:
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetTrigger(UWidget* Trigger);
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetBody(UWidget* Content);
  UFUNCTION(BlueprintCallable, Category = "DrUI") void Open();
  UFUNCTION(BlueprintCallable, Category = "DrUI") void Close();
  UFUNCTION(BlueprintCallable, Category = "DrUI") void Toggle();
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UMenuAnchor> Anchor;
  virtual TSharedRef<SWidget> RebuildWidget() override;
private:
  UFUNCTION() UUserWidget* HandleGetMenu();
  UPROPERTY(Transient) TObjectPtr<UDrPopoverPanel> Panel;
  UPROPERTY(Transient) TObjectPtr<UWidget> PendingTrigger;
  UPROPERTY(Transient) TObjectPtr<UWidget> PendingBody;
};
