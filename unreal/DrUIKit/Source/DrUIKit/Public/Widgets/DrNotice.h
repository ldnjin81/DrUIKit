#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/DrUIMotion.h"
#include "Widgets/DrGlyph.h"
#include "DrNotice.generated.h"

class UDrButton;
class UDrSurface;
class UDrText;
class USizeBox;
class UVerticalBox;

/** 알림 부품(토스트·스낵바). 잠깐 떠서 알리고 스스로 사라진다. 확인 대화상자는 화면 층(UI 흐름)과 함께 따로 만든다. */

UENUM(BlueprintType)
enum class EDrNoticeTone : uint8 { Info, Success, Warning, Danger };

/**
 * 잠깐 떴다 사라지는 부품의 공통 틀. 나타남 정도(스프링 0~1)로 불투명도와 위치를 움직이고, 보이는 시간이 끝나면 스스로 물러난다.
 * 움직임 줄이기면 바로 나타나고 바로 사라진다. 파생은 BuildDefaultTree·ApplyTheme·HandleLeft만 채운다.
 */
UCLASS(Abstract)
class DRUIKIT_API UDrNoticeBase : public UUserWidget {
  GENERATED_BODY()
public:
  /** 나타날 때 출발하는 위치(px). 위에서 내려오면 Y가 음수. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FVector2D EnterOffset = FVector2D(0.f, -12.f);

  /** 나타난다. Seconds가 0 이하면 Leave를 부를 때까지 남는다. */
  void Enter(float Seconds);
  /** 물러난다. 다 사라지면 HandleLeft. */
  void Leave();
  bool IsShown() const { return bShown; }
  bool IsLeaving() const { return bShown && Presence.Target <= 0.f; }

protected:
  virtual void BuildDefaultTree() {}
  virtual void ApplyTheme() {}
  /** 다 사라진 뒤(접힌 상태). */
  virtual void HandleLeft() {}
  /** 보이는 동안의 표시 방법(토스트는 눌리지 않게 HitTestInvisible). */
  virtual ESlateVisibility ShownVisibility() const { return ESlateVisibility::SelfHitTestInvisible; }

  /** 참이면 남은 시간이 줄지 않는다(마우스가 올라와 있는 동안 등). */
  bool bHoldTimer = false;

  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeOnInitialized() override;
  virtual void NativeDestruct() override;
  virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;

private:
  void ApplyPresence();
  FDrUISpring Presence;
  bool bShown = false;
  float Remaining = 0.f;
  FDelegateHandle ThemeHandle;
};

/** 토스트 하나(유리 알약 + 색 표시 + 한 줄 글). 눌리지 않는다. 여러 개는 UDrToastStack이 쌓는다. */
UCLASS()
class DRUIKIT_API UDrToast : public UDrNoticeBase {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Message;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") EDrNoticeTone Tone = EDrNoticeTone::Info;
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetContent(const FText& InMessage, EDrNoticeTone InTone);
  /** 색 표시의 모양과 색 토큰. */
  static void ToneGlyph(EDrNoticeTone Tone, EDrGlyphShape& OutShape, FName& OutColor);
  /** 남은 토스트가 다 사라지면 쌓개가 지운다. */
  FSimpleMulticastDelegate OnLeft;
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Surface;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrGlyph> Icon;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> MessageText;
  virtual void BuildDefaultTree() override;
  virtual void ApplyTheme() override;
  virtual void HandleLeft() override { OnLeft.Broadcast(); }
  virtual ESlateVisibility ShownVisibility() const override { return ESlateVisibility::HitTestInvisible; }
};

/**
 * 토스트 쌓개. 화면 위(또는 아래) 가운데에 두면 Push한 토스트가 차례로 쌓였다가 각자 시간이 끝나면 사라진다.
 * MaxVisible을 넘으면 가장 오래된 것부터 먼저 물러난다. 새 토스트는 아래에 붙는다.
 */
UCLASS()
class DRUIKIT_API UDrToastStack : public UUserWidget {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") TSubclassOf<UDrToast> ToastClass;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI", meta = (ClampMin = 1)) int32 MaxVisible = 3;
  /** 토스트 사이 간격(px). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float Gap = 8.f;
  UFUNCTION(BlueprintCallable, Category = "DrUI") UDrToast* Push(const FText& Message, EDrNoticeTone Tone = EDrNoticeTone::Info, float Seconds = 3.f);
  /** 지금 떠 있는(물러나는 중이 아닌) 토스트 수. */
  int32 GetActiveCount() const;
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UVerticalBox> Stack;
  virtual TSharedRef<SWidget> RebuildWidget() override;
};

/** 알림 피드의 한 줄(작은 유리 칩 + 색 표시 + 글 + 겹친 수 "×3"). 왼쪽에서 밀려 들어온다. */
UCLASS()
class DRUIKIT_API UDrFeedEntry : public UDrNoticeBase {
  GENERATED_BODY()
public:
  UDrFeedEntry(const FObjectInitializer& Initializer);
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Message;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") EDrNoticeTone Tone = EDrNoticeTone::Info;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") int32 Count = 1;
  void SetContent(const FText& InMessage, EDrNoticeTone InTone, int32 InCount);
  FSimpleMulticastDelegate OnLeft;
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Surface;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrGlyph> Icon;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> MessageText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> CountText;
  virtual void BuildDefaultTree() override;
  virtual void ApplyTheme() override;
  virtual void HandleLeft() override { OnLeft.Broadcast(); }
  virtual ESlateVisibility ShownVisibility() const override { return ESlateVisibility::HitTestInvisible; }
};

/**
 * 알림 피드(획득 로그·이벤트 줄). 화면 구석에 두면 Push한 줄이 아래에 쌓였다가 시간이 지나면 사라진다.
 * 방금 들어온 줄과 같은 글이 또 오면 새 줄 대신 그 줄의 수를 올리고 시간을 다시 센다("철광석 ×3"). MaxVisible을 넘으면 오래된 줄부터 물러난다.
 */
UCLASS()
class DRUIKIT_API UDrFeed : public UUserWidget {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") TSubclassOf<UDrFeedEntry> EntryClass;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI", meta = (ClampMin = 1)) int32 MaxVisible = 5;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float Gap = 4.f;
  UFUNCTION(BlueprintCallable, Category = "DrUI") UDrFeedEntry* Push(const FText& Message, EDrNoticeTone Tone = EDrNoticeTone::Info, float Seconds = 4.f);
  int32 GetActiveCount() const;
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UVerticalBox> Stack;
  virtual TSharedRef<SWidget> RebuildWidget() override;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDrSnackbarAction, int32, RequestId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDrSnackbarDismissed, int32, RequestId, bool, bByAction);

/**
 * 스낵바(진한 알약 + 글 + 행동 버튼 하나, 예: "삭제했어요 [되돌리기]"). 한 번에 하나만 보이고 나머지는 차례를 기다린다.
 * 마우스가 올라와 있는 동안은 시간이 멈춘다. 화면 아래 가운데에 두는 것이 기본(아래에서 올라온다).
 */
UCLASS()
class DRUIKIT_API UDrSnackbar : public UDrNoticeBase {
  GENERATED_BODY()
public:
  UDrSnackbar(const FObjectInitializer& Initializer);
  /** 보여 줄 요청을 넣는다(앞 요청이 있으면 차례를 기다림). ActionLabel이 비면 버튼 없음. Seconds가 0 이하면 닫을 때까지. 요청 번호를 돌려준다. */
  UFUNCTION(BlueprintCallable, Category = "DrUI") int32 Show(const FText& Message, const FText& ActionLabel, float Seconds = 4.f);
  /** 지금 요청을 닫는다(다음 요청이 이어서 뜬다). */
  UFUNCTION(BlueprintCallable, Category = "DrUI") void Dismiss();
  int32 GetCurrentId() const { return Current.Id; }
  int32 GetQueuedCount() const { return Queue.Num(); }

  UPROPERTY(BlueprintAssignable, Category = "DrUI") FDrSnackbarAction OnAction;
  UPROPERTY(BlueprintAssignable, Category = "DrUI") FDrSnackbarDismissed OnDismissed;
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Surface;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> MessageText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrButton> ActionButton;
  virtual void BuildDefaultTree() override;
  virtual void ApplyTheme() override;
  virtual void HandleLeft() override;
  /** 눌리고 호버를 받아야 하므로 Visible(뒤로 클릭이 새지 않음). */
  virtual ESlateVisibility ShownVisibility() const override { return ESlateVisibility::Visible; }
  virtual void NativeOnInitialized() override;
  virtual void NativeOnMouseEnter(const FGeometry& Geometry, const FPointerEvent& Event) override;
  virtual void NativeOnMouseLeave(const FPointerEvent& Event) override;
private:
  struct FRequest { int32 Id = 0; FText Message, ActionLabel; float Seconds = 0.f; };
  void Present(const FRequest& Request);
  void HandleActionClicked();
  FRequest Current;
  TArray<FRequest> Queue;
  int32 NextId = 1;
  bool bByAction = false;
};
