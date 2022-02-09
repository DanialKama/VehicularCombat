// Copyright 2022 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Enumerations/ActorEnums.h"
#include "PickupActor.generated.h"

UCLASS()
class VEHICULARCOMBAT_API APickupActor : public AActor
{
	GENERATED_BODY()

// Functions
public:	
	/** Sets default values for this actor's properties */
	APickupActor();

	UFUNCTION()
	virtual void OnRep_PickupState();
	
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	
// Variables
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defaults")
	EPickupType PickupType;

	UPROPERTY(Replicated, EditInstanceOnly, Category = "Defaults")
	uint8 bIsSpawned : 1;

	UPROPERTY(ReplicatedUsing = OnRep_PickupState)
	EPickupState PickupState;

	UPROPERTY()
	class ASpawnManagerActor* SpawnManager;
};