// Copyright 2022 Danial Kamali. All Rights Reserved.

#include "Core/CustomGameMode.h"

void ACustomGameMode::ServerStartRespawn_Implementation(AController* Controller)
{
	if (Controller)
	{
		ControllersToRespawn.Add(Controller);

		FTimerDelegate TimerDelegate;
		TimerDelegate.BindUObject(this, &ACustomGameMode::ServerRespawn);
		GetWorld()->GetTimerManager().SetTimerForNextTick(TimerDelegate);
	}
}

void ACustomGameMode::ServerRespawn_Implementation()
{
	// Check if the player is still in the session
	if (ExitedControllers.Num() > 0)
	{
		for (uint8 i = 0; i < ControllersToRespawn.Num(); ++i)
		{
			for (uint8 j = 0; j < ExitedControllers.Num(); ++j)
			{
				// Exited players will be removed from the controllers to respawn and will not get respawned
				if (ControllersToRespawn[i] == ExitedControllers[j])
				{
					ControllersToRespawn.RemoveAt(i);
				}

				ExitedControllers[j]->Destroy();
				ExitedControllers.RemoveAt(j);
			}
		}
	}

	// Respawn players
	if (ControllersToRespawn.Num() > 0)
	{
		for (uint8 k = 0; k < ControllersToRespawn.Num(); ++k)
		{
			const AActor* RespawnActor = ChoosePlayerStart(ControllersToRespawn[k]);
			const FVector Location = RespawnActor->GetActorLocation();
			const FRotator Rotation = RespawnActor->GetActorRotation();
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			APawn* Player = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, Location, Rotation, SpawnParameters);
			if (Player)
			{
				ControllersToRespawn[k]->Possess(Player);
			}

			ControllersToRespawn.RemoveAt(k);
		}
	}
}

void ACustomGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	ExitedControllers.Add(Exiting);
}