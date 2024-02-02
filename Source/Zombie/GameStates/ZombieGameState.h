#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ZombieGameState.generated.h"

class AZombieGameMode;

UCLASS()
class ZOMBIE_API AZombieGameState : public AGameState
{
	GENERATED_BODY()


public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UFUNCTION(BlueprintCallable)
	FORCEINLINE int32 GetTime() const { return Time; }

protected:
	virtual void BeginPlay() override;
	UFUNCTION()
	void OnRep_Counting();

private:
	AZombieGameMode* ZombieGameMode;

	UPROPERTY(EditAnywhere)
		int32 StartingTime = 5;
	UPROPERTY(EditAnywhere)
		int32 GameTime = 120;

	FTimerHandle StartTimer;
	FTimerHandle GameTimer;
	FTimerHandle RestartTimer;

	void StartTimeCounting();
	void GameTimeCounting();

	UPROPERTY(ReplicatedUsing = OnRep_Counting)
	int32 ServerTime;
	int32 Time;

};
