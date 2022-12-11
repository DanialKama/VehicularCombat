// Copyright 2022 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Enumerations/ActorEnums.h"
#include "ProjectileActor.generated.h"

class USoundCue;
class UDataTable;

USTRUCT(BlueprintType)
struct FHitEffects
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* MetalHitEmitter;

	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* PlasticHitEmitter;
	
	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* GlassHitEmitter;

	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* AsphaltHitEmitter;

	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* StoneHitEmitter;
		
	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* WoodHitEmitter;
	
	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* ExplosiveEmitter;
	
	UPROPERTY(EditDefaultsOnly)
	USoundCue* ObjectHitSound;

	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* MetalDecal;

	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* PlasticDecal;

	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* GlassDecal;

	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* AsphaltDecal;
	
	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* StoneDecal;
	
	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* WoodDecal;
	
	UPROPERTY(EditDefaultsOnly)
	FVector DecalSize;
	
	UPROPERTY(EditDefaultsOnly)
	float DecalLifeSpan;

	// Default constructor
	FHitEffects()
	{
		MetalHitEmitter = nullptr;
		PlasticHitEmitter = nullptr;
		GlassHitEmitter = nullptr;
		AsphaltHitEmitter = nullptr;
		StoneHitEmitter = nullptr;
		WoodHitEmitter = nullptr;
		ExplosiveEmitter = nullptr;
		ObjectHitSound = nullptr;
		MetalDecal = nullptr;
		PlasticDecal = nullptr;
		GlassDecal = nullptr;
		AsphaltDecal = nullptr;
		StoneDecal = nullptr;
		WoodDecal = nullptr;
		DecalSize = FVector(5.0f, 10.0f, 10.0f);
		DecalLifeSpan = 10.0f;
	}
};

UCLASS()
class AProjectileActor : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UParticleSystemComponent* TrailParticle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UProjectileMovementComponent* ProjectileMovement;

// Functions
public:
	AProjectileActor();

protected:
	virtual void BeginPlay() override;
	virtual void LifeSpanExpired() override;

private:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastHitEffects(uint8 InSwitchExpression, FHitResult HitResult);
	void MulticastHitEffects_Implementation(uint8 InSwitchExpression, FHitResult HitResult);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDestroyEffect(FVector Location, FRotator Rotation);
	void MulticastDestroyEffect_Implementation(FVector Location, FRotator Rotation);

	/** Calculating the point damage that needs to be applied based on the surface type. */
	float CalculatePointDamage(const struct FProjectileDamage* ProjectileInfo) const;

	/** Returns effects that need to spawn based on the surface type. */
	void FindHitEffects(uint8 InSwitchExpression, UParticleSystem*& Emitter, USoundCue*& Sound, UMaterialInterface*& Decal, FVector& DecalSize, float& DecalLifeSpan) const;

// Variables
public:
	UPROPERTY(EditDefaultsOnly, Category = "Defaults", meta = (ClampMin = "1", UIMin = "1", AllowPrivateAccess = true))
	int32 NumberOfPellets;

	UPROPERTY(EditDefaultsOnly, Category = "Defaults", meta = (ClampMin = "0", UIMin = "0", AllowPrivateAccess = true))
	int32 PelletSpread;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Defaults", meta = (AllowPrivateAccess = true))
	EAmmoType AmmoType;

	UPROPERTY(EditDefaultsOnly, Category = "Defaults", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly, Category = "Defaults", meta = (AllowPrivateAccess = true))
	uint8 bIsExplosive : 1;
	
	UPROPERTY(EditDefaultsOnly, Category = "Defaults", meta = (ClampMin = "1", UIMin = "1", AllowPrivateAccess = true))
	float LifeSpan;

	UPROPERTY(EditDefaultsOnly, Category = "Defaults", meta = (ToolTip = "Non-explosive projectiles", AllowPrivateAccess = true))
	UDataTable* ProjectileDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "Defaults", meta = (AllowPrivateAccess = true))
	UDataTable* ExplosiveProjectileDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "Defaults", meta = (AllowPrivateAccess = "true"))
	FHitEffects HitEffects;

	/** To use in all functions that switch on the surface type */
	uint8 SwitchExpression;
};