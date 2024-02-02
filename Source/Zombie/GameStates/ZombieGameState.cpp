#include "ZombieGameState.h"
#include "Net/UnrealNetwork.h"
#include "Zombie/ZombieGameMode.h"


void AZombieGameState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(
			StartTimer,
			this,
			&AZombieGameState::StartTimeCounting,
			1.f,
			true
		);
	}
}

void AZombieGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AZombieGameState, ServerTime);
}

void AZombieGameState::OnRep_Counting()
{
	Time = ServerTime;
}

void AZombieGameState::StartTimeCounting()
{
	if (StartingTime <= 0)
	{
		GetWorldTimerManager().ClearTimer(StartTimer);
		ZombieGameMode = Cast<AZombieGameMode>(GetWorld()->GetAuthGameMode());
		if(ZombieGameMode) 
			ZombieGameMode->HandleGameStart();
		GetWorldTimerManager().SetTimer(
			GameTimer,
			this,
			&AZombieGameState::GameTimeCounting,
			1.f,
			true
		);
	}
	ServerTime = Time = StartingTime--;
}

void AZombieGameState::GameTimeCounting()
{
	if (GameTime <= 0)
	{
		GetWorldTimerManager().ClearTimer(GameTimer);
		ZombieGameMode = Cast<AZombieGameMode>(GetWorld()->GetAuthGameMode());
		if (ZombieGameMode)
			ZombieGameMode->IsHumanWon(true);
	}
	ServerTime = Time = GameTime--;
}