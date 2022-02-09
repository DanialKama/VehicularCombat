// Copyright 2022 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "Enumerations/ActorEnums.h"
#include "Enumerations/CharacterEnums.h"
#include "BaseWheeledVehiclePawn.generated.h"

class APickupActor;
class AWeaponPickupActor;
class AAmmoPickupActor;

PRAGMA_DISABLE_DEPRECATION_WARNINGS

UCLASS()
class VEHICULARCOMBAT_API ABaseWheeledVehiclePawn : public AWheeledVehiclePawn
{
	GENERATED_BODY()

	/** Audio component for the engine sound */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UAudioComponent* EngineSoundComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UHealthComponent* HealthComponent;

// Functions
public:
	ABaseWheeledVehiclePawn();

	UFUNCTION()
	void OnRep_PickupRef();
	
	UFUNCTION(Server, Reliable)
	void ServerSetHealthLevel(float CurrentHealth, float MaxHealth);

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void Tick(float DeltaSeconds) override;
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAddWeapon(AWeaponPickupActor* NewWeapon);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSwitchWeapon(EWeaponToDo NewWeapon);
	bool ServerSwitchWeapon_Validate(EWeaponToDo NewWeapon);
	void ServerSwitchWeapon_Implementation(EWeaponToDo NewWeapon);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartFireWeapon();
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopFireWeapon();

	/** Checking if there is any ammo for this weapon. */
	bool CanFireWeapon() const;

	/** Checking if there is any ammo in inventory. */
	bool CanReloadWeapon() const;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReloadWeapon();
	
	UFUNCTION()
	virtual void OnRep_CurrentWeapon();

	UFUNCTION()
	virtual void OnRep_IsAlive();
	
	virtual void ClientUpdateWeaponState_Implementation(EWeaponState WeaponState);
	
	UFUNCTION(Client, Unreliable)
	void ClientUpdateAmmo(int32 CurrentAmmo);
	virtual void ClientUpdateAmmo_Implementation(int32 CurrentAmmo);

	virtual void ClientUpdatePickup_Implementation(EPickupType PickupType);
	virtual void ServerSetHealthLevel_Implementation(float CurrentHealth, float MaxHealth);
	virtual void ClientUpdateMagAmmo_Implementation(int32 CurrentMagAmmo);

private:
	bool ServerStartFireWeapon_Validate();
	void ServerStartFireWeapon_Implementation();

	bool ServerStopFireWeapon_Validate();
	void ServerStopFireWeapon_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFireWeapon();
	bool ServerFireWeapon_Validate();
	void ServerFireWeapon_Implementation();

	/** Add a delay to stop the player from firing faster than the weapon's fire rate */
	UFUNCTION(Server, Reliable)
	void ServerResetFireWeapon();
	void ServerResetFireWeapon_Implementation();

	bool ServerReloadWeapon_Validate();
	void ServerReloadWeapon_Implementation();
	
	bool ServerAddWeapon_Validate(AWeaponPickupActor* NewWeapon);
	void ServerAddWeapon_Implementation(AWeaponPickupActor* NewWeapon);

	UFUNCTION(Server, Reliable)
	void ServerUpdateCurrentWeapon(AWeaponPickupActor* NewWeapon, EWeaponToDo NewWeaponSlot);
	void ServerUpdateCurrentWeapon_Implementation(AWeaponPickupActor* NewWeapon, EWeaponToDo NewWeaponSlot);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAddAmmo(AAmmoPickupActor* PickupAmmo);
	bool ServerAddAmmo_Validate(AAmmoPickupActor* PickupAmmo);
	void ServerAddAmmo_Implementation(AAmmoPickupActor* PickupAmmo);

	UFUNCTION(Client, Unreliable)
	void ClientUpdatePickup(EPickupType PickupType);
	
	UFUNCTION(Client, Reliable)
	void ClientUpdateWeaponState(EWeaponState WeaponState);
	
	UFUNCTION(Client, Unreliable)
	void ClientUpdateMagAmmo(int32 CurrentMagAmmo);
	
	/** Update speed boost time */
	UFUNCTION(Server, Reliable)
	void ServerUpdateSpeedBoost();
	void ServerUpdateSpeedBoost_Implementation();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastDeath();
	void MulticastDeath_Implementation();

	UFUNCTION(Server, Reliable)
	void ServerStartDestroy();
	void ServerStartDestroy_Implementation();

// Variables
public:
	UPROPERTY(ReplicatedUsing = OnRep_PickupRef)
	APickupActor* PickupRef;
	
protected:
	UPROPERTY(Replicated)
	class ACustomPlayerState* PlayerStateRef;

	/** The weapon that is currently in the player's hand */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon, BlueprintReadOnly)
	AWeaponPickupActor* CurrentWeapon;

	UPROPERTY(Replicated)
	EWeaponToDo CurrentWeaponSlot;

	/** To call Multicast Death only once */
	UPROPERTY(ReplicatedUsing = OnRep_IsAlive)
	uint8 bIsAlive : 1;

	UPROPERTY(Replicated)
	uint8 bCanFireWeapon : 1;

	UPROPERTY(Replicated)
	uint8 bDoOnceReload : 1;

	UPROPERTY(Replicated)
	uint8 bBoostSpeed : 1;

	UPROPERTY(Replicated)
	float SpeedBoostMultiplier;
	
	/** To check only once if character is moving or not */
	uint8 bDoOnceMoving : 1, bDoOnceStopped : 1;

	/** Use as a reference for projectile line trace. */
	UPROPERTY(Replicated)
	class UCameraComponent* CurrentCamera;

private:
	static const FName EngineAudioRPM;

	UPROPERTY(Replicated, EditDefaultsOnly, Category = "Defaults", meta = (ClampMin = "0.0", UIMin = "0.0", AllowPrivateAccess = true))
	float RespawnDelay;

	UPROPERTY(Replicated)
	float SpeedBoostTime;
	
	FTimerHandle FireWeaponTimer, ResetFireWeaponTimer, ReloadTimer, SpeedBoostTimer;
};

PRAGMA_ENABLE_DEPRECATION_WARNINGS