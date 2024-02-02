#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class USphereComponent;
class UNiagaraSystem;
class UParticleSystem;
class AHumanCharacter;
class AHumanPlayerController;
class ASpawnPoint;

UCLASS()
class ZOMBIE_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Fire(const FVector& HitTarget);
	void Reload();
	void SetVisibility(bool visibility);
	void UpdateAmmoHUD();
	void UpdateMagazineHUD();

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float FireDelay = 0.3f;
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float Damage = 10.f;
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	bool bAutomatic = false;
	UPROPERTY(ReplicatedUsing = OnRep_Magazine, EditAnywhere, Category = "Weapon Properties")
	int32 Magazine = 5;
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float RecoilPitch = 0.3f;
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float RecoilYaw = 0.3f;
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	FVector MagazineSize = FVector(1,1,1);
	UPROPERTY(ReplicatedUsing = OnRep_Ammo)
	int32 Ammo;

protected:
	virtual void BeginPlay() override;
	void Shot(const FVector& HitTarget);

	UFUNCTION()
	void OnSphereOverlap(
		UPrimitiveComponent* OverlapComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromWeep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlapComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	UStaticMeshComponent* MagazineMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USphereComponent* AreaSphere;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	FName WeaponSocketName = TEXT("Default_Weapon_Socket");

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	FName MagazineBoneName = TEXT("joint2");

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	UNiagaraSystem* FireEffectMuzzle;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	UNiagaraSystem* FireEffectImpact;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	UNiagaraSystem* FireBodyEffectImpact;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	int32 MaxAmmo = 30; 

	AHumanCharacter* Character;
	AHumanPlayerController* CharacterController;
	ASpawnPoint* SpawnPoint;

	UFUNCTION()
	void OnRep_Ammo();
	UFUNCTION()
	void OnRep_Magazine();

public:
	FORCEINLINE FName GetWeaponSocketName() const { return WeaponSocketName; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE UStaticMeshComponent* GetMagazineMesh() const { return MagazineMesh; }
	FORCEINLINE bool IsEmpty() const { return Ammo <= 0 ? true : false; }
	FORCEINLINE int32 GetMagazineCount() const { return Magazine; }
	FORCEINLINE void SetSpawnPoint(ASpawnPoint* NewSpawnPoint) { SpawnPoint = NewSpawnPoint; }
	FORCEINLINE ASpawnPoint* GetSpawnPoint() const  { return SpawnPoint; }
};
