#include "HumanAnimInstance.h"
#include "HumanCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Zombie/Weapons/Weapon.h"


void UHumanAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	HumanCharacter = Cast<AHumanCharacter>(TryGetPawnOwner());
}

void UHumanAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (HumanCharacter == nullptr)
	{
		HumanCharacter = Cast<AHumanCharacter>(TryGetPawnOwner());
	}
	if (HumanCharacter == nullptr) return;

	FVector Velocity = HumanCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = HumanCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = HumanCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bUseFABRIK = HumanCharacter->GetFABRIK();
	bTurnLeft = HumanCharacter->GetTurnLeft();
	bTurnRight = HumanCharacter->GetTurnRight();
	bIsAlive = HumanCharacter->IsAlive();

	FRotator AimRotation = HumanCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(HumanCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;

	if (bIsInAir)
		Pitch = 0;//FMath::Clamp(DeltaRot.Pitch * (-1), -40, 40);
	else
		Pitch = DeltaRot.Pitch * (-1);

	if (HumanCharacter->GetWeapon() && HumanCharacter->GetMesh())
	{
		LeftHandTransform = HumanCharacter->GetWeapon()->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		HumanCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));
	}
}
