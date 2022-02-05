// Copyright 2022 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InCarWidget.generated.h"

UCLASS()
class VEHICULARCOMBAT_API UInCarWidget : public UUserWidget
{
	GENERATED_BODY()

// Functions
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PlayerHUD")
	void UpdateSpeedAndGear(int32 Speed, int32 Gear);
};