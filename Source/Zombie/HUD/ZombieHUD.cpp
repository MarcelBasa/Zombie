#include "ZombieHUD.h"
#include "ZombieOverlay.h"


void AZombieHUD::BeginPlay()
{
	Super::BeginPlay();

	AddZombieOverlay();
}

void AZombieHUD::AddZombieOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();

	if (PlayerController && OverlayClass)
	{
		ZombieOverlay = CreateWidget<UZombieOverlay>(PlayerController, OverlayClass);
		ZombieOverlay->AddToViewport();
	}
}

void AZombieHUD::Destroyed()
{
	Super::Destroyed();

	if (ZombieOverlay)
		ZombieOverlay->RemoveFromParent();
}