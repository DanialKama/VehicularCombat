// Copyright 2022 Danial Kamali. All Rights Reserved.

#include "Actors/WeaponPickupActor.h"
#include "Actors/ProjectileActor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Camera/CameraComponent.h"
#include "Sound/SoundCue.h"
#include "Net/UnrealNetwork.h"

AWeaponPickupActor::AWeaponPickupActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh"));
	SetRootComponent(SkeletalMesh);
	SkeletalMesh->SetComponentTickEnabled(false);
	SkeletalMesh->bApplyImpulseOnDamage = false;
	SkeletalMesh->CanCharacterStepUpOn = ECB_No;
	SkeletalMesh->SetCollisionProfileName("Pickup");

	// Initialize variables
	ProjectileRef = nullptr;
	WeaponType = EWeaponType::Primary;
	AmmoType = EAmmoType::Rocket;
	WeaponName = EWeaponName::NoName;
	bIsAutomatic = false;
	TimeBetweenShots = 0.5;
	Range = 5000.0f;
	ReloadAmount = CurrentMagazineAmmo = MagazineSize = 1;
	ReloadTime = 2.0f;
}

void AWeaponPickupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate to everyone
	DOREPLIFETIME(AWeaponPickupActor, MagazineSize);
	DOREPLIFETIME(AWeaponPickupActor, CurrentMagazineAmmo);
	DOREPLIFETIME(AWeaponPickupActor, ReloadAmount);
	DOREPLIFETIME(AWeaponPickupActor, ReloadTime);
}

void AWeaponPickupActor::BeginPlay()
{
	Super::BeginPlay();

	if (GetLocalRole() == ROLE_Authority)
	{
		ProjectileRef = ProjectileClass.GetDefaultObject();
	}
}

bool AWeaponPickupActor::ServerSpawnProjectile_Validate(FTransform NewTransform)
{
	if (ProjectileRef)
	{
		return true;
	}
	return false;
}

void AWeaponPickupActor::ServerSpawnProjectile_Implementation(FTransform NewTransform)
{
	// if (bIsAimed)	// TODO - Need cleanup
	// {
	// 	ReferenceTransform = NewTransform;
	// 	const FTransform Transform = ProjectileLineTrace();
	// 	MulticastSpawnProjectile(ProjectileClass, ProjectileRef->NumberOfPellets, Transform.GetLocation(), Transform.Rotator(), Owner);
	// }
	// else
	// {
		MulticastSpawnProjectile(ProjectileClass, ProjectileRef->NumberOfPellets, SkeletalMesh->GetSocketLocation(FName("MuzzleSocket")), SkeletalMesh->GetSocketRotation(FName("MuzzleSocket")), Owner);
	// }

	MulticastWeaponEffects();
}

void AWeaponPickupActor::MulticastSpawnProjectile_Implementation(TSubclassOf<AProjectileActor> ProjectileToSpawn, int32 NumberOfPellets, FVector Location, FRotator Rotation, AActor* OwnerRef)
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = OwnerRef;
	SpawnParameters.Instigator = OwnerRef->GetInstigator();
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	for(uint8 i = 0; i < NumberOfPellets; ++i)
	{
		GetWorld()->SpawnActor<AProjectileActor>(ProjectileToSpawn, Location, Rotation, SpawnParameters);
	}
}

FTransform AWeaponPickupActor::ProjectileLineTrace() const
{
	FVector Start;
	FVector End;
	
	CalculateLineTrace(Start, End);
	
	FHitResult HitResult;
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.bTraceComplex = true;
	CollisionQueryParams.AddIgnoredActor(this);
	CollisionQueryParams.AddIgnoredActor(Owner);
	const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionQueryParams);

	FTransform OutTransform;
	const FVector MuzzleLocation = SkeletalMesh->GetSocketLocation(TEXT("MuzzleSocket"));
	OutTransform.SetLocation(MuzzleLocation);
	bHit ? OutTransform.SetRotation(UKismetMathLibrary::FindLookAtRotation(MuzzleLocation, HitResult.ImpactPoint).Quaternion()) : OutTransform.SetRotation(UKismetMathLibrary::FindLookAtRotation(MuzzleLocation, HitResult.TraceEnd).Quaternion());

	return OutTransform;
}

void AWeaponPickupActor::CalculateLineTrace(FVector& Start, FVector& End) const
{
	const FVector TraceStart = ReferenceTransform.GetLocation();
	const FVector UpVector = ReferenceTransform.GetUnitAxis(EAxis::Z);
	const FVector RightVector = ReferenceTransform.GetUnitAxis(EAxis::Y);
	const FVector TraceEnd = ReferenceTransform.GetUnitAxis(EAxis::X);

	if (ProjectileRef->NumberOfPellets > 1)
	{
		const FRotator Points = RandomPointInCircle(FMath::FRandRange(ProjectileRef->PelletSpread * -1.0f, ProjectileRef->PelletSpread), true);
		Start = TraceStart;
		const FVector EndPoint = TraceStart + TraceEnd * Range;
		End = EndPoint + RightVector * Points.Roll + UpVector * Points.Pitch;
	}
	else
	{
		Start = TraceStart;
		End = TraceStart + TraceEnd * Range;
	}
}

FRotator AWeaponPickupActor::RandomPointInCircle(const float Radius, const bool bIncludesNegative) const
{
	// Distance From Center can be a random value from 0 to Radius or just Radius
	float DistanceFromCenter;
	// DistanceFromCenter = FMath::FRandRange(0.0f, Radius); // Option 1
	DistanceFromCenter = Radius; // Option 2
	const float Angle = FMath::FRandRange(0.0f, 360.0f);

	FRotator Points;
	if (bIncludesNegative)
	{
		Points.Roll = DistanceFromCenter * UKismetMathLibrary::DegCos(Angle);
		Points.Pitch = DistanceFromCenter * UKismetMathLibrary::DegSin(Angle);
		Points.Yaw = 0.0f;
	}
	else
	{
		Points.Roll = abs(DistanceFromCenter * UKismetMathLibrary::DegCos(Angle));
		Points.Pitch = abs(DistanceFromCenter * UKismetMathLibrary::DegSin(Angle));
		Points.Yaw = 0.0f;
	}
	
	return Points;
}

void AWeaponPickupActor::MulticastWeaponEffects_Implementation()
{
	const FVector Location = SkeletalMesh->GetSocketLocation(FName("MuzzleSocket"));
	const FRotator Rotation = SkeletalMesh->GetSocketRotation(FName("MuzzleSocket"));
	
	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), Effects.FireSound, Location);
	
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Effects.MuzzleFlash, Location, Rotation, Effects.MuzzleFlashScale);
	if (Effects.MuzzleFlashRear)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Effects.MuzzleFlashRear, Location, Rotation, Effects.MuzzleFlashScale);
	}
}

void AWeaponPickupActor::OnRep_PickupState()
{
	switch (PickupState)
	{
	case 0:
		// Picked up
		SkeletalMesh->SetSimulatePhysics(false);
		SkeletalMesh->SetCollisionProfileName("Weapon");
		SetLifeSpan(0.0f);
		break;
	case 1:
		// Dropped
		SetOwner(nullptr);
		SkeletalMesh->SetSimulatePhysics(true);
		SkeletalMesh->SetCollisionProfileName("Pickup");
		SetLifeSpan(FMath::FRandRange(10.0f, 15.0f));
		break;
	case 2:
		// Used
		Destroy();
		break;
	}
}