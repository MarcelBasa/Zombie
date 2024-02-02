#include "ZombieCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Zombie/Characters/HumanCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Zombie/ZombieGameMode.h"
#include "Zombie/PlayerControllers/HumanPlayerController.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"


AZombieCharacter::AZombieCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->JumpZVelocity = 750.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = SpeedNormal;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	AttackRangeBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Attack Range Box"));
	AttackRangeBox->SetupAttachment(RootComponent);
}

void AZombieCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		OnTakePointDamage.AddDynamic(this, &AZombieCharacter::OnPointDamage);
		if (AttackRangeBox)
		{
			AttackRangeBox->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnAttackRangeBoxOverlap);
			AttackRangeBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
	PlayRoarMontage();
}

void AZombieCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AZombieCharacter, Health);
	DOREPLIFETIME(AZombieCharacter, MaxHealth);
	DOREPLIFETIME(AZombieCharacter, Speed);
}

void AZombieCharacter::OnPointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser)
{
	if (BoneName == "head")
		Damage *= 2;

	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHealthHUD(Health, MaxHealth);
	PlayReactMontage();
	StartSlow(SlowTime);

	if (Health <= 0.f)
		ServerDeath();
}

void AZombieCharacter::ServerDeath_Implementation()
{
	MulticastDeath();
}

void AZombieCharacter::MulticastDeath_Implementation()
{
	PlayerController = PlayerController == nullptr ? Cast<AHumanPlayerController>(GetController()) : PlayerController;
	if (PlayerController)
		DisableInput(PlayerController);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetSimulatePhysics(true);
	AZombieGameMode* GameMode = Cast<AZombieGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode)
		GameMode->ZombieDeath(this, Controller);
}

void AZombieCharacter::OnRep_Health(float LastHealth)
{
	if(Health < LastHealth)
		PlayReactMontage();
	UpdateHealthHUD(Health, MaxHealth);
}

void AZombieCharacter::StartSlow(float TimeToSlow)
{
	Speed = SpeedSlow;
	GetCharacterMovement()->MaxWalkSpeed = Speed;
	GetWorldTimerManager().SetTimer(
		SlowTimer,
		this,
		&ThisClass::FinishSlow,
		TimeToSlow,
		false
	);
}

void AZombieCharacter::FinishSlow()
{
	Speed = SpeedNormal;
	GetCharacterMovement()->MaxWalkSpeed = Speed;
}

void AZombieCharacter::OnRep_Speed()
{
	GetCharacterMovement()->MaxWalkSpeed = Speed;
}

void AZombieCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AZombieCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ThisClass::Jump);
	PlayerInputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &ThisClass::AttackButtonPressed);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AZombieCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AZombieCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AZombieCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AZombieCharacter::LookUp);
}

void AZombieCharacter::MoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}
void AZombieCharacter::MoveRight(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}
void AZombieCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AZombieCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void AZombieCharacter::PlayReactMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReactMontage)
	{
		AnimInstance->Montage_Play(ReactMontage);
	}
}

void AZombieCharacter::PlayRoarMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && RoarMontage)
	{
		AnimInstance->Montage_Play(RoarMontage);
	}
}

void AZombieCharacter::PlayAttackMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		AnimInstance->Montage_Play(AttackMontage);
		if (bRightHandAttack)
			AnimInstance->Montage_JumpToSection(FName("AttackRight"), AttackMontage);
		else
			AnimInstance->Montage_JumpToSection(FName("AttackLeft"), AttackMontage);
		bRightHandAttack = !bRightHandAttack;
	}
}

void AZombieCharacter::AttackButtonPressed()
{
	if (bCanAttack)
	{
		bCanAttack = false;
		ServerAttack();
	}
}

void AZombieCharacter::AttackFinish()
{
	bCanAttack = true;
	HittedActors.Empty();
	if (AttackRangeBox)
		AttackRangeBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AZombieCharacter::ServerAttack_Implementation()
{
	MulticastAttack();
}

void AZombieCharacter::MulticastAttack_Implementation()
{
	if (AttackRangeBox)
		AttackRangeBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetWorldTimerManager().SetTimer(
		AttackTimer,
		this,
		&ThisClass::AttackFinish,
		0.6f,
		false
	);
	PlayAttackMontage();
}

void AZombieCharacter::OnAttackRangeBoxOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromWeep, const FHitResult& SweepResult)
{
	if(OtherActor && HittedActors.Contains(OtherActor) == false)
	{
		AHumanCharacter* HittedHuman = Cast<AHumanCharacter>(OtherActor);
		if (HittedHuman)
		{
			HittedActors.Add(OtherActor);
			UGameplayStatics::ApplyDamage(
				HittedHuman,
				AttackDamage,
				Controller,
				this,
				UDamageType::StaticClass()
			);
		}
	}
}

void AZombieCharacter::UpdateHealthHUD(float NewHealth, float NewMaxHealth)
{
	PlayerController = PlayerController == nullptr ? Cast<AHumanPlayerController>(GetController()) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHealthHUD(NewHealth, NewMaxHealth);
	}
}

void AZombieCharacter::SetMaxHealth(float NewMaxHealth)
{
	if (NewMaxHealth > 0.f)
		Health = MaxHealth = NewMaxHealth;
	else
		Health = MaxHealth = 1000.f;

	UpdateHealthHUD(Health, MaxHealth);
}
