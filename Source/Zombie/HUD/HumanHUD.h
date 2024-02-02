#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HumanHUD.generated.h"

class UTexture2D;
class UHumanOverlay;

UCLASS()
class ZOMBIE_API AHumanHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	virtual void DrawHUD() override;
	void AddHumanOverlay();
	UHumanOverlay* HumanOverlay;
	void CreateWinOverlay(FString TextToDisplay);

protected:
	virtual void BeginPlay();
	virtual void Destroyed();

private:
	UPROPERTY(EditAnywhere)
	UTexture2D* CrosshairsTop;
	UPROPERTY(EditAnywhere)
	UTexture2D* CrosshairsBottom;
	UPROPERTY(EditAnywhere)
	UTexture2D* CrosshairsLeft;
	UPROPERTY(EditAnywhere)
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UHumanOverlay> OverlayClass;

	void DrawCrosshair();
};
