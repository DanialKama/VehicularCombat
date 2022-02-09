// Copyright 2022 Danial Kamali. All Rights Reserved.

#include "Actors/SpawnManagerActor.h"
#include "NavigationSystem.h"
#include "Components/BillboardComponent.h"
#include "Components/SphereComponent.h"
#include "Actors/PickupActor.h"
#include "Actors/WeaponPickupActor.h"
#include "Actors/AmmoPickupActor.h"
#include "Actors/HealthPickupActor.h"
#include "Actors/SpeedBoostPickupActor.h"
#include "Kismet/GameplayStatics.h"

ASpawnManagerActor::ASpawnManagerActor()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	SetCanBeDamaged(false);

	Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
	SetRootComponent(Billboard);
	Billboard->PrimaryComponentTick.bStartWithTickEnabled = false;
	Billboard->bIsScreenSizeScaled = true;
	
	RespawnRadius = CreateDefaultSubobject<USphereComponent>(TEXT("Respawn Radius"));
	RespawnRadius->SetupAttachment(Billboard);
	RespawnRadius->PrimaryComponentTick.bStartWithTickEnabled = false;
	RespawnRadius->SetSphereRadius(250.0f);
	RespawnRadius->SetGenerateOverlapEvents(false);
	RespawnRadius->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RespawnRadius->SetCollisionResponseToAllChannels(ECR_Ignore);
	RespawnRadius->CanCharacterStepUpOn = ECB_No;

	// Initialize variables
	FirstSpawnDelay = 30.0f;
}

void ASpawnManagerActor::BeginPlay()
{
	Super::BeginPlay();

	if (GetLocalRole() == ROLE_Authority)
	{
		// Initialize navigation system
		NavigationSystem = UNavigationSystemV1::GetCurrent(GetWorld());

		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ASpawnManagerActor::ServerInitiateSpawn, FirstSpawnDelay);
	}
}

void ASpawnManagerActor::ServerInitiateSpawn_Implementation()
{
	// Start respawning if there is any actor to respawn
	if (WeaponSpawnInfo.WeaponsToSpawn.Num() > 0)
	{
		ServerSpawnWeapon();
	}

	if (AmmoSpawnInfo.AmmoToSpawn.Num() > 0)
	{
		ServerSpawnAmmo();
	}

	if (HealthSpawnInfo.HealthToSpawn.Num() > 0)
	{
		ServerSpawnHealth();
	}

	if (SpeedBoostSpawnInfo.SpeedBoostToSpawn.Num() > 0)
	{
		ServerSpawnSpeedBoost();
	}
}

void ASpawnManagerActor::ServerEnterSpawnQueue_Implementation(EPickupType PickupType)
{
	switch (PickupType)
	{
	case 0:
		// Weapon
		GetWorld()->GetTimerManager().SetTimer(SpawnWeaponTimer, this, &ASpawnManagerActor::ServerSpawnWeapon, WeaponSpawnInfo.SpawnDelay);
		break;
	case 1:
		// Ammo
		GetWorld()->GetTimerManager().SetTimer(SpawnAmmoTimer, this, &ASpawnManagerActor::ServerSpawnAmmo, AmmoSpawnInfo.SpawnDelay);
		break;
	case 2:
		// Health
		GetWorld()->GetTimerManager().SetTimer(SpawnHealthTimer, this, &ASpawnManagerActor::ServerSpawnHealth, HealthSpawnInfo.SpawnDelay);
		break;
	case 3:
		// Speed Boost
		GetWorld()->GetTimerManager().SetTimer(SpawnSpeedBoostTimer, this, &ASpawnManagerActor::ServerSpawnSpeedBoost, SpeedBoostSpawnInfo.SpawnDelay);
		break;
	}
}

void ASpawnManagerActor::ServerSpawnWeapon_Implementation()
{
	const int32 Index = FMath::RandRange(0, WeaponSpawnInfo.WeaponsToSpawn.Num() - 1);
	MulticastSpawn(CalculateSpawnTransform(), WeaponSpawnInfo.WeaponsToSpawn[Index]);
}

void ASpawnManagerActor::ServerSpawnAmmo_Implementation()
{
	const int32 Index = FMath::RandRange(0, AmmoSpawnInfo.AmmoToSpawn.Num() - 1);
	MulticastSpawn(CalculateSpawnTransform(), AmmoSpawnInfo.AmmoToSpawn[Index]);
}

void ASpawnManagerActor::ServerSpawnHealth_Implementation()
{
	const int32 Index = FMath::RandRange(0, HealthSpawnInfo.HealthToSpawn.Num() - 1);
	MulticastSpawn(CalculateSpawnTransform(), HealthSpawnInfo.HealthToSpawn[Index]);
}

void ASpawnManagerActor::ServerSpawnSpeedBoost_Implementation()
{
	const int32 Index = FMath::RandRange(0, SpeedBoostSpawnInfo.SpeedBoostToSpawn.Num() - 1);
	MulticastSpawn(CalculateSpawnTransform(), SpeedBoostSpawnInfo.SpeedBoostToSpawn[Index]);
}

void ASpawnManagerActor::MulticastSpawn_Implementation(FTransform Transform, TSubclassOf<APickupActor> ActorToSpawn)
{
	APickupActor* NewActor = GetWorld()->SpawnActorDeferred<APickupActor>(ActorToSpawn, Transform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	if (NewActor)
	{
		if (GetLocalRole() == ROLE_Authority)
		{
			NewActor->bIsSpawned = true;
			NewActor->SpawnManager = this;
		}
		
		UGameplayStatics::FinishSpawningActor(NewActor, Transform);
	}
}

FTransform ASpawnManagerActor::CalculateSpawnTransform() const
{
	FTransform Transform;
	FNavLocation NavLocation;
	const bool bResult = NavigationSystem->GetRandomReachablePointInRadius(GetActorLocation(), RespawnRadius->GetScaledSphereRadius(), NavLocation);
	bResult ? Transform.SetLocation(NavLocation.Location) : Transform.SetLocation(GetActorLocation());
	Transform.SetRotation(FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f).Quaternion());
	return Transform;
}