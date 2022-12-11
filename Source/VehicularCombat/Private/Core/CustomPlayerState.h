// Copyright 2022 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CustomPlayerState.generated.h"

class AWeaponPickupActor;
class AAmmoPickupActor;

UCLASS()
class ACustomPlayerState : public APlayerState
{
	GENERATED_BODY()

// Functions
public:
	ACustomPlayerState();

	/** When the player left the session */
	UFUNCTION(Server, Reliable)
	void ServerPlayerDied();

private:
	void ServerPlayerDied_Implementation();
	
// Variables
public:
	UPROPERTY(Replicated)
	AWeaponPickupActor* PrimaryWeapon;

	UPROPERTY(Replicated)
	AWeaponPickupActor* SecondaryWeapon;
};