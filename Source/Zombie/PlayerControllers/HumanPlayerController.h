#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HumanPlayerController.generated.h"

class AZombieHUD;
class AHumanHUD;
class AZombieGameMode;
class UWinOverlay;
class UCountdownOverlay;

UCLASS()
class ZOMBIE_API AHumanPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

public:
	void ChangeToZombieHUD();
	void ChangeToHumanHUD();
	void SetAmmoHUD(int32 Ammo);
	void SetMagazineHUD(int32 Magazine);
	void SetHealthHUD(float Health, float MaxHealth);
	void SetWinOverlay(const FString& TextToDisplay);
	UFUNCTION(Server, Reliable)
	void ServerCreateWinOverlay(const FString& TextToDisplay);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastCreateWinOverlay(const FString& TextToDisplay);

	UFUNCTION(Client, Reliable)
	void ClientCreateTimerOverlay();

private:
	AHumanHUD* HumanHUD;
	AZombieHUD* ZombieHUD;
	AZombieGameMode* ZombieGameMode;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UWinOverlay> WinOverlayClass;
	UWinOverlay* WinOverlay;
	UPROPERTY(EditAnywhere)
	TSubclassOf<UCountdownOverlay> CountdownOverlayClass;
	UCountdownOverlay* CountdownOverlay;
};
