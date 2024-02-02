#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class AHumanCharacter;
class AWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZOMBIE_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class AHumanCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void FireButtonPressed(bool bPressed);
	void ReloadButtonPressed();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_EquippedWeapon();
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);
	UFUNCTION(Server, Reliable)
	void ServerReload();
	UPROPERTY(ReplicatedUsing = OnRep_IsReloading)
	bool bReload;
	bool bIsReloading;
	UFUNCTION()
	void OnRep_IsReloading();

private:
	UPROPERTY()
	AHumanCharacter* Character;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* Weapon;

	void EquipStartWeapon();
	void EquipWeapon(AWeapon* WeaponToEquip);
	void StartAttachActorToRightHandTimer(AWeapon* WeaponToAttach);
	void AttachActorToRightHand(AWeapon* WeaponToAttach);
	
	void StartReloadTimer(UStaticMeshComponent* Mesh);
	void FinishReloadTimer();
	void AttachActorToLeftHand();

	bool bFireButtonPressed = false;
	bool bCanFire = true;
	FTimerHandle FireTimer;
	FTimerHandle EquipTimer;
	FTimerHandle ReloadTimer;

	void Fire();
	void StartFireTimer();
	void FireTimerFinished();
	void Trace(FHitResult& HitResultCamera);
	void Recoil();

	void UpdateAmmoHUD();
	void UpdateMagazineHUD();
};
