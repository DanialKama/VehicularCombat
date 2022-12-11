// Copyright 2022 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PickupActor.h"
#include "SpeedBoostPickupActor.generated.h"

UCLASS()
class ASpeedBoostPickupActor : public APickupActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* SphereCollision;

// Functions
public:
	/** Sets default values for this actor's properties */
	ASpeedBoostPickupActor();
	
	virtual void OnRep_PickupState() override;

// Variables
public:
	UPROPERTY(Replicated, EditDefaultsOnly, Category = "Defaults", meta = (ClampMin = "1", UIMin = "1"))
	float BoostTime;

	UPROPERTY(Replicated, EditDefaultsOnly, Category = "Defaults", meta = (ClampMin = "1", UIMin = "1"))
	float BoostMultiplier;
};