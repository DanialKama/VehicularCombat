// Copyright 2022 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PlayerHUD.generated.h"

UCLASS()
class VEHICULARCOMBAT_API APlayerHUD : public AHUD
{
	GENERATED_BODY()

// Functions
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PlayerHUD")
	void Initialize();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PlayerHUD")
	void SetUIVisibility(ESlateVisibility Visibility);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PlayerHUD")
	void AddCrosshairRecoil(float Recoil, float ControlTime);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PlayerHUD")
	void UpdateSpeedAndGear(int32 Speed, int32 Gear);
	
	/** Update health level on player UI */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PlayerHUD")
	void UpdateHealth(float NewHealth);
};