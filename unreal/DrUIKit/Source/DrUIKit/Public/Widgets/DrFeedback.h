#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "Widgets/DrGlyph.h"
#include "DrFeedback.generated.h"

class SDrStepIndicator;
class SDrCompletion;
class UDrButton;
class UDrSurface;
class UDrText;

/** 진행·완료·빈 상태 부품(단계 표시·완료 표시·빈 화면). */

/**
 * 단계 표시(1 — 2 — 3). 끝난 단계는 체크가 든 채운 원, 지금 단계는 강조 링 + 번호, 남은 단계는 옅은 링 + 번호. 이음선은 지금 단계까지 채워진다.
 * 단계가 바뀌면 채움이 스프링으로 따라간다. 이름은 원 아래.
 */
UCLASS()
class DRUIKIT_API UDrStepIndicator : public UWidget {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") TArray<FText> Steps;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") int32 Current = 0;
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetSteps(const TArray<FText>& InSteps);
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetCurrent(int32 Index);
  /** 단계 Index의 원 가운데 x(전체 너비 Width, 단계 Count개, 양 끝은 반 칸 안쪽). */
  static float StepCenterX(int32 Index, int32 Count, float Width);
  void ApplyTheme();
protected:
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void SynchronizeProperties() override;
  virtual void ReleaseSlateResources(bool bReleaseChildren) override;
private:
  TSharedPtr<SDrStepIndicator> Slate;
  FDelegateHandle ThemeHandle;
};

/**
 * 완료 표시(체크 그리기). Play하면 원이 살짝 튀며 커지고 그 안에 체크가 그려지며, 바깥으로 옅은 고리가 한 번 퍼진다.
 * 움직임 줄이기면 바로 끝 모습. 저장·결제·퀘스트 완료 같은 순간에 쓴다.
 */
UCLASS()
class DRUIKIT_API UDrCompletion : public UWidget {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float MarkSize = 64.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName ColorToken = TEXT("success");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName InkToken = TEXT("on-accent");
  /** 처음 보일 때 끝 모습으로 둘지(거짓이면 Play 전까지 비어 있음). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") bool bStartComplete = false;
  UFUNCTION(BlueprintCallable, Category = "DrUI") void Play();
  UFUNCTION(BlueprintCallable, Category = "DrUI") void ResetMark();
  /** 시작 뒤 Seconds초의 (원 크기 0~1.1, 체크 진행 0~1, 고리 0~1). */
  static FVector3f Timeline(float Seconds);
  void ApplyTheme();
protected:
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void SynchronizeProperties() override;
  virtual void ReleaseSlateResources(bool bReleaseChildren) override;
private:
  TSharedPtr<SDrCompletion> Slate;
  FDelegateHandle ThemeHandle;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDrEmptyStateEvent);

/** 빈 화면(이유 + 다음 행동): 옅은 원 안 표시, 제목(왜 비었나), 설명, 주 행동 버튼, 보조 행동(글 버튼). 가운데 정렬. */
UCLASS()
class DRUIKIT_API UDrEmptyState : public UUserWidget {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") EDrGlyphShape Shape = EDrGlyphShape::Plus;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Title;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Body;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText ActionLabel;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText SecondaryLabel;
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetContent(EDrGlyphShape InShape, const FText& InTitle, const FText& InBody, const FText& InActionLabel, const FText& InSecondaryLabel);
  UPROPERTY(BlueprintAssignable, Category = "DrUI") FDrEmptyStateEvent OnAction;
  UPROPERTY(BlueprintAssignable, Category = "DrUI") FDrEmptyStateEvent OnSecondary;
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> IconCircle;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrGlyph> Icon;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> TitleText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> BodyText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrButton> ActionButton;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrButton> SecondaryButton;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void ApplyTheme();
};
