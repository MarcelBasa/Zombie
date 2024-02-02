#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CountdownOverlay.generated.h"

class UTextBlock;

UCLASS()
class ZOMBIE_API UCountdownOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	bool Initialize() override;

private:
	UPROPERTY(meta = (BindWidget))
		UTextBlock* Time;

	UFUNCTION()
		FText SetTimeText();
};
