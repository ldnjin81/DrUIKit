#pragma once
#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "Theme/DrUITheme.h"
#include "Theme/DrUIKitSettings.h"
#include "DrUIThemeSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FDrUIOnThemeChanged, const UDrUITheme&);

/** 현재 테마를 보관·전환한다. 엔진 서브시스템이라 에디터 디자이너 미리보기에서도 같은 값을 쓴다. */
UCLASS()
class DRUIKIT_API UDrUIThemeSubsystem : public UEngineSubsystem {
  GENERATED_BODY()
public:
  /** 엔진이 없거나(일부 테스트) 아직 만들어지지 않았으면 nullptr. */
  static UDrUIThemeSubsystem* Get();
  /** 서브시스템이 없어도 쓸 수 있는 현재 테마(없으면 기본 테마). */
  static const UDrUITheme& Current();

  virtual void Initialize(FSubsystemCollectionBase& Collection) override;
  const UDrUITheme& GetTheme() const;
  /** ThemeId로 전환. 없는 Id면 false. */
  bool SetTheme(FName ThemeId);
  /** 설정에 없는 테마(샘플·테스트가 만든 것)를 직접 넣고 바로 쓴다. */
  void SetThemeObject(UDrUITheme* Theme);
  /** 테마를 목록에만 더한다(같은 ThemeId는 교체). 지금 테마가 코어 기본이면 이것으로 바꾼다. */
  void AddTheme(UDrUITheme* Theme);
  TArray<FName> GetThemeIds() const;
  EDrUISurfaceQuality GetSurfaceQuality() const { return SurfaceQuality; }
  void SetSurfaceQuality(EDrUISurfaceQuality Quality);
  bool IsReduceMotion() const { return bReduceMotion; }
  void SetReduceMotion(bool bReduce) { bReduceMotion = bReduce; }

  /** 테마나 표면 품질이 바뀌면 알린다. DrUIKit 위젯은 이걸 받아 다시 칠한다. */
  FDrUIOnThemeChanged OnThemeChanged;
private:
  UPROPERTY(Transient) TArray<TObjectPtr<UDrUITheme>> Themes;
  UPROPERTY(Transient) TObjectPtr<UDrUITheme> Active;
  UPROPERTY(Transient) TObjectPtr<UDrUITheme> Fallback;
  EDrUISurfaceQuality SurfaceQuality = EDrUISurfaceQuality::High;
  bool bReduceMotion = false;
  void Broadcast();
};
