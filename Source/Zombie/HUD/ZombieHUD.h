#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ZombieHUD.generated.h"

class UZombieOverlay;

UCLASS()
class ZOMBIE_API AZombieHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	void AddZombieOverlay();
	UZombieOverlay* ZombieOverlay;

protected:
	virtual void BeginPlay();
	virtual void Destroyed();

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UZombieOverlay> OverlayClass;
};
