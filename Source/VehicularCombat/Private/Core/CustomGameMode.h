// Copyright 2022 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CustomGameMode.generated.h"

UCLASS()
class ACustomGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
// Functions
public:
	/** Respawn Player */
	UFUNCTION(Server, Reliable)
	void ServerStartRespawn(AController* Controller);

protected:
	virtual void Logout(AController* Exiting) override;

private:
	void ServerStartRespawn_Implementation(AController* Controller);

	UFUNCTION(Server, Reliable)
	void ServerRespawn();
	void ServerRespawn_Implementation();

// Variables
private:
	UPROPERTY()
	TArray<AController*> ControllersToRespawn;
	
	UPROPERTY()
	TArray<AController*> ExitedControllers;
};