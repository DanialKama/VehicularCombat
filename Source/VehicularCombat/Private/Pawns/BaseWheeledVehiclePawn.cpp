// Copyright 2022 Danial Kamali. All Rights Reserved.

#include "Pawns/BaseWheeledVehiclePawn.h"
#include "Components/AudioComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Actors/WeaponPickupActor.h"
#include "Components/HealthComponent.h"
#include "Core/CustomPlayerState.h"
#include "Net/UnrealNetwork.h"

const FName ABaseWheeledVehiclePawn::EngineAudioRPM("RPM");

PRAGMA_DISABLE_DEPRECATION_WARNINGS

ABaseWheeledVehiclePawn::ABaseWheeledVehiclePawn()
{
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);

	GetVehicleMovementComponent()->EnableSelfRighting(true);

	// Setup the audio component
	EngineSoundComp = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineSound"));
	EngineSoundComp->SetupAttachment(GetMesh());
	EngineSoundComp->SetComponentTickEnabled(false);
	
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("Health Component"));

	// Initialize variables
	CurrentWeapon = nullptr;
	PlayerStateRef = nullptr;
	RespawnDelay = 5.0f;
	bDoOnceMoving = bDoOnceStopped = true;
	bIsAlive = true;
	bCanFireWeapon = true;
	bDoOnceReload = true;
}

void ABaseWheeledVehiclePawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate to everyone
	DOREPLIFETIME(ABaseWheeledVehiclePawn, PlayerStateRef);
	DOREPLIFETIME(ABaseWheeledVehiclePawn, CurrentCamera);
	DOREPLIFETIME(ABaseWheeledVehiclePawn, bDoOnceReload);
	DOREPLIFETIME(ABaseWheeledVehiclePawn, bCanFireWeapon);
	DOREPLIFETIME(ABaseWheeledVehiclePawn, CurrentWeapon);
	DOREPLIFETIME(ABaseWheeledVehiclePawn, CurrentWeaponSlot);
	DOREPLIFETIME(ABaseWheeledVehiclePawn, bIsAlive);
	DOREPLIFETIME(ABaseWheeledVehiclePawn, RespawnDelay);
}

void ABaseWheeledVehiclePawn::BeginPlay()
{
	Super::BeginPlay();
	
	if (GetLocalRole() == ROLE_Authority)
	{
		HealthComponent->ServerInitialize();
	}
}

void ABaseWheeledVehiclePawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	PlayerStateRef = Cast<ACustomPlayerState>(NewController->PlayerState);
}

void ABaseWheeledVehiclePawn::Tick(float Delta)
{
	Super::Tick(Delta);

	// Pass the engine RPM to the sound component
	const UChaosWheeledVehicleMovementComponent* WheeledVehicle = static_cast<UChaosWheeledVehicleMovementComponent*>(GetVehicleMovement());
	const float RPMToAudioScale = 2500.0f / WheeledVehicle->GetEngineMaxRotationSpeed();
	EngineSoundComp->SetFloatParameter(EngineAudioRPM, WheeledVehicle->GetEngineRotationSpeed()*RPMToAudioScale);
}

void ABaseWheeledVehiclePawn::ServerInteractWithWeapon_Implementation()	// TODO - Check for weapon
{
	// AWeaponPickupActor* NewWeapon = Cast<AWeaponPickupActor>(???);
	// if (NewWeapon)
	// {
	// 	ServerAddWeapon(NewWeapon);
	// }
}

bool ABaseWheeledVehiclePawn::ServerAddWeapon_Validate(AWeaponPickupActor* NewWeapon)
{
	if (NewWeapon)
	{
		return true;
	}
	return false;
}

void ABaseWheeledVehiclePawn::ServerAddWeapon_Implementation(AWeaponPickupActor* NewWeapon)
{
	// Update state of the new weapon
	NewWeapon->SetOwner(this);
	NewWeapon->PickupState = EPickupState::PickedUp;
	NewWeapon->OnRep_PickupState();
		
	const FAttachmentTransformRules AttachmentRules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
	const FDetachmentTransformRules DetachmentRules = FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepRelative, false);
	
	switch (NewWeapon->WeaponType)
	{
	case 0:
		// Primary
		if (PlayerStateRef->PrimaryWeapon == nullptr)
		{
			NewWeapon->AttachToComponent(GetMesh(), AttachmentRules, FName("PrimaryWeaponSocket"));
		}
		else if (PlayerStateRef->PrimaryWeapon != nullptr)
		{
			PlayerStateRef->PrimaryWeapon->DetachFromActor(DetachmentRules);
			PlayerStateRef->PrimaryWeapon->PickupState = EPickupState::Used;
			PlayerStateRef->PrimaryWeapon->OnRep_PickupState();
			NewWeapon->AttachToComponent(GetMesh(), AttachmentRules, FName("PrimaryWeaponSocket"));
		}
		PlayerStateRef->PrimaryWeapon = NewWeapon;
		CurrentWeapon = NewWeapon;
		CurrentWeaponSlot = EWeaponToDo::Primary;
		break;
	case 1:
		// Secondary
		if (PlayerStateRef->SecondaryWeapon == nullptr)
		{
			NewWeapon->AttachToComponent(GetMesh(), AttachmentRules, FName("SecondaryWeaponSocket"));
		}
		else if (PlayerStateRef->SecondaryWeapon != nullptr)
		{
			PlayerStateRef->SecondaryWeapon->DetachFromActor(DetachmentRules);
			PlayerStateRef->SecondaryWeapon->PickupState = EPickupState::Used;
			PlayerStateRef->SecondaryWeapon->OnRep_PickupState();
			NewWeapon->AttachToComponent(GetMesh(), AttachmentRules, FName("SecondaryWeaponSocket"));
		}
		PlayerStateRef->SecondaryWeapon = NewWeapon;
		break;
	}
}

bool ABaseWheeledVehiclePawn::ServerSwitchWeapon_Validate(EWeaponToDo NewWeapon)
{
	if (bIsAlive && CurrentWeaponSlot != NewWeapon)
	{
		switch (NewWeapon)
		{
		case 0:
			// No Weapon = Nothing to switch
			return false;
		case 1:
			// Primary Weapon
			if (PlayerStateRef->PrimaryWeapon)
			{
				return true;
			}
			return false;
		case 2:
			// Secondary Weapon
			if (PlayerStateRef->SecondaryWeapon)
			{
				return true;
			}
			return false;
		}
	}
	return false;
}

void ABaseWheeledVehiclePawn::ServerSwitchWeapon_Implementation(EWeaponToDo NewWeapon)
{
	GetWorld()->GetTimerManager().ClearTimer(ReloadTimer);
	CurrentWeaponSlot = NewWeapon;
	switch (NewWeapon)
	{
	case 0:
		// No Weapon
		break;
	case 1:
		// Primary Weapon
		CurrentWeapon = PlayerStateRef->PrimaryWeapon;
		break;
	case 2:
		// Secondary Weapon
		CurrentWeapon = PlayerStateRef->SecondaryWeapon;
		break;
	}
}

bool ABaseWheeledVehiclePawn::ServerStartFireWeapon_Validate()
{
	if (CanFireWeapon())
	{
		return true;
	}
	return false;
}

void ABaseWheeledVehiclePawn::ServerStartFireWeapon_Implementation()
{
	ServerFireWeapon();
	if (CurrentWeapon->bIsAutomatic)
	{
		GetWorld()->GetTimerManager().SetTimer(FireWeaponTimer, this, &ABaseWheeledVehiclePawn::ServerFireWeapon, CurrentWeapon->TimeBetweenShots, true);
	}

	bCanFireWeapon = false;
	GetWorld()->GetTimerManager().SetTimer(ResetFireWeaponTimer, this, &ABaseWheeledVehiclePawn::ServerResetFireWeapon, CurrentWeapon->TimeBetweenShots);
}

bool ABaseWheeledVehiclePawn::ServerStopFireWeapon_Validate()
{
	if (CurrentWeapon)
	{
		return true;
	}
	return false;
}

void ABaseWheeledVehiclePawn::ServerStopFireWeapon_Implementation()
{
	GetWorld()->GetTimerManager().ClearTimer(FireWeaponTimer);
}

bool ABaseWheeledVehiclePawn::ServerFireWeapon_Validate()
{
	// Checking if there is any ammo for this weapon, and if not, stop firing the weapon
	if (CanFireWeapon())
	{
		return true;
	}
	GetWorld()->GetTimerManager().ClearTimer(FireWeaponTimer);
	return false;
}

void ABaseWheeledVehiclePawn::ServerFireWeapon_Implementation()
{
	CurrentWeapon->ServerSpawnProjectile(GetActorTransform());

	CurrentWeapon->CurrentMagazineAmmo = --CurrentWeapon->CurrentMagazineAmmo;
	ClientUpdateAmmo(CurrentWeapon->CurrentMagazineAmmo);

	if (CurrentWeapon->CurrentMagazineAmmo <= 0)
	{
		if (CanReloadWeapon())
		{
			ClientUpdateWeaponState(EWeaponState::Reloading);
			GetWorld()->GetTimerManager().SetTimer(ReloadTimer, this, &ABaseWheeledVehiclePawn::ServerReloadWeapon, CurrentWeapon->ReloadTime);
		}
		else
		{
			ClientUpdateWeaponState(EWeaponState::NoAmmo);
		}
	}
}

bool ABaseWheeledVehiclePawn::CanFireWeapon() const
{
	if (bIsAlive && bCanFireWeapon && CurrentWeapon && CurrentWeapon->CurrentMagazineAmmo > 0)
	{
		return true;
	}
	return false;
}

void ABaseWheeledVehiclePawn::ServerResetFireWeapon_Implementation()
{
	bCanFireWeapon = true;
}

void ABaseWheeledVehiclePawn::ServerReloadWeapon_Implementation()
{
	switch (CurrentWeapon->AmmoType)
	{
		int32 UsedAmmoFromMag;
		int32 CurrentReloadAmount;
	case 0:
		// Assault Rifle ammo
		UsedAmmoFromMag = CurrentWeapon->MagazineSize - CurrentWeapon->CurrentMagazineAmmo;
		if (PlayerStateRef->AssaultRifleAmmo >= UsedAmmoFromMag)
		{
			CurrentReloadAmount = FMath::Clamp(CurrentWeapon->ReloadAmount, 0, UsedAmmoFromMag);
			PlayerStateRef->AssaultRifleAmmo -= CurrentReloadAmount;
			CurrentWeapon->CurrentMagazineAmmo = CurrentReloadAmount + CurrentWeapon->CurrentMagazineAmmo;
		}
		else
		{
			CurrentWeapon->CurrentMagazineAmmo += PlayerStateRef->AssaultRifleAmmo;
			PlayerStateRef->AssaultRifleAmmo = 0;
		}
		break;
	case 1:
		// MiniGun Ammo
		UsedAmmoFromMag = CurrentWeapon->MagazineSize - CurrentWeapon->CurrentMagazineAmmo;
		if (PlayerStateRef->MiniGunAmmo >= UsedAmmoFromMag)
		{
			CurrentReloadAmount = FMath::Clamp(CurrentWeapon->ReloadAmount, 0, UsedAmmoFromMag);
			PlayerStateRef->MiniGunAmmo -= CurrentReloadAmount;
			CurrentWeapon->CurrentMagazineAmmo = CurrentReloadAmount + CurrentWeapon->CurrentMagazineAmmo;
		}
		else
		{
			CurrentWeapon->CurrentMagazineAmmo += PlayerStateRef->MiniGunAmmo;
			PlayerStateRef->MiniGunAmmo = 0;
		}
		break;
	case 2:
		// Shotgun Ammo
		UsedAmmoFromMag = CurrentWeapon->MagazineSize - CurrentWeapon->CurrentMagazineAmmo;
		if (PlayerStateRef->ShotgunAmmo >= UsedAmmoFromMag)
		{
			CurrentReloadAmount = FMath::Clamp(CurrentWeapon->ReloadAmount, 0, UsedAmmoFromMag);
			PlayerStateRef->ShotgunAmmo -= CurrentReloadAmount;
			CurrentWeapon->CurrentMagazineAmmo = CurrentReloadAmount + CurrentWeapon->CurrentMagazineAmmo;
		}
		else
		{
			CurrentWeapon->CurrentMagazineAmmo += PlayerStateRef->ShotgunAmmo;
			PlayerStateRef->ShotgunAmmo = 0;
		}
		break;
	case 3:
		// Rocket Ammo
		UsedAmmoFromMag = CurrentWeapon->MagazineSize - CurrentWeapon->CurrentMagazineAmmo;
		if (PlayerStateRef->RocketAmmo >= UsedAmmoFromMag)
		{
			CurrentReloadAmount = FMath::Clamp(CurrentWeapon->ReloadAmount, 0, UsedAmmoFromMag);
			PlayerStateRef->RocketAmmo -= CurrentReloadAmount;
			CurrentWeapon->CurrentMagazineAmmo = CurrentReloadAmount + CurrentWeapon->CurrentMagazineAmmo;
		}
		else
		{
			CurrentWeapon->CurrentMagazineAmmo += PlayerStateRef->RocketAmmo;
			PlayerStateRef->RocketAmmo = 0;
		}
		break;
	}
	
	ClientUpdateAmmo(CurrentWeapon->CurrentMagazineAmmo);
	ClientUpdateWeaponState(EWeaponState::Idle);
	bDoOnceReload = true;
}

bool ABaseWheeledVehiclePawn::CanReloadWeapon() const
{
	if (bDoOnceReload && CurrentWeapon && CurrentWeapon->CurrentMagazineAmmo < CurrentWeapon->MagazineSize)
	{
		switch (CurrentWeapon->AmmoType)
		{
		case 0:
			// Assault Rifle ammo
			if (PlayerStateRef->AssaultRifleAmmo > 0)
			{
				return true;
			}
			return false;
		case 1:
			// MiniGun Ammo
			if (PlayerStateRef->MiniGunAmmo > 0)
			{
				return true;
			}
			return false;
		case 2:
			// Shotgun Ammo
			if (PlayerStateRef->ShotgunAmmo > 0)
			{
				return true;
			}
			return false;
		case 3:
			// Rocket Ammo
			if (PlayerStateRef->RocketAmmo > 0)
			{
				return true;
			}
			return false;
		}
	}
	return false;
}

void ABaseWheeledVehiclePawn::ServerSetHealthLevel_Implementation(float CurrentHealth, float MaxHealth)
{
	// Start restoring health if 0 < Current Health < Max Health and restoring health is not started yet.
	if (CurrentHealth > 0.0f && CurrentHealth < MaxHealth && HealthComponent->bRestoreHealth != true)
	{
		HealthComponent->ServerStartRestoreHealth();
	}
	if (CurrentHealth <= 0.0f && bIsAlive)
	{
		CurrentWeapon = nullptr;
		bIsAlive = false;
		OnRep_IsAlive();
		MulticastDeath();

		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ABaseWheeledVehiclePawn::ServerStartDestroy, RespawnDelay);
	}
		
	ClientUpdateHealth(CurrentHealth / MaxHealth);
}

void ABaseWheeledVehiclePawn::MulticastDeath_Implementation()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		PlayerStateRef->ServerPlayerDied();
	}

	HealthComponent->DestroyComponent();
}

void ABaseWheeledVehiclePawn::ServerStartDestroy_Implementation()
{
	Destroy();
}

// Override in the player vehicle class
void ABaseWheeledVehiclePawn::ClientUpdateWeaponState_Implementation(EWeaponState WeaponState)
{
}

void ABaseWheeledVehiclePawn::ClientUpdateAmmo_Implementation(int32 CurrentMagAmmo)
{
}

void ABaseWheeledVehiclePawn::ClientUpdateHealth_Implementation(float NewHealth)
{
}

void ABaseWheeledVehiclePawn::OnRep_CurrentWeapon()
{
}

void ABaseWheeledVehiclePawn::OnRep_IsAlive()
{
}

PRAGMA_ENABLE_DEPRECATION_WARNINGS