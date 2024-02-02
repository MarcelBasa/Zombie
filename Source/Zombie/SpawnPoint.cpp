#include "SpawnPoint.h"
#include "Zombie/Weapons/Weapon.h"
#include "Kismet/KismetMathLibrary.h"


void ASpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	SpawnWeapon();
}


void ASpawnPoint::SpawnWeapon()
{
	if (HasAuthority())
	{
		int32 index = UKismetMathLibrary::RandomIntegerInRange(0, WeaponsClasses.Num() - 1);
		if (WeaponsClasses[index])
			Weapon = GetWorld()->SpawnActor<AWeapon>(WeaponsClasses[index], GetActorLocation(), FRotator(0.f));
		if (Weapon)
			Weapon->SetSpawnPoint(this);
	}
}

void ASpawnPoint::WeaponTaked()
{
	if (Weapon)
	{
		Weapon->SetSpawnPoint(nullptr);
		Weapon = nullptr;
	}	

	GetWorldTimerManager().SetTimer(
		SpawnWeaponTimer,
		this,
		&ThisClass::SpawnWeapon,
		SpawnWeaponTime
	);
}