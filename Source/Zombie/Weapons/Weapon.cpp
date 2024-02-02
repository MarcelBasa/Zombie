#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "Zombie/Characters/HumanCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameViewportClient.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Zombie/Characters/ZombieCharacter.h"
#include "Zombie/PlayerControllers/HumanPlayerController.h"
#include "Engine/SkeletalMeshSocket.h"


AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MagazineMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MagazineMesh"));
	MagazineMesh->SetupAttachment(WeaponMesh, FName("Magazine"));
	MagazineMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	MagazineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MagazineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	WeaponMesh->HideBoneByName(MagazineBoneName, EPhysBodyOp::PBO_MAX);
	MagazineMesh->SetWorldScale3D(MagazineSize);
	Ammo = MaxAmmo;
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnSphereEndOverlap);	
	}
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, Ammo);
	DOREPLIFETIME(AWeapon, Magazine);
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromWeep, const FHitResult& SweepResult)
{
	AHumanCharacter* CharacterOverllap = Cast<AHumanCharacter>(OtherActor);
	if (CharacterOverllap)
		CharacterOverllap->SetOverlappingWeapon(this);
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AHumanCharacter* CharacterOverllap = Cast<AHumanCharacter>(OtherActor);
	if (CharacterOverllap)
		CharacterOverllap->SetOverlappingWeapon(nullptr);
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::Fire(const FVector& HitTarget)
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MaxAmmo);
	UpdateAmmoHUD();

	Shot(HitTarget);
}

void AWeapon::Shot(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("Muzzle");
	if (MuzzleFlashSocket == nullptr) return;

	FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	SocketTransform.SetScale3D(FVector(1.f, 1.f, 1.f));
	FVector Start = SocketTransform.GetLocation();
	FVector End = Start + (HitTarget - Start) * 1.25f;
	FHitResult TraceHitResult;
	GetWorld()->LineTraceSingleByChannel(
		TraceHitResult,
		Start,
		End,
		ECollisionChannel::ECC_Visibility
	);
	/*DrawDebugLine(
		GetWorld(),
		Start,
		TraceHitResult.ImpactPoint,
		FColor::Red,
		false,
		2,
		2,
		1
	);*/

	if (FireEffectMuzzle)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			FireEffectMuzzle,
			WeaponMesh,
			FName("Muzzle"),
			FVector(0.f),
			FRotator(0.f),
			EAttachLocation::Type::KeepRelativeOffset,
			true
		);
	}
	AZombieCharacter* HitZombie = Cast<AZombieCharacter>(TraceHitResult.GetActor());
	if (HitZombie && HasAuthority())
	{
		UGameplayStatics::ApplyPointDamage(
			HitZombie,
			Damage,
			TraceHitResult.ImpactPoint,
			TraceHitResult,
			GetOwner()->GetInstigatorController(),
			this,
			UDamageType::StaticClass()
		);
	}
	if (FireBodyEffectImpact && HitZombie)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			FireBodyEffectImpact,
			TraceHitResult.ImpactPoint,
			TraceHitResult.ImpactNormal.Rotation()
		);
	}
	if (FireEffectImpact && TraceHitResult.bBlockingHit && HitZombie == nullptr)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			FireEffectImpact,
			TraceHitResult.ImpactPoint,
			TraceHitResult.ImpactNormal.Rotation()
		);
	}
	if (BeamParticles)
	{
		UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			BeamParticles,
			WeaponMesh->GetSocketLocation("Muzzle"),
			FRotator::ZeroRotator,
			true
		);
		FVector BeamEnd = TraceHitResult.bBlockingHit == true ? TraceHitResult.ImpactPoint : TraceHitResult.TraceEnd;
		if (Beam)
		{
			Beam->SetVectorParameter(FName("Target"), BeamEnd);
		}
	}
}

void AWeapon::Reload()
{
	if (Magazine > 0)
		Ammo = MaxAmmo;
	Magazine = FMath::Clamp(Magazine-1, 0, Magazine);
	UpdateMagazineHUD();
	UpdateAmmoHUD();
}

void AWeapon::SetVisibility(bool visibility)
{
	WeaponMesh->SetVisibility(visibility);
	MagazineMesh->SetVisibility(visibility);
}

void AWeapon::OnRep_Magazine()
{
	UpdateMagazineHUD();
}

void AWeapon::OnRep_Ammo()
{
	UpdateAmmoHUD();
}

void AWeapon::UpdateAmmoHUD()
{
	Character = Character == nullptr ? Cast<AHumanCharacter>(GetOwner()) : Character;
	if (Character)
	{
		CharacterController = CharacterController == nullptr ? Cast<AHumanPlayerController>(Character->GetController()) : CharacterController;
		if (CharacterController)
			CharacterController->SetAmmoHUD(Ammo);
	}
}

void AWeapon::UpdateMagazineHUD()
{
	Character = Character == nullptr ? Cast<AHumanCharacter>(GetOwner()) : Character;
	if (Character)
	{
		CharacterController = CharacterController == nullptr ? Cast<AHumanPlayerController>(Character->GetController()) : CharacterController;
		if (CharacterController)
			CharacterController->SetMagazineHUD(Magazine);
	}
}