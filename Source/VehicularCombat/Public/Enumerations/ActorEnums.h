// Copyright 2022 Danial Kamali. All Rights Reserved.

#pragma once

#include "ActorEnums.generated.h"

UENUM()
enum class EPickupType : uint8
{
	Weapon,
	Ammo,
	Health
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
enum class EAmmoType : uint8
{
	AssaultRifle	UMETA(DisplayName = "Assault Rifle ammo"),	// Assault Rifle ammo
	MiniGun			UMETA(DisplayName = "Minigun ammo"),		// MiniGun ammo
	Shotgun			UMETA(DisplayName = "Shotgun ammo"),		// Shotgun ammo
	Rocket			UMETA(DisplayName = "Rocket")				// Rocket
};

/** To use in player UI */
UENUM(BlueprintType)
enum class EWeaponName : uint8
{
	Default			UMETA(DisplayName = "Default"),
	AssaultRifle	UMETA(DisplayName = "Assault Rifle"),
	MiniGun			UMETA(DisplayName = "Minigun"),
	Shotgun			UMETA(DisplayName = "Shotgun"),
	Launcher		UMETA(DisplayName = "Launcher")
};