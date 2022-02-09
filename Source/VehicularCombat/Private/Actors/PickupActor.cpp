// Copyright 2022 Danial Kamali. All Rights Reserved.

#include "Actors/PickupActor.h"
#include "Pawns/BaseWheeledVehiclePawn.h"
#include "Actors/SpawnManagerActor.h"
#include "Net/UnrealNetwork.h"

APickupActor::APickupActor()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	SetCanBeDamaged(false);

	// Initialize variables
	PickupType = EPickupType::Weapon;
	PickupState = EPickupState::Dropped;
	bIsSpawned = false;
}

void APickupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate to everyone
	DOREPLIFETIME(APickupActor, PickupState);
	DOREPLIFETIME(APickupActor, bIsSpawned);
}

void APickupActor::BeginPlay()
{
	Super::BeginPlay();

	if (GetLocalRole() == ROLE_Authority)
	{
		SetReplicates(true);
		
		if (bIsSpawned)
		{
			PickupState = EPickupState::Dropped;
			OnRep_PickupState();
		}
	}
	else if (bIsSpawned)
	{
		OnRep_PickupState();
	}
}

void APickupActor::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		ABaseWheeledVehiclePawn* BaseVehicle = Cast<ABaseWheeledVehiclePawn>(OtherActor);
		if (BaseVehicle)
		{
			BaseVehicle->PickupRef = this;
			BaseVehicle->OnRep_PickupRef();
		}
	}
}

void APickupActor::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		ABaseWheeledVehiclePawn* BaseVehicle = Cast<ABaseWheeledVehiclePawn>(OtherActor);
		if (BaseVehicle)
		{
			BaseVehicle->PickupRef = nullptr;
			BaseVehicle->OnRep_PickupRef();
		}
	}
}

void APickupActor::OnRep_PickupState()
{
	switch (PickupState)
	{
	case 0:
		// Picked up
		break;
	case 1:
		// Dropped
		break;
	case 2:
		// Used
		break;
	}
}

void APickupActor::Destroyed()
{
	if (SpawnManager)
	{
		SpawnManager->ServerEnterSpawnQueue(PickupType);
	}

	Super::Destroyed();
}