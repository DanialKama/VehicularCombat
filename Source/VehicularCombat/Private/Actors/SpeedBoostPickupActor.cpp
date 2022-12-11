// Copyright 2022 Danial Kamali. All Rights Reserved.

#include "SpeedBoostPickupActor.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "Pawns/BaseWheeledVehiclePawn.h"

ASpeedBoostPickupActor::ASpeedBoostPickupActor()
{
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Skeletal Mesh"));
	SetRootComponent(StaticMesh);
	StaticMesh->PrimaryComponentTick.bStartWithTickEnabled = false;
	StaticMesh->bApplyImpulseOnDamage = false;
	StaticMesh->CanCharacterStepUpOn = ECB_No;
	StaticMesh->SetCollisionProfileName("Pickup");
	
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Collision"));
	SphereCollision->SetupAttachment(StaticMesh);
	SphereCollision->PrimaryComponentTick.bStartWithTickEnabled = false;
	SphereCollision->bApplyImpulseOnDamage = false;
	SphereCollision->CanCharacterStepUpOn = ECB_No;
	SphereCollision->SetCollisionProfileName("CollisionBound");
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);	// Collision is only get enabled when actor get dropped or spawned.
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &APickupActor::OnBeginOverlap);
	SphereCollision->OnComponentEndOverlap.AddDynamic(this, &APickupActor::OnEndOverlap);

	// Initialize variables
	PickupType = EPickupType::SpeedBoost;
	BoostTime = 5.0f;
	BoostMultiplier = 1.5f;
}

void ASpeedBoostPickupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate to everyone
	DOREPLIFETIME(ASpeedBoostPickupActor, BoostTime);
	DOREPLIFETIME(ASpeedBoostPickupActor, BoostMultiplier);
}

void ASpeedBoostPickupActor::OnRep_PickupState()
{
	switch (PickupState)
	{
	case 0: case 2:
		// Picked up, Used
		Destroy();
		break;
	case 1:
		// Dropped
		SetOwner(nullptr);
		StaticMesh->SetSimulatePhysics(true);
		StaticMesh->SetCollisionProfileName("Pickup");
		SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		SetLifeSpan(FMath::FRandRange(5.0f, 10.0f));
		break;
	}
}