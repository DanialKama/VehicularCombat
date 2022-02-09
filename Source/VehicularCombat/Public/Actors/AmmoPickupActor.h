// Copyright 2022 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/PickupActor.h"
#include "AmmoPickupActor.generated.h"

UCLASS()
class VEHICULARCOMBAT_API AAmmoPickupActor : public APickupActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* SphereCollision;

// Functions
public:
	/** Sets default values for this actor's properties */
	AAmmoPickupActor();
	
	virtual void OnRep_PickupState() override;

// Variables
public:
	UPROPERTY(Replicated, EditDefaultsOnly, Category = "Defaults", meta = (ClampMin = "1", UIMin = "1"))
	EAmmoType AmmoType;
	
	UPROPERTY(Replicated, EditDefaultsOnly, Category = "Defaults", meta = (ClampMin = "1", UIMin = "1"))
	int32 AmmoAmount;
};