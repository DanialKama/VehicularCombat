// Copyright 2022 Danial Kamali. All Rights Reserved.

#include "Core/CustomPlayerState.h"
#include "Actors/WeaponPickupActor.h"
#include "Net/UnrealNetwork.h"

ACustomPlayerState::ACustomPlayerState()
{
	// Initialize variables
	PrimaryWeapon = SecondaryWeapon =  nullptr;
	MiniGunAmmo = 999;
	ShotgunAmmo = 60;
	Rocket = 10;
}

void ACustomPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate to everyone
	DOREPLIFETIME(ACustomPlayerState, PrimaryWeapon);
	DOREPLIFETIME(ACustomPlayerState, SecondaryWeapon);
	DOREPLIFETIME(ACustomPlayerState, MiniGunAmmo);
	DOREPLIFETIME(ACustomPlayerState, ShotgunAmmo);
	DOREPLIFETIME(ACustomPlayerState, Rocket);
}

void ACustomPlayerState::ServerPlayerDied_Implementation()
{
	const FDetachmentTransformRules DetachmentRules = FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepRelative, false);
	if (PrimaryWeapon)
	{
		PrimaryWeapon->DetachFromActor(DetachmentRules);
		PrimaryWeapon->PickupState = EPickupState::Dropped;
		PrimaryWeapon->OnRep_PickupState();
		PrimaryWeapon = nullptr;
	}

	if (SecondaryWeapon)
	{
		SecondaryWeapon->DetachFromActor(DetachmentRules);
		SecondaryWeapon->PickupState = EPickupState::Dropped;
		SecondaryWeapon->OnRep_PickupState();
		SecondaryWeapon = nullptr;
	}
}