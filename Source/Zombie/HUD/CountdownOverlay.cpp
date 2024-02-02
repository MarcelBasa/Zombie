#include "CountdownOverlay.h"
#include "Components/TextBlock.h"
#include "Zombie/GameStates/ZombieGameState.h"


void UCountdownOverlay::NativeConstruct()
{
	Super::NativeConstruct();

}

bool UCountdownOverlay::Initialize()
{
	bool Success = Super::Initialize();

	if (!Success) return false;

	if (Time)
	{
		Time->TextDelegate.BindUFunction(this, "SetTimeText");
	}

	return Success;
}

FText UCountdownOverlay::SetTimeText()
{
	AZombieGameState* GameState = Cast<AZombieGameState>(GetWorld()->GetGameState());
	if (GameState)
	{
		FText TimeText = FText::FromString(FString::Printf(TEXT("%02d:%02d"), GameState->GetTime() / 60, GameState->GetTime() % 60));
		return TimeText;
	}
	return FText();
}
