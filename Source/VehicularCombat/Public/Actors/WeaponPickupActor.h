// Copyright 2022 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/PickupActor.h"
#include "WeaponPickupActor.generated.h"

class USoundCue;
class AProjectileActor;

USTRUCT(BlueprintType)
struct FEffects
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* MuzzleFlashRear;

	UPROPERTY(EditDefaultsOnly)
	FVector MuzzleFlashScale;

	UPROPERTY(EditDefaultsOnly)
	USoundCue* FireSound;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCameraShakeBase> CameraShake;

	// Default constructor
	FEffects()
	{
		MuzzleFlash = nullptr;
		MuzzleFlashRear = nullptr;
		MuzzleFlashScale = FVector::OneVector;
		FireSound = nullptr;
	}
};

USTRUCT(BlueprintType)
struct FRecoilData
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, meta = (ToolTip = "Smaller number = more intensity"))
	FRotator RotationIntensity;
	
	UPROPERTY(EditDefaultsOnly, meta = (ToolTip = "Bigger number = faster control"))
	float ControlTime;
	
	UPROPERTY(EditDefaultsOnly, meta = (ToolTip = "Bigger number = more fedback"))
	float CrosshairRecoil;
	
	UPROPERTY(EditDefaultsOnly, meta = (ToolTip = "Smaller number = more fedback"))
	float ControllerPitch;

	// Default constructor
	FRecoilData()
	{
		RotationIntensity = FRotator(0.0f, 0.0f, -5.0f);
		ControlTime = 0.25f;
		CrosshairRecoil = 10.0f;
		ControllerPitch = -0.5f;
	}
};

UCLASS()
class VEHICULARCOMBAT_API AWeaponPickupActor : public APickupActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* SkeletalMesh;
	
// Functions
public:
	/** Sets default values for this actor's properties */
	AWeaponPickupActor();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSpawnProjectile(FTransform NewTransform);
	
	virtual void OnRep_PickupState() override;

protected:
	virtual void BeginPlay() override;
	
private:
	bool ServerSpawnProjectile_Validate(FTransform NewTransform);
	void ServerSpawnProjectile_Implementation(FTransform NewTransform);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpawnProjectile(TSubclassOf<AProjectileActor> ProjectileToSpawn, int32 NumberOfPellets, FVector Location, FRotator Rotation, AActor* OwnerRef);
	void MulticastSpawnProjectile_Implementation(TSubclassOf<AProjectileActor> ProjectileToSpawn, int32 NumberOfPellets, FVector Location, FRotator Rotation, AActor* OwnerRef);

	/** Calculate spawn location and rotation for the projectile. */
	FTransform ProjectileLineTrace() const;
	
	void CalculateLineTrace(FVector &Start, FVector &End) const;

	FRotator RandomPointInCircle(float Radius, bool bIncludesNegative) const;

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastWeaponEffects();
	void MulticastWeaponEffects_Implementation();

// Variables
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defaults")
	EWeaponType WeaponType;

	UPROPERTY(EditDefaultsOnly, Category = "Defaults")
	EAmmoType AmmoType;

	UPROPERTY(EditDefaultsOnly, Category = "Defaults")
	EWeaponName WeaponName;

	UPROPERTY(EditDefaultsOnly, Category = "Defaults")
	uint8 bIsAutomatic : 1;

	UPROPERTY(EditDefaultsOnly, Category = "Defaults")
	float TimeBetweenShots;

	UPROPERTY(EditDefaultsOnly, Category = "Defaults")
	float Range;

	UPROPERTY(Replicated, EditDefaultsOnly, Category = "Defaults", meta = (ClampMin = "0", UIMin = "0"))
	int32 MagazineSize;
	
	UPROPERTY(Replicated, EditAnywhere, Category = "Defaults", meta = (ToolTip = "If value set to something greater than zero then in initial value dose not change"))
	int32 CurrentMagazineAmmo;

	UPROPERTY(Replicated, EditDefaultsOnly, Category = "Defaults", meta = (ClampMin = "0", ClampMax = "999", UIMin = "0", UIMax = "999"))
	int32 ReloadAmount;

	UPROPERTY(Replicated, EditDefaultsOnly, Category = "Defaults", meta = (ClampMin = "0", UIMin = "0"))
	float ReloadTime;

	UPROPERTY(EditDefaultsOnly, Category = "Defaults")
	FRecoilData RecoilData;
	
	UPROPERTY(EditDefaultsOnly, Category = "Defaults")
	FEffects Effects;

	/** Current transform of the owner for calculating the Start and End points of projectile line trace */
	FTransform ReferenceTransform;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Defaults", meta = (AllowPrivateAccess = true))
	TSubclassOf<AProjectileActor> ProjectileClass;

	UPROPERTY()
	AProjectileActor* ProjectileRef;
};