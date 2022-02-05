// Copyright 2022 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Pawns/BaseWheeledVehiclePawn.h"
#include "PlayerWheeledVehiclePawn.generated.h"

class USpringArmComponent;
class UCameraComponent;

PRAGMA_DISABLE_DEPRECATION_WARNINGS

UCLASS()
class VEHICULARCOMBAT_API APlayerWheeledVehiclePawn : public ABaseWheeledVehiclePawn
{
	GENERATED_BODY()

	/** Spring arm that will offset the camera */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArmOutside;

	/** Camera component that will be our viewpoint */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* CameraOutside;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArmInside;

	/** Camera component for the In-Car view */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* CameraInside;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* WidgetDashboard;

	/** Reset controller rotation timeline */
	class UTimelineComponent* ResetRotationTimeline;

// Functions
public:
	APlayerWheeledVehiclePawn();
	
protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	virtual void Destroyed() override;

private:
	/** Handle pressing forwards */
	void MoveForward(float Val);

	/** Handle pressing right */
	void MoveRight(float Val);

	void LookUp(float Value);
	void LookRight(float Value);

	UFUNCTION()
	void PlayResetTimeline();

	/** Reset control rotation with timeline */
	UFUNCTION()
	void ResetControlRotation(float Value);

	/** Set Use Pawn Control Rotation to false after timeline finished */
	UFUNCTION()
	void SetUseRotation();
	
	/** Handle handbrake pressed */
	void OnHandbrakePressed();
	
	/** Handle handbrake released */
	void OnHandbrakeReleased();
	
	/** Switch between cameras */
	void ToggleCamera();

	UFUNCTION(Server, Reliable)
	void ServerToggleCamera();
	void ServerToggleCamera_Implementation();

	UFUNCTION(Client, Reliable)
	void ClientUpdateCurrentCamera(bool bInCar);
	void ClientUpdateCurrentCamera_Implementation(bool bInCar);

	void StartFireWeapon();
	void StopFireWeapon();

	/** Add recoil to player crosshair */
	void AddRecoil();

	virtual void OnRep_CurrentWeapon() override;

	/** Update the gear and speed */
	void UpdateUI() const;

	virtual void ClientUpdateHealth_Implementation(float NewHealth) override;

// Variables
private:
	UPROPERTY(EditDefaultsOnly, Category = "Defaults")
	TSubclassOf<AWeaponPickupActor> PrimaryWeaponToAdd;

	UPROPERTY(EditDefaultsOnly, Category = "Defaults")
	TSubclassOf<AWeaponPickupActor> SecondaryWeaponToAdd;

	UPROPERTY(EditDefaultsOnly, Category = "Defaults", meta = (ToolTip = "Automatically moves the camera to its default location", AllowPrivateAccess = true))
	uint8 bAutoAdjustCamera : 1;
	
	UPROPERTY(EditDefaultsOnly, Category = "Defaults", meta = (ToolTip = "Enable it if the vehicle has no roof and the dashboard is always visible.", AllowPrivateAccess = true))
	uint8 bAlwaysUpdateDashboard : 1;
	
	UPROPERTY(EditDefaultsOnly, Category = "Defaults")
	UCurveFloat* ResetRotationCurve;
	
	/** Are we using In-Car camera */
	UPROPERTY(Replicated)
	bool bInCarCamera;

	uint8 bDoOnceResetRotation : 1;

	/** Can the player look left and right? Used to limit the player's field of view inside the car. */
	uint8 bCanLookRight : 1;
	
	/** Initial offset of inCar camera */
	FVector InternalCameraOrigin;

	float LookUpValue, LookRightValue;

	UPROPERTY()
	APlayerController* PlayerControllerRef;

	UPROPERTY()
	class APlayerHUD* PlayerHUDRef;

	UPROPERTY()
	class UInCarWidget* WidgetRef;

	UPROPERTY()
	APlayerCameraManager* CameraManager;

	FTimerHandle ResetRotationTimer, RecoilTimer;
};

PRAGMA_ENABLE_DEPRECATION_WARNINGS