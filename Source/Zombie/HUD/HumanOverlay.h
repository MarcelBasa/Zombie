#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HumanOverlay.generated.h"

class UTextBlock;

UCLASS()
class ZOMBIE_API UHumanOverlay : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Ammo;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Magazine;
};
