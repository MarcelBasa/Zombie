#include "HumanCharacter.h"
#include "Zombie/Components/CombatComponent.h"
#include "Zombie/Weapons/Weapon.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Zombie/ZombieGameMode.h"
#include "Kismet/GameplayStatics.h"


AHumanCharacter::AHumanCharacter()
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->JumpZVelocity = 450.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true; 

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); 
	FollowCamera->bUsePawnControlRotation = false; 
	
	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);
}

void AHumanCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Combat)
	{
		Combat->Character = this;
	}
}

void AHumanCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AHumanCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(AHumanCharacter, MaxHealth);
	DOREPLIFETIME(AHumanCharacter, Health);
	DOREPLIFETIME(AHumanCharacter, bTurnLeft);
	DOREPLIFETIME(AHumanCharacter, bTurnRight);
}

void AHumanCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AHumanCharacter::ReceiveDamage);
	}
}

void AHumanCharacter::SetMaxHealth(float NewMaxHealth)
{
	if (NewMaxHealth > 0.f)
		Health = MaxHealth = NewMaxHealth;
	else
		Health = MaxHealth = 100.f;
}

void AHumanCharacter::OnRep_Health(float LastHealth)
{
	// Update HUD
}

void AHumanCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);

	if (Health <= 0.f)
	{
		AZombieGameMode* GameMode = Cast<AZombieGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
		if (GameMode)
			GameMode->InfectPlayer(this);
	}
}

void AHumanCharacter::Destroyed()
{
	Super::Destroyed();

	if (Combat && Combat->Weapon)
		Combat->Weapon->Destroy();
}

void AHumanCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

AWeapon* AHumanCharacter::GetWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->Weapon;
}

void AHumanCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ThisClass::Jump);
	PlayerInputComponent->BindAction(TEXT("Equip"), IE_Pressed, this, &AHumanCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &AHumanCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Fire"), IE_Released, this, &AHumanCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction(TEXT("Reload"), IE_Pressed, this, &AHumanCharacter::ReloadButtonPressed);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AHumanCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AHumanCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AHumanCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AHumanCharacter::LookUp);
}

void AHumanCharacter::MoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}
void AHumanCharacter::MoveRight(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void AHumanCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);

	if (GetVelocity().Size() != 0.f)
	{
		return;
	}

	if (Value > 0.1f)
	{
		bTurnRight = true; 
		ServerUpdateTurn(bTurnLeft, bTurnRight);
	}
	else if (Value < -0.1f)
	{
		bTurnLeft = true;
		ServerUpdateTurn(bTurnLeft, bTurnRight);
	}	
	else if (bTurnLeft == false && bTurnRight == false)
	{
		return;
	}
	else 
	{
		bTurnLeft = false;
		bTurnRight = false;
		ServerUpdateTurn(bTurnLeft, bTurnRight);
	}
}

void AHumanCharacter::ServerUpdateTurn_Implementation(bool bLeft, bool bRight)
{
	bTurnLeft = bLeft;
	bTurnRight = bRight;
}

void AHumanCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}
void AHumanCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	OverlappingWeapon = Weapon;
}
void AHumanCharacter::EquipButtonPressed()
{
	if (Combat == nullptr || OverlappingWeapon == nullptr) return;

	if (HasAuthority())
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
	else
	{
		ServerEquipButtonPressed();
	}
}
void AHumanCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
		Combat->EquipWeapon(OverlappingWeapon);
}
void AHumanCharacter::FireButtonPressed()
{
	if (Combat)
		Combat->FireButtonPressed(true);
}
void AHumanCharacter::FireButtonReleased()
{
	if (Combat)
		Combat->FireButtonPressed(false);
}
void AHumanCharacter::ReloadButtonPressed()
{
	if (Combat)
		Combat->ReloadButtonPressed();
}

void AHumanCharacter::PlayFireMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireMontage) 
	{
		AnimInstance->Montage_Play(FireMontage);
	}
}

void AHumanCharacter::PlayEquipMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipMontage) 
	{
		AnimInstance->Montage_Play(EquipMontage);
	}
}

void AHumanCharacter::PlayReloadMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
	}
}
