// Copyright 2022 Danial Kamali. All Rights Reserved.

#include "HealthComponent.h"
#include "Pawns/BaseWheeledVehiclePawn.h"
#include "Net/UnrealNetwork.h"

UHealthComponent::UHealthComponent()
{
	// Initialize variables
	CurrentHealth = MaxHealth = 1000.0f;
	RestoreAmount = 50.0f;
	RestoreDelay = 2.0f;
	bRestoreHealth = false;
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate to everyone
	DOREPLIFETIME(UHealthComponent, CurrentHealth);
	DOREPLIFETIME(UHealthComponent, MaxHealth);
	DOREPLIFETIME(UHealthComponent, RestoreAmount);
	DOREPLIFETIME(UHealthComponent, RestoreDelay);
	DOREPLIFETIME(UHealthComponent, bRestoreHealth);
}

void UHealthComponent::ServerInitialize_Implementation()
{
	Super::ServerInitialize_Implementation();

	CurrentHealth = MaxHealth;
	ComponentOwner->ServerSetHealthLevel(CurrentHealth, MaxHealth);
	ComponentOwner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::TakeAnyDamage);
}

void UHealthComponent::TakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (GetOwnerRole() == ROLE_Authority && Damage > 0.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(RestoreHealthTimer);
		CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);
		ComponentOwner->ServerSetHealthLevel(CurrentHealth, MaxHealth);
	}
}

void UHealthComponent::ServerIncreaseHealth_Implementation(float IncreaseAmount)
{
	CurrentHealth = FMath::Clamp(CurrentHealth + IncreaseAmount, 0.0f, MaxHealth);
	ComponentOwner->ServerSetHealthLevel(CurrentHealth, MaxHealth);
}

void UHealthComponent::ServerStartRestoreHealth_Implementation()
{
	GetWorld()->GetTimerManager().SetTimer(RestoreHealthTimer, this, &UHealthComponent::ServerRestoreHealth, 0.2f, true, RestoreDelay);
}

bool UHealthComponent::ServerRestoreHealth_Validate()
{
	if (CurrentHealth >= MaxHealth)
	{
		ServerStopRestoreHealth();
		return false;
	}
	return true;
}

void UHealthComponent::ServerRestoreHealth_Implementation()
{
	bRestoreHealth = true;
		
	if (CurrentHealth >= MaxHealth)
	{
		ServerStopRestoreHealth();
	}
	else
	{
		CurrentHealth = FMath::Clamp(CurrentHealth + RestoreAmount, 0.0f, MaxHealth);
		ComponentOwner->ServerSetHealthLevel(CurrentHealth, MaxHealth);
	}
}

void UHealthComponent::ServerStopRestoreHealth_Implementation()
{
	bRestoreHealth = false;
	GetWorld()->GetTimerManager().ClearTimer(RestoreHealthTimer);
}