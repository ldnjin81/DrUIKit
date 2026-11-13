#include "DrGlassGalleryGameMode.h"
#include "Blueprint/UserWidget.h"
#include "DrGlassGallery.h"
#include "GameFramework/PlayerController.h"

ADrGlassGalleryGameMode::ADrGlassGalleryGameMode() { DefaultPawnClass = nullptr; }

void ADrGlassGalleryGameMode::BeginPlay() {
  Super::BeginPlay();
  APlayerController* PC = GetWorld()->GetFirstPlayerController();
  if (!PC) return;
  if (UDrGlassGallery* Gallery = CreateWidget<UDrGlassGallery>(PC, UDrGlassGallery::StaticClass())) {
    // 플레이어 화면에 붙인다(UI 흐름의 화면 층도 여기에 1000으로 붙어 대화상자·메뉴가 갤러리 위에 뜬다. AddToViewport는 플레이어 화면보다 위에 그려진다).
    Gallery->AddToPlayerScreen(0);
    FInputModeUIOnly Mode;
    Mode.SetWidgetToFocus(Gallery->TakeWidget());
    PC->SetInputMode(Mode);
    PC->SetShowMouseCursor(true);
  }
}
