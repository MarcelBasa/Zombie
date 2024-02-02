#include "ZombieGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Zombie/Characters/HumanCharacter.h"
#include "Zombie/Characters/ZombieCharacter.h"
#include "Zombie/PlayerControllers/HumanPlayerController.h"
#include "Zombie/HUD/HumanHUD.h"
#include "Zombie/HUD/ZombieHUD.h"
#include "GameFramework/PlayerStart.h"
#include "Zombie/Components/CombatComponent.h"
#include "Zombie/GameStates/ZombieGameState.h"


AZombieGameMode::AZombieGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AZombieGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AZombieGameMode::BeginPlay()
{
	Super::BeginPlay();

}


void AZombieGameMode::HandleGameStart()
{
	UE_LOG(LogTemp, Warning, TEXT("START!"));
	InfectPlayer(GetRandomPlayer(), false);
}

void AZombieGameMode::HandleGameEnd()
{
	UE_LOG(LogTemp, Warning, TEXT("END!"));
	IsHumanWon(true);
}

AHumanCharacter* AZombieGameMode::GetRandomPlayer()
{
	TArray<AActor*> HumanPlayers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHumanCharacter::StaticClass(), HumanPlayers);
	AHumanCharacter* HumanPlayerToInfect = Cast<AHumanCharacter>(HumanPlayers[FMath::RandRange(0, HumanPlayers.Num() - 1)]);
	return HumanPlayerToInfect;
}

void AZombieGameMode::InfectPlayer(AHumanCharacter* Player, bool bCheckGameState)
{
	if (ZombieHUDClass == nullptr || ZombieCharacterClass == nullptr || Player == nullptr || GetWorldTimerManager().IsTimerActive(RestartTimer)) return;

	APlayerController* PlayerController = Cast<APlayerController>(Player->Controller);
	if (PlayerController)
	{
		PlayerController->ClientSetHUD(ZombieHUDClass);
		AZombieCharacter* Zombie = GetWorld()->SpawnActor<AZombieCharacter>(ZombieCharacterClass, Player->GetActorLocation(), Player->GetActorRotation());
		if (Zombie)
		{
			PlayerController->Possess(Zombie);
			Zombie->SetMaxHealth(ZombieHealth);
		}
	}
	Player->Destroy();
	if(bCheckGameState)
		CheckZombieWon();
}

void AZombieGameMode::ZombieDeath(AZombieCharacter* ZombieCharacter, AController* Controller)
{
	if (ZombieCharacter == nullptr || Controller == nullptr) return;

	FTimerHandle ZombieRespawnTimer;
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &ThisClass::RequestRespawnZombie, ZombieCharacter, Controller);
	GetWorldTimerManager().SetTimer(
		ZombieRespawnTimer,
		TimerDelegate,
		ZombieRespawnTime,
		false
	);
}

void AZombieGameMode::RequestRespawnZombie(AZombieCharacter* ZombieCharacter, AController* Controller)
{
	if (ZombieCharacter == nullptr || Controller == nullptr || ZombieCharacterClass == nullptr) return;

	ZombieCharacter->Destroy();
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
	int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
	AZombieCharacter* Zombie = GetWorld()->SpawnActor<AZombieCharacter>(ZombieCharacterClass, PlayerStarts[Selection]->GetActorLocation(), PlayerStarts[Selection]->GetActorRotation());
	if (Zombie)
	{
		ZombieCharacter->Restart();
		Controller->Possess(Zombie);
		Zombie->SetMaxHealth(ZombieHealth);
	}
}

void AZombieGameMode::RequestRespawnHuman(ACharacter* HumanCharacter, AController* HumanController)
{
	if (HumanCharacter == nullptr || HumanController == nullptr || HumanHUDClass == nullptr) return;

	HumanCharacter->Restart();
	HumanCharacter->Destroy();
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
	int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
	RestartPlayerAtPlayerStart(HumanController, PlayerStarts[Selection]);
	APlayerController* PlayerController = Cast<APlayerController>(HumanController);
	if (PlayerController)
		PlayerController->ClientSetHUD(HumanHUDClass);
}

void AZombieGameMode::RestartPlayers()
{
	TArray<AActor*> Players;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), Players);
	for (int i = 0; i < Players.Num(); i++)
	{
		ACharacter* PlayerToRespawn = Cast<ACharacter>(Players[i]);
		RequestRespawnHuman(PlayerToRespawn, PlayerToRespawn->Controller);
	}
}

void AZombieGameMode::CheckZombieWon()
{
	TArray<AActor*> HumanPlayers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHumanCharacter::StaticClass(), HumanPlayers);
	if (HumanPlayers.Num() == 0)
	{
		IsHumanWon(false);
	}
}

void AZombieGameMode::IsHumanWon(bool bIsHumanWon)
{
	if (GetWorldTimerManager().IsTimerActive(RestartTimer)) return;

	DisplayWinOverlayForAll(bIsHumanWon);
	GetWorldTimerManager().SetTimer(
		RestartTimer,
		this,
		&AZombieGameMode::RestartGame,
		RestartTime,
		false
	);
}

void AZombieGameMode::DisplayWinOverlayForAll(bool bIsHumanWon)
{
	TArray<AActor*> Players;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), Players);
	
	for (int i = 0; i < Players.Num(); i++)
	{
		AHumanPlayerController* HumanController = Cast<AHumanPlayerController>(Cast<ACharacter>(Players[i])->Controller);
		if (HumanController)
		{
			if(bIsHumanWon)
				HumanController->SetWinOverlay("Human Won");
			else
				HumanController->SetWinOverlay("Zombie Won");
		}
	}
}

void AZombieGameMode::RestartGame()
{
	GetWorld()->ServerTravel(FString("/Game/Maps/GameMap?listen"));
}
