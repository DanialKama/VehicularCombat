// Copyright 2022 Danial Kamali. All Rights Reserved.

#include "WeaponPickupActor.h"
#include "ProjectileActor.h"
#include "SpawnManagerActor.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"

AWeaponPickupActor::AWeaponPickupActor()
{
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh"));
	SetRootComponent(SkeletalMesh);
	SkeletalMesh->PrimaryComponentTick.bStartWithTickEnabled = false;
	SkeletalMesh->bApplyImpulseOnDamage = false;
	SkeletalMesh->CanCharacterStepUpOn = ECB_No;
	SkeletalMesh->SetCollisionProfileName("Pickup");
	
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Collision"));
	SphereCollision->SetupAttachment(SkeletalMesh);
	SphereCollision->PrimaryComponentTick.bStartWithTickEnabled = false;
	SphereCollision->bApplyImpulseOnDamage = false;
	SphereCollision->CanCharacterStepUpOn = ECB_No;
	SphereCollision->SetCollisionProfileName("CollisionBound");
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);	// Collision is only get enabled when actor get dropped or spawned.
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &APickupActor::OnBeginOverlap);
	SphereCollision->OnComponentEndOverlap.AddDynamic(this, &APickupActor::OnEndOverlap);

	// Initialize variables
	PickupType = EPickupType::Weapon;
	ProjectileRef = nullptr;
	WeaponType = EWeaponType::Primary;
	AmmoType = EAmmoType::Rocket;
	WeaponName = EWeaponName::NoName;
	bIsAutomatic = false;
	TimeBetweenShots = 0.5;
	Range = 5000.0f;
	MaxAmmo = 150;
	CurrentAmmo = 120;
	ReloadAmount = CurrentMagazineAmmo = MagazineSize = 1;
	ReloadTime = 2.0f;
}

void AWeaponPickupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate to everyone
	DOREPLIFETIME(AWeaponPickupActor, MaxAmmo);
	DOREPLIFETIME(AWeaponPickupActor, CurrentAmmo);
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
	MulticastSpawnProjectile(ProjectileClass, ProjectileRef->NumberOfPellets, SkeletalMesh->GetSocketLocation(FName("MuzzleSocket")), SkeletalMesh->GetSocketRotation(FName("MuzzleSocket")), Owner);
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
		SphereCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SetLifeSpan(0.0f);
		if (SpawnManager)
		{
			SpawnManager->ServerEnterSpawnQueue(PickupType);
			SpawnManager = nullptr;
		}
		break;
	case 1:
		// Dropped
		SetOwner(nullptr);
		SkeletalMesh->SetSimulatePhysics(true);
		SkeletalMesh->SetCollisionProfileName("Pickup");
		SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		SetLifeSpan(FMath::FRandRange(10.0f, 15.0f));
		break;
	case 2:
		// Used
		Destroy();
		break;
	}
}