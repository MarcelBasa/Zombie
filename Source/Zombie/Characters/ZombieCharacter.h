#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZombieCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UAnimMontage;
class AHumanPlayerController;
class UBoxComponent;

UCLASS()
class ZOMBIE_API AZombieCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

public:
	AZombieCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UFUNCTION(Server, Reliable)
	void ServerAttack();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastAttack();
	void SetMaxHealth(float NewMaxHealth);
	UFUNCTION(Server, Reliable)
	void ServerDeath();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastDeath();

protected:
	virtual void BeginPlay() override;
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);

	UFUNCTION()
	void OnPointDamage(
		AActor* DamagedActor, 
		float Damage, 
		AController* InstigatedBy, 
		FVector HitLocation, 
		UPrimitiveComponent* FHitComponent, 
		FName BoneName, 
		FVector ShotFromDirection, 
		const UDamageType* DamageType, 
		AActor* DamageCauser
	);
	
	UFUNCTION()
	void OnAttackRangeBoxOverlap(
		UPrimitiveComponent* OverlapComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromWeep,
		const FHitResult& SweepResult
	);

private:	
	UPROPERTY(EditAnywhere)
	UAnimMontage* RoarMontage;
	UPROPERTY(EditAnywhere)
	UAnimMontage* ReactMontage;
	UPROPERTY(EditAnywhere)
	UAnimMontage* AttackMontage;

	UPROPERTY(Replicated)
	float MaxHealth;
	UPROPERTY(ReplicatedUsing = OnRep_Health)
	float Health;

	UFUNCTION()
	void OnRep_Health(float LastHealth);

	void PlayRoarMontage();
	void PlayReactMontage();
	void PlayAttackMontage();

	TArray<AActor*> HittedActors;
	UPROPERTY(EditAnywhere)
	UBoxComponent* AttackRangeBox;
	UPROPERTY(EditAnywhere)
	float AttackRange = 130.f;
	UPROPERTY(EditAnywhere)
	float AttackDamage = 100.f;
	bool bRightHandAttack = true;
	bool bCanAttack = true;
	FTimerHandle AttackTimer;

	void AttackButtonPressed();
	void AttackFinish();

	UPROPERTY(ReplicatedUsing = OnRep_Speed)
	float Speed = 600.f;
	UPROPERTY(EditAnywhere)
	float SpeedNormal = 600.f;
	UPROPERTY(EditAnywhere)
	float SpeedSlow = 300.f;
	UPROPERTY(EditAnywhere)
	float SlowTime = .2f;
	FTimerHandle SlowTimer;

	void StartSlow(float TimeToSlow);
	void FinishSlow();
	UFUNCTION()
	void OnRep_Speed();

	AHumanPlayerController* PlayerController;
	void UpdateHealthHUD(float NewHealth, float NewMaxHealth);

public:
	FORCEINLINE bool IsAlive() const { return Health > 0.f ? true : false; }
};
