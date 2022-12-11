// Copyright 2022 Danial Kamali. All Rights Reserved.

#pragma once

#include "Engine/DataTable.h"
#include "ProjectileDamageStruct.generated.h"

/** Projectile info that projectile class needs */
USTRUCT(BlueprintType)
struct FProjectileDamage : public FTableRowBase
{
	GENERATED_BODY()

	FORCEINLINE FProjectileDamage();

	explicit FORCEINLINE FProjectileDamage(float InDefaultDamage, float InDamageToMetal, float InDamageToPlastic, float InDamageToGlass);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Structs")
	float DefaultDamage = 75.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Structs")
	float DamageToMetal = 75.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Structs")
	float DamageToPlastic = 110.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Structs")
	float DamageToGlass = 150.0f;
};

FORCEINLINE FProjectileDamage::FProjectileDamage()
{
}

FORCEINLINE FProjectileDamage::FProjectileDamage(const float InDefaultDamage, const float InDamageToMetal, const float InDamageToPlastic,
	const float InDamageToGlass) : DefaultDamage(InDefaultDamage) ,DamageToMetal(InDamageToMetal), DamageToPlastic(InDamageToPlastic),
	DamageToGlass(InDamageToGlass)
{
}