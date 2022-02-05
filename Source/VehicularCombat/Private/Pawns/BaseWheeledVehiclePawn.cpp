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
	bDoOnceDeath = true;
	
	// Initialize variables
	CurrentWeapon = nullptr;
	PlayerStateRef = nullptr;
	RespawnDelay = 5.0f;
	bDoOnceMoving = bDoOnceStopped = true;
	bDoOnceDeath = true;
	bCanFireWeapon = true;
	bDoOnceReload = true;
}

void ABaseWheeledVehiclePawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate to everyone
	DOREPLIFETIME(ABaseWheeledVehiclePawn, PlayerStateRef);
	DOREPLIFETIME(ABaseWheeledVehiclePawn, bDoOnceReload);
	DOREPLIFETIME(ABaseWheeledVehiclePawn, bCanFireWeapon);
	DOREPLIFETIME(ABaseWheeledVehiclePawn, CurrentWeapon);
	DOREPLIFETIME(ABaseWheeledVehiclePawn, bDoOnceDeath);
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

void ABaseWheeledVehiclePawn::OnRep_CurrentWeapon()
{
	// Override in the player vehicle class
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
	CurrentWeapon->CurrentMagazineAmmo = --CurrentWeapon->CurrentMagazineAmmo;
	CurrentWeapon->ServerSpawnProjectile(GetActorTransform());
}

bool ABaseWheeledVehiclePawn::CanFireWeapon() const
{
	if (bCanFireWeapon && CurrentWeapon && CurrentWeapon->CurrentMagazineAmmo > 0)
	{
		return true;
	}
	return false;
}

void ABaseWheeledVehiclePawn::ServerResetFireWeapon_Implementation()
{
	bCanFireWeapon = true;
}

bool ABaseWheeledVehiclePawn::ServerReloadWeapon_Validate()
{
	if (bDoOnceReload && CanReloadWeapon())
	{
		return true;
	}
	return false;
}

void ABaseWheeledVehiclePawn::ServerReloadWeapon_Implementation()
{
	bDoOnceReload = false;
}

bool ABaseWheeledVehiclePawn::CanReloadWeapon() const
{
	if (CurrentWeapon && CurrentWeapon->CurrentMagazineAmmo < CurrentWeapon->MagazineSize)
	{
		switch (CurrentWeapon->AmmoType)
		{
		case 0:
			// MiniGun Ammo
			if (PlayerStateRef->MiniGunAmmo > 0)
			{
				return true;
			}
			return false;
		case 1:
			// Shotgun Ammo
			if (PlayerStateRef->ShotgunAmmo > 0)
			{
				return true;
			}
			return false;
		case 2:
			// Rocket
			if (PlayerStateRef->Rocket > 0)
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
	if (CurrentHealth <= 0.0f && bDoOnceDeath)
	{
		bDoOnceDeath = false;
		MulticastDeath();

		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ABaseWheeledVehiclePawn::ServerStartDestroy, RespawnDelay);
	}
		
	ClientUpdateHealth(CurrentHealth / MaxHealth);
}

void ABaseWheeledVehiclePawn::ClientUpdateHealth_Implementation(float NewHealth)
{
	// Override in the player vehicle class
}

void ABaseWheeledVehiclePawn::MulticastDeath_Implementation()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		PlayerStateRef->ServerPlayerDied();
	}

	GetMesh()->bPauseAnims = true;
	HealthComponent->DestroyComponent();
}

void ABaseWheeledVehiclePawn::ServerStartDestroy_Implementation()
{
	Destroy();
}

PRAGMA_ENABLE_DEPRECATION_WARNINGS