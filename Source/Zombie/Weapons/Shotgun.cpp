#include "Shotgun.h"
#include "Kismet/KismetMathLibrary.h"

void AShotgun::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	float Distance = FVector::Dist(HitTarget, GetOwner()->GetActorLocation());
	Distance /= 15;
	Shot(HitTarget+FVector(Distance, Distance,Distance));
	Shot(HitTarget+FVector(-Distance,Distance,Distance));
	Shot(HitTarget+FVector(-Distance,-Distance,Distance));
	Shot(HitTarget+FVector(-Distance,-Distance,-Distance));
	Shot(HitTarget+FVector(Distance,Distance,-Distance));
	Shot(HitTarget+FVector(Distance,-Distance,-Distance));
	Shot(HitTarget+FVector(-Distance,Distance,-Distance));
}