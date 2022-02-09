// Copyright 2022 Danial Kamali. All Rights Reserved.

#include "Components/BaseComponent.h"
#include "Pawns/BaseWheeledVehiclePawn.h"
#include "Net/UnrealNetwork.h"

UBaseComponent::UBaseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UBaseComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate to everyone
	DOREPLIFETIME(UBaseComponent, ComponentOwner);
}

void UBaseComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwnerRole() == ROLE_Authority)
	{
		SetIsReplicated(true);
	}
}

void UBaseComponent::ServerInitialize_Implementation()
{
	ComponentOwner = Cast<ABaseWheeledVehiclePawn>(GetOwner());
}