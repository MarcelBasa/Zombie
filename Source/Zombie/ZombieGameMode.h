#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ZombieGameMode.generated.h"

class AHumanCharacter;
class AZombieCharacter;
class AZombieHUD;
class AHumanHUD;

UCLASS(minimalapi)
class AZombieGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AZombieGameMode();
	virtual void Tick(float DeltaTime) override;
	void InfectPlayer(AHumanCharacter* Player, bool bCheckGameState = true);
	void ZombieDeath(AZombieCharacter* ZombieCharacter, AController* Controller);
	void RequestRespawnHuman(ACharacter* HumanCharacter, AController* HumanController);
	void RequestRespawnZombie(AZombieCharacter* ZombieCharacter, AController* Controller);
	void HandleGameStart();
	void HandleGameEnd();
	void IsHumanWon(bool bIsHumanWon);

protected:
	virtual void BeginPlay() override;

private:
	AHumanCharacter* GetRandomPlayer();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AZombieCharacter> ZombieCharacterClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AZombieHUD> ZombieHUDClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AHumanCharacter> HumanCharacterClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AHumanHUD> HumanHUDClass;

	UPROPERTY(EditAnywhere)
		float ZombieHealth = 1000.f;
	UPROPERTY(EditAnywhere)
		int32 ZombieRespawnTime = 5;
	UPROPERTY(EditAnywhere)
		int32 RestartTime = 10;

	FTimerHandle RestartTimer;

	void RestartPlayers();
	void CheckZombieWon(); 
	void RestartGame();
	void DisplayWinOverlayForAll(bool bIsHumanWon);
};



