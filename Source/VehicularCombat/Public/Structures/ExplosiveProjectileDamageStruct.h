// Copyright 2022 Danial Kamali. All Rights Reserved.

#pragma once

#include "Engine/DataTable.h"
#include "ExplosiveProjectileDamageStruct.generated.h"

/** Projectile info that projectile class needs */
USTRUCT(BlueprintType)
struct FExplosiveProjectileDamage : public FTableRowBase
{
	GENERATED_BODY()

	FORCEINLINE FExplosiveProjectileDamage();

	explicit FORCEINLINE FExplosiveProjectileDamage(float InBaseDamage, float InMinimumDamage, float InDamageInnerRadius, float InDamageOuterRadius);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Structs")
	float BaseDamage = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Structs")
	float MinimumDamage = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Structs")
	float DamageInnerRadius = 150.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Structs")
	float DamageOuterRadius = 500.0f;
};

FORCEINLINE FExplosiveProjectileDamage::FExplosiveProjectileDamage()
{
}

FORCEINLINE FExplosiveProjectileDamage::FExplosiveProjectileDamage(const float InBaseDamage, const float InMinimumDamage,
	const float InDamageInnerRadius, const float InDamageOuterRadius) : BaseDamage(InBaseDamage), MinimumDamage(InMinimumDamage),
	DamageInnerRadius(InDamageInnerRadius), DamageOuterRadius(InDamageOuterRadius)
{
}