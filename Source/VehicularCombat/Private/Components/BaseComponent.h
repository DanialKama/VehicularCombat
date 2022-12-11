// Copyright 2022 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BaseComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UBaseComponent : public UActorComponent
{
	GENERATED_BODY()

// Functions
public:	
	/** Sets default values for this component's properties */
	UBaseComponent();

	/** Initialize this component */
	UFUNCTION(Server, Reliable)
	void ServerInitialize();

protected:
	virtual void BeginPlay() override;

	virtual void ServerInitialize_Implementation();

// Variables
protected:
	/** Owner of this component */
	UPROPERTY(Replicated)
	class ABaseWheeledVehiclePawn* ComponentOwner;
};