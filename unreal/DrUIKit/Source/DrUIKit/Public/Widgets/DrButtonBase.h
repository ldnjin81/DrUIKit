#pragma once
#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "Core/DrUIMotion.h"
#include "Core/DrUIState.h"
#include "DrButtonBase.generated.h"

/** 그리지 않는 버튼 스타일. 모양은 DrUIKit 부품이 칠한다. */
UCLASS()
class DRUIKIT_API UDrButtonStyle : public UCommonButtonStyle {
  GENERATED_BODY()
public:
  UDrButtonStyle();
};

/**
 * 누르는 부품(버튼·체크박스·라디오·스위치·탭 버튼)의 공통 틀. CommonUI 버튼을 상속해 입력·포커스·패드·선택은 엔진이 맡고,
 * 여기서는 호버·눌림·포커스 등을 모아 상태 하나로 판정(DrUIState::Resolve)하고 테마가 바뀌면 다시 칠한다.
 * 파생은 BuildDefaultTree(기본 구성)와 ApplyVisualState(칠하기)만 채운다.
 */
UCLASS(Abstract)
class DRUIKIT_API UDrButtonBase : public UCommonButtonBase {
  GENERATED_BODY()
public:
  UDrButtonBase(const FObjectInitializer& Initializer);

  EDrUIState GetVisualState() const { return VisualState; }
  virtual FDrUIStateInputs GetStateInputs() const;

protected:
  /** 위젯 블루프린트가 루트를 두지 않았을 때 C++ 기본 구성을 만든다. */
  virtual void BuildDefaultTree() {}
  /** 상태가 바뀌거나 테마가 바뀌면 다시 칠한다. 게임은 이것만 덮어써도 모양을 바꿀 수 있다. */
  virtual void ApplyVisualState() {}
  /** 상태를 다시 판정하고 칠한다. */
  void RefreshState();
  /** 움직임을 써도 되는지. 움직임 줄이기가 켜졌거나 아직 한 번도 틱하지 않았으면(처음 보일 때) false: 스프링을 바로 맞춘다. */
  bool CanAnimate() const;
  /** 스프링 하나를 목표로 보낸다. 움직일 수 없으면 바로 맞춘다. */
  void DriveSpring(FDrUISpring& Spring, float Target) const;

  /** 눌렸을 때 크기(1이면 줄지 않음). */
  float PressedScale = .97f;
  bool bPressedNow = false, bFocusedNow = false;
  EDrUIState VisualState = EDrUIState::Default;
  FDrUISpring Scale;

  virtual void InitializeNativeClassData() override;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeOnInitialized() override;
  virtual void NativeDestruct() override;
  virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
  virtual void NativeOnHovered() override;
  virtual void NativeOnUnhovered() override;
  virtual void NativeOnPressed() override;
  virtual void NativeOnReleased() override;
  virtual void NativeOnSelected(bool bBroadcast) override;
  virtual void NativeOnDeselected(bool bBroadcast) override;
  virtual void NativeOnEnabled() override;
  virtual void NativeOnDisabled() override;
  virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
  virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;

private:
  FDelegateHandle ThemeHandle;
  bool bTicked = false;
};
