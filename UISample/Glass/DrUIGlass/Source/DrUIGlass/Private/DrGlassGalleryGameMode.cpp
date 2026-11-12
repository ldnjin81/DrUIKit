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
    Gallery->AddToViewport();
    FInputModeUIOnly Mode;
    Mode.SetWidgetToFocus(Gallery->TakeWidget());
    PC->SetInputMode(Mode);
    PC->SetShowMouseCursor(true);
  }
}
