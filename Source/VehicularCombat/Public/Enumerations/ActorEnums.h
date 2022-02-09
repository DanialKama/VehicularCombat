// Copyright 2022 Danial Kamali. All Rights Reserved.

#pragma once

#include "ActorEnums.generated.h"

UENUM(BlueprintType)
enum class EPickupType : uint8
{
	Weapon,
	Ammo,
	Health,
	SpeedBoost
};

UENUM()
enum class EPickupState : uint8
{
	PickedUp,
	Dropped,
	Used
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Primary,
	Secondary
};

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	Idle,
	Firing,
	Reloading,
	NoAmmo
};

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	AssaultRifle	UMETA(DisplayName = "Assault Rifle ammo"),	// Assault Rifle ammo
	MiniGun			UMETA(DisplayName = "Minigun ammo"),		// MiniGun ammo
	Shotgun			UMETA(DisplayName = "Shotgun ammo"),		// Shotgun ammo
	Rocket			UMETA(DisplayName = "Rocket ammo")			// Rocket ammo
};

/** To use in player UI */
UENUM(BlueprintType)
enum class EWeaponName : uint8
{
	NoName		UMETA(DisplayName = "No Name"),
	AssaultRifle	UMETA(DisplayName = "Assault Rifle"),
	MiniGun			UMETA(DisplayName = "Minigun"),
	Shotgun			UMETA(DisplayName = "Shotgun"),
	RocketLauncher		UMETA(DisplayName = "Rocket Launcher")
};