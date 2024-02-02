#include "HumanHUD.h"
#include "HumanOverlay.h"

void AHumanHUD::BeginPlay()
{
	Super::BeginPlay();

	AddHumanOverlay();
}

void AHumanHUD::DrawHUD()
{
	Super::DrawHUD();

	DrawCrosshair();
}

void AHumanHUD::DrawCrosshair()
{
	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		if (CrosshairsTop)
			DrawTexture(CrosshairsTop, ViewportCenter.X - CrosshairsTop->GetSizeX()/2, ViewportCenter.Y - CrosshairsTop->GetSizeY()/2, CrosshairsTop->GetSizeX(), CrosshairsTop->GetSizeY(), 0, 0, 1, 1, FLinearColor::Green);
		if (CrosshairsBottom)
			DrawTexture(CrosshairsBottom, ViewportCenter.X - CrosshairsBottom->GetSizeX()/2, ViewportCenter.Y - CrosshairsBottom->GetSizeY()/2, CrosshairsBottom->GetSizeX(), CrosshairsBottom->GetSizeY(), 0, 0, 1, 1, FLinearColor::Green);
		if (CrosshairsLeft)
			DrawTexture(CrosshairsLeft, ViewportCenter.X - CrosshairsLeft->GetSizeX()/2, ViewportCenter.Y - CrosshairsLeft->GetSizeY()/2, CrosshairsLeft->GetSizeX(), CrosshairsLeft->GetSizeY(), 0, 0, 1, 1, FLinearColor::Green);
		if (CrosshairsRight)
			DrawTexture(CrosshairsRight, ViewportCenter.X - CrosshairsRight->GetSizeX()/2, ViewportCenter.Y - CrosshairsRight->GetSizeY()/2, CrosshairsRight->GetSizeX(), CrosshairsRight->GetSizeY(), 0, 0, 1, 1, FLinearColor::Green);
	}
}

void AHumanHUD::AddHumanOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();

	if (PlayerController && OverlayClass)
	{
		HumanOverlay = CreateWidget<UHumanOverlay>(PlayerController, OverlayClass);
		HumanOverlay->AddToViewport();
	}
}

void AHumanHUD::Destroyed()
{
	if(HumanOverlay)
		HumanOverlay->RemoveFromParent();

	Super::Destroyed();
}
