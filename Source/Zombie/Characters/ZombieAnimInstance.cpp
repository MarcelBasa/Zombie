#include "ZombieAnimInstance.h"
#include "ZombieCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"


void UZombieAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	ZombieCharacter = Cast<AZombieCharacter>(TryGetPawnOwner());
}

void UZombieAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (ZombieCharacter == nullptr)
	{
		ZombieCharacter = Cast<AZombieCharacter>(TryGetPawnOwner());
	}
	if (ZombieCharacter == nullptr) return;

	FVector Velocity = ZombieCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = ZombieCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = ZombieCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bIsAlive = ZombieCharacter->IsAlive();
}
