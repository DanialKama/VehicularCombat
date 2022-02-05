// Copyright 2022 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CustomPlayerState.generated.h"

class AWeaponPickupActor;

UCLASS()
class VEHICULARCOMBAT_API ACustomPlayerState : public APlayerState
{
	GENERATED_BODY()

// Functions
public:
	ACustomPlayerState();

	/** When the player left the session */
	UFUNCTION(Server, Reliable)
	void ServerPlayerDied();
	void ServerPlayerDied_Implementation();
	
// Variables
public:
	UPROPERTY(Replicated)
	AWeaponPickupActor* PrimaryWeapon;

	UPROPERTY(Replicated)
	AWeaponPickupActor* SecondaryWeapon;

	/** MiniGun Ammo */
	UPROPERTY(Replicated)
	int32 MiniGunAmmo;

	/** Shotgun Ammo */
	UPROPERTY(Replicated)
	int32 ShotgunAmmo;

	/** 40 mm HE Grenade */
	UPROPERTY(Replicated)
	int32 Rocket;
};