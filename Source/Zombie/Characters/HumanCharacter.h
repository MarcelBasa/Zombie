#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HumanCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UCombatComponent;
class AWeapon;
class UAnimMontage;
class AZombieCharacter;
class UWinOverlay;

UCLASS()
class ZOMBIE_API AHumanCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

public:
	AHumanCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Destroyed() override;
	AWeapon* GetWeapon();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon> StartingWeaponClass;

	void SetOverlappingWeapon(AWeapon* Weapon);
	void SetMaxHealth(float NewMaxHealth);
	void PlayFireMontage();
	void PlayEquipMontage();
	void PlayReloadMontage();

	UFUNCTION(Server, Unreliable)
	void ServerUpdateTurn(bool bLeft, bool bRight);

protected:
	virtual void BeginPlay();
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCombatComponent* Combat;

	UPROPERTY(Replicated)
	AWeapon* OverlappingWeapon;

	bool bFabrik = true;
	UPROPERTY(Replicated)
	bool bTurnLeft = false;
	UPROPERTY(Replicated)
	bool bTurnRight = false;

	UPROPERTY(Replicated)
	float MaxHealth = 100.f;
	UPROPERTY(ReplicatedUsing = OnRep_Health)
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health(float LastHealth);

	UPROPERTY(EditAnywhere)
	UAnimMontage* FireMontage;
	UPROPERTY(EditAnywhere)
	UAnimMontage* EquipMontage; 
	UPROPERTY(EditAnywhere)
	UAnimMontage* ReloadMontage;

	void EquipButtonPressed();
	void FireButtonPressed();
	void FireButtonReleased();
	void ReloadButtonPressed();

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

public:
	FORCEINLINE void SetFABRIK(bool newState) { bFabrik = newState; }
	FORCEINLINE bool GetFABRIK() const { return bFabrik; }
	FORCEINLINE bool GetTurnLeft() const { return bTurnLeft; }
	FORCEINLINE bool GetTurnRight() const { return bTurnRight; }
	FORCEINLINE bool IsAlive() const { return Health > 0.f ? true : false; }
};
