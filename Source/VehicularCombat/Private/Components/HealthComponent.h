// Copyright 2022 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseComponent.h"
#include "HealthComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UHealthComponent : public UBaseComponent
{
	GENERATED_BODY()

// Functions
public:	
	/** Sets default values for this component's properties */
	UHealthComponent();
	
	UFUNCTION(Server, Reliable)
	void ServerIncreaseHealth(float IncreaseAmount);

	UFUNCTION(Server, Reliable)
	void ServerStartRestoreHealth();

	UFUNCTION(Server, Reliable)
	void ServerStopRestoreHealth();
	void ServerStopRestoreHealth_Implementation();
	
private:
	virtual void ServerInitialize_Implementation() override;

	UFUNCTION()
	void TakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);
	
	void ServerIncreaseHealth_Implementation(float IncreaseAmount);

	void ServerStartRestoreHealth_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRestoreHealth();
	bool ServerRestoreHealth_Validate();
	void ServerRestoreHealth_Implementation();

// Variables
public:
	UPROPERTY(Replicated, EditAnywhere, Category = "Defaults", meta = (ClampMin = "0.0", UIMin = "0.0", AllowPrivateAccess = true))
	float MaxHealth;
	
	UPROPERTY(Replicated)
	float CurrentHealth;

	UPROPERTY(Replicated)
	uint8 bRestoreHealth : 1;
	
private:
	UPROPERTY(Replicated, EditAnywhere, Category = "Defaults", meta = (ClampMin = "0.0", UIMin = "0.0", AllowPrivateAccess = true))
	float RestoreAmount;

	UPROPERTY(Replicated, EditAnywhere, Category = "Defaults", meta = (ClampMin = "0.0", UIMin = "0.0", AllowPrivateAccess = true))
	float RestoreDelay;

	FTimerHandle RestoreHealthTimer;
};