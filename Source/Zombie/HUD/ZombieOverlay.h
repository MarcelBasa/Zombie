#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZombieOverlay.generated.h"

class UTextBlock;
class UProgressBar;

UCLASS()
class ZOMBIE_API UZombieOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Health;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;
};
