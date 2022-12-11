// Copyright 2022 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Enumerations/ActorEnums.h"
#include "GameFramework/Actor.h"
#include "SpawnManagerActor.generated.h"

class APickupActor;

USTRUCT(BlueprintType)
struct FWeaponSpawnInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly)
	float SpawnDelay;
	
	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<APickupActor>> WeaponsToSpawn;

	// Default constructor
	FWeaponSpawnInfo()
	{
		SpawnDelay = 5.0f;
	}
};

USTRUCT(BlueprintType)
struct FAmmoSpawnInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, Category = "Defaults")
	float SpawnDelay;
	
	UPROPERTY(EditDefaultsOnly, Category = "Defaults")
	TArray<TSubclassOf<APickupActor>> AmmoToSpawn;

	// Default constructor
	FAmmoSpawnInfo()
	{
		SpawnDelay = 5.0f;
	}
};

USTRUCT(BlueprintType)
struct FHealthSpawnInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, Category = "Defaults")
	float SpawnDelay;
	
	UPROPERTY(EditDefaultsOnly, Category = "Defaults")
	TArray<TSubclassOf<APickupActor>> HealthToSpawn;

	// Default constructor
	FHealthSpawnInfo()
	{
		SpawnDelay = 5.0f;
	}
};

USTRUCT(BlueprintType)
struct FSpeedBoostSpawnInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, Category = "Defaults")
	float SpawnDelay;
	
	UPROPERTY(EditDefaultsOnly, Category = "Defaults")
	TArray<TSubclassOf<APickupActor>> SpeedBoostToSpawn;

	// Default constructor
	FSpeedBoostSpawnInfo()
	{
		SpawnDelay = 5.0f;
	}
};

UCLASS()
class ASpawnManagerActor : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UBillboardComponent* Billboard;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* RespawnRadius;

// Functions
public:
	ASpawnManagerActor();

	/** Enter the respawn queue and wait till respawning */
	UFUNCTION(Server, Reliable)
	void ServerEnterSpawnQueue(EPickupType PickupType);

protected:
	virtual void BeginPlay() override;

private:
	/** Starting the first wave of spawning. */
	UFUNCTION(Server, Reliable)
	void ServerInitiateSpawn();
	void ServerInitiateSpawn_Implementation();
	
	UFUNCTION(Server, Reliable)
	void ServerSpawnWeapon();
	void ServerSpawnWeapon_Implementation();

	UFUNCTION(Server, Reliable)
	void ServerSpawnAmmo();
	void ServerSpawnAmmo_Implementation();

	UFUNCTION(Server, Reliable)
	void ServerSpawnHealth();
	void ServerSpawnHealth_Implementation();

	UFUNCTION(Server, Reliable)
	void ServerSpawnSpeedBoost();
	void ServerSpawnSpeedBoost_Implementation();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpawn(FTransform Transform, TSubclassOf<APickupActor> ActorToSpawn);
	void MulticastSpawn_Implementation(FTransform Transform, TSubclassOf<APickupActor> ActorToSpawn);

	FTransform CalculateSpawnTransform() const;

	void ServerEnterSpawnQueue_Implementation(EPickupType PickupType);
	
// Variables
private:
	UPROPERTY(EditDefaultsOnly, Category = "Defaults")
	float FirstSpawnDelay;

	UPROPERTY(EditDefaultsOnly, Category = "Defaults")
	FWeaponSpawnInfo WeaponSpawnInfo;
	
	UPROPERTY(EditDefaultsOnly, Category = "Defaults")
	FAmmoSpawnInfo AmmoSpawnInfo;
	
	UPROPERTY(EditDefaultsOnly, Category = "Defaults")
	FHealthSpawnInfo HealthSpawnInfo;
	
	UPROPERTY(EditDefaultsOnly, Category = "Defaults")
	FSpeedBoostSpawnInfo SpeedBoostSpawnInfo;

	UPROPERTY()
	class UNavigationSystemV1* NavigationSystem;

	FTimerHandle SpawnWeaponTimer, SpawnAmmoTimer, SpawnHealthTimer, SpawnSpeedBoostTimer;
};