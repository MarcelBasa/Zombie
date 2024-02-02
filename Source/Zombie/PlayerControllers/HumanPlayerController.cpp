#include "HumanPlayerController.h"
#include "Zombie/HUD/HumanHUD.h"
#include "Zombie/HUD/ZombieHUD.h"
#include "Zombie/HUD/HumanOverlay.h"
#include "Zombie/HUD/ZombieOverlay.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Zombie/ZombieGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Zombie/HUD/WinOverlay.h"
#include "Zombie/HUD/CountdownOverlay.h"


void AHumanPlayerController::BeginPlay()
{
	Super::BeginPlay();

	HumanHUD = Cast<AHumanHUD>(GetHUD());

	ClientCreateTimerOverlay();
}

void AHumanPlayerController::ClientCreateTimerOverlay_Implementation()
{
	if (CountdownOverlayClass && IsLocalPlayerController())
	{
		CountdownOverlay = CreateWidget<UCountdownOverlay>(this, CountdownOverlayClass);
		if (CountdownOverlay)
			CountdownOverlay->AddToViewport();
	}
}

void AHumanPlayerController::SetWinOverlay(const FString& TextToDisplay)
{
	ServerCreateWinOverlay(TextToDisplay);
}

void AHumanPlayerController::ServerCreateWinOverlay_Implementation(const FString& TextToDisplay)
{
	MulticastCreateWinOverlay(TextToDisplay);
}

void AHumanPlayerController::MulticastCreateWinOverlay_Implementation(const FString& TextToDisplay)
{
	if (WinOverlayClass && IsLocalPlayerController())
	{
		if (HumanHUD)
			HumanHUD->Destroy();
		WinOverlay = CreateWidget<UWinOverlay>(this, WinOverlayClass);
		if (WinOverlay)
		{
			WinOverlay->AddToViewport();
			WinOverlay->WinText->SetText(FText::FromString(TextToDisplay));
		}
	}
}

void AHumanPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHumanPlayerController::SetAmmoHUD(int32 Ammo)
{
	if (HumanHUD &&
		HumanHUD->HumanOverlay &&
		HumanHUD->HumanOverlay->Ammo)
	{
		HumanHUD->HumanOverlay->Ammo->SetText(FText::FromString(FString::FromInt(Ammo)));
	}
}

void AHumanPlayerController::SetMagazineHUD(int32 Magazine)
{
	if (HumanHUD &&
		HumanHUD->HumanOverlay &&
		HumanHUD->HumanOverlay->Magazine)
	{
		HumanHUD->HumanOverlay->Magazine->SetText(FText::FromString(FString::FromInt(Magazine)));
	}
}

void AHumanPlayerController::SetHealthHUD(float Health, float MaxHealth)
{
	ZombieHUD = ZombieHUD == nullptr ? Cast<AZombieHUD>(GetHUD()) : ZombieHUD;
	if (ZombieHUD &&
		ZombieHUD->ZombieOverlay &&
		ZombieHUD->ZombieOverlay->HealthBar &&
		ZombieHUD->ZombieOverlay->Health
		)
	{
		const float HealthPercent = Health / MaxHealth;
		ZombieHUD->ZombieOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		ZombieHUD->ZombieOverlay->Health->SetText(FText::FromString(HealthText));
	}
}
