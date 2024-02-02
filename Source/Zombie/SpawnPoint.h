#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnPoint.generated.h"

class AWeapon;

UCLASS()
class ZOMBIE_API ASpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:
	void WeaponTaked();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	AWeapon* Weapon;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AWeapon>> WeaponsClasses;
	FTimerHandle SpawnWeaponTimer;
	float SpawnWeaponTime = 10.f;

	void SpawnWeapon();


};
