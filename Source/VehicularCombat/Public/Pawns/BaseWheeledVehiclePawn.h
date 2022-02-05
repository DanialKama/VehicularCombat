// Copyright 2022 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "BaseWheeledVehiclePawn.generated.h"

class APickupActor;
class AWeaponPickupActor;

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
	
	UFUNCTION(Server, Reliable)
	void ServerSetHealthLevel(float CurrentHealth, float MaxHealth);

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(Server, Reliable)
	void ServerInteractWithWeapon();
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAddWeapon(AWeaponPickupActor* NewWeapon);

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
	
	virtual void ClientUpdateHealth_Implementation(float NewHealth);

private:
	void ServerInteractWithWeapon_Implementation();

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

	void ServerSetHealthLevel_Implementation(float CurrentHealth, float MaxHealth);
	
	/** Update health on player UI */
	UFUNCTION(Client, Unreliable)
	void ClientUpdateHealth(float NewHealth);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastDeath();
	void MulticastDeath_Implementation();

	UFUNCTION(Server, Reliable)
	void ServerStartDestroy();
	void ServerStartDestroy_Implementation();

// Variables
protected:
	UPROPERTY(Replicated)
	class ACustomPlayerState* PlayerStateRef;

	/** The weapon that is currently in the player's hand */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon, BlueprintReadOnly, Category = "Defaults")
	AWeaponPickupActor* CurrentWeapon;

	/** To call Multicast Death only once */
	UPROPERTY(Replicated)
	uint8 bDoOnceDeath : 1;

	UPROPERTY(Replicated)
	uint8 bCanFireWeapon : 1;

	UPROPERTY(Replicated)
	uint8 bDoOnceReload : 1;
	
	/** To check only once if character is moving or not */
	uint8 bDoOnceMoving : 1, bDoOnceStopped : 1;

	/** Use as a reference for projectile line trace. */
	UPROPERTY()
	class UCameraComponent* CurrentCamera;

private:
	static const FName EngineAudioRPM;

	UPROPERTY(Replicated, EditDefaultsOnly, Category = "Defaults", meta = (ClampMin = "0.0", UIMin = "0.0", AllowPrivateAccess = true))
	float RespawnDelay;
	
	FTimerHandle FireWeaponTimer, ResetFireWeaponTimer;
};

PRAGMA_ENABLE_DEPRECATION_WARNINGS