#include "CombatComponent.h"
#include "Zombie/Characters/HumanCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Zombie/Weapons/Weapon.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Zombie/SpawnPoint.h"


UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (Character && Character->HasAuthority())
	{
		EquipStartWeapon();
	}
}

void UCombatComponent::EquipStartWeapon()
{
	if (Character->StartingWeaponClass)
	{
		AWeapon* Pistol = GetWorld()->SpawnActor<AWeapon>(Character->StartingWeaponClass, Character->GetActorLocation(), Character->GetActorRotation());
		EquipWeapon(Pistol);
		Weapon->UpdateAmmoHUD();
		Weapon->UpdateMagazineHUD();	
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, Weapon);
	DOREPLIFETIME(UCombatComponent, bReload);
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;

	if (Weapon)
	{
		Weapon->Destroy();
	}
	Weapon = WeaponToEquip;
	Weapon->SetOwner(Character);
	Weapon->SetActorEnableCollision(false);
	StartAttachActorToRightHandTimer(Weapon);
	if(Weapon->GetSpawnPoint())
		Weapon->GetSpawnPoint()->WeaponTaked();
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (Weapon == nullptr || Character == nullptr) return;

	Weapon->SetActorEnableCollision(false);
	Character->PlayEquipMontage();
	StartAttachActorToRightHandTimer(Weapon);
}

void UCombatComponent::StartAttachActorToRightHandTimer(AWeapon* WeaponToAttach)
{
	if (WeaponToAttach == nullptr || Character == nullptr) return;

	bFireButtonPressed = false;
	bCanFire = false;
	WeaponToAttach->SetVisibility(false);
	Character->SetFABRIK(false);
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &ThisClass::AttachActorToRightHand, WeaponToAttach);
	Character->PlayEquipMontage();
	Character->GetWorldTimerManager().SetTimer(
		EquipTimer,
		TimerDelegate,
		0.8f,
		false
	);
}

void UCombatComponent::AttachActorToRightHand(AWeapon* WeaponToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || WeaponToAttach == nullptr) return;

	bCanFire = true;
	WeaponToAttach->SetVisibility(true);
	Character->SetFABRIK(true);
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(WeaponToAttach->GetWeaponSocketName());
	if (HandSocket)
	{
		HandSocket->AttachActor(WeaponToAttach, Character->GetMesh());
	}
	Weapon->UpdateAmmoHUD();
	Weapon->UpdateMagazineHUD();
}

void UCombatComponent::ReloadButtonPressed()
{
	if (Weapon == nullptr || bIsReloading || Character == nullptr) return;

	if (Weapon->GetMagazineCount() <= 0)
	{
		Weapon->Destroy();
		EquipStartWeapon();
		return;
	}
	bIsReloading = true;
	ServerReload();
}

void UCombatComponent::OnRep_IsReloading()
{
	if (Weapon == nullptr || Character == nullptr) return;
	
	StartReloadTimer(Weapon->GetMagazineMesh());
	Character->PlayReloadMontage();
}

void UCombatComponent::ServerReload_Implementation()
{
	if (Weapon == nullptr || Character == nullptr) return;

	bReload = !bReload;
	StartReloadTimer(Weapon->GetMagazineMesh());
}

void UCombatComponent::StartReloadTimer(UStaticMeshComponent* Mesh)
{
	if (Weapon == nullptr || Character == nullptr) return;

	Character->SetFABRIK(false);
	Character->PlayReloadMontage();
	Mesh->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, FName("hand_l_Magazine"));
	Character->GetWorldTimerManager().SetTimer(
		ReloadTimer,
		this,
		&ThisClass::FinishReloadTimer,
		1.9f
	);
}
void UCombatComponent::FinishReloadTimer()
{
	if (Weapon == nullptr || Character == nullptr) return;

	AttachActorToLeftHand();
	if (Character->HasAuthority())
		Weapon->Reload();

	bIsReloading = false;

	if (bFireButtonPressed)
		Fire();
}

void UCombatComponent::AttachActorToLeftHand()
{
	Character->SetFABRIK(true);
	Weapon->GetMagazineMesh()->AttachToComponent(Weapon->GetWeaponMesh(), FAttachmentTransformRules::KeepRelativeTransform, FName("Magazine"));
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	if (Weapon == nullptr || Character == nullptr) return;
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::Fire()
{
	if (bCanFire && Weapon->IsEmpty() == false && bIsReloading == false)
	{
		bCanFire = false;
		FHitResult HitResult;
		Trace(HitResult);
		FVector TraceEnd = HitResult.bBlockingHit == true ? HitResult.ImpactPoint : HitResult.TraceEnd;
		ServerFire(TraceEnd);
		Recoil();
		StartFireTimer();
	}
}

void UCombatComponent::StartFireTimer()
{
	if (Weapon == nullptr || Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&ThisClass::FireTimerFinished,
		Weapon->FireDelay
	);
}

void UCombatComponent::FireTimerFinished()
{
	if (Weapon == nullptr) return;
	bCanFire = true;
	if (Weapon->IsEmpty())
		ReloadButtonPressed();
	if (bFireButtonPressed && Weapon->bAutomatic)
	{
		Fire();
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (Weapon == nullptr) return;
	if (Character)
	{
		Weapon->Fire(TraceHitTarget);
		Character->PlayFireMontage();
	}
}

void UCombatComponent::Trace(FHitResult& HitResultCamera)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;

		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 20.f);
		}

		FVector End = Start + CrosshairWorldDirection * 8000;

		GetWorld()->LineTraceSingleByChannel(
			HitResultCamera,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);
	}
}

void UCombatComponent::Recoil()
{
	if (Character == nullptr || Weapon == nullptr) return;

	Character->AddControllerPitchInput(FMath::RandRange(Weapon->RecoilPitch * -3, Weapon->RecoilPitch));
	Character->AddControllerYawInput(FMath::RandRange(Weapon->RecoilYaw * -1, Weapon->RecoilYaw));
}

