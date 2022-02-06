// Copyright 2022 Danial Kamali. All Rights Reserved.

#include "Pawns/PlayerWheeledVehiclePawn.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Actors/WeaponPickupActor.h"
#include "Camera/CameraComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/TimelineComponent.h"
#include "Components/WidgetComponent.h"
#include "Core/CustomGameMode.h"
#include "Core/CustomPlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "UI/InCarWidget.h"
#include "UI/PlayerHUD.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

APlayerWheeledVehiclePawn::APlayerWheeledVehiclePawn()
{
	// Create a spring arm component for our chase camera
	SpringArmOutside = CreateDefaultSubobject<USpringArmComponent>(TEXT("Outside Spring Arm"));
	SpringArmOutside->SetRelativeLocation(FVector(0.0f, 0.0f, 35.0f));
	SpringArmOutside->SetupAttachment(RootComponent);
	SpringArmOutside->SetComponentTickEnabled(false);
	SpringArmOutside->SocketOffset = FVector(0.0f, 0.0f, 60.0f);
	SpringArmOutside->TargetArmLength = 250.0f;
	SpringArmOutside->bUsePawnControlRotation = false;
	SpringArmOutside->bInheritPitch = true;
	SpringArmOutside->bInheritYaw = true;
	SpringArmOutside->bInheritRoll = false;
	SpringArmOutside->bEnableCameraLag = true;
	SpringArmOutside->CameraLagMaxDistance = 50.0f;

	// Create the chase camera component 
	CameraOutside = CreateDefaultSubobject<UCameraComponent>(TEXT("Chase Camera"));
	CameraOutside->SetupAttachment(SpringArmOutside, USpringArmComponent::SocketName);
	CameraOutside->SetRelativeRotation(FRotator(-15.0f, 0.0f, 0.0f));
	CameraOutside->SetComponentTickEnabled(false);
	CameraOutside->SetIsReplicated(true);
	CameraOutside->bUsePawnControlRotation = false;

	SpringArmInside = CreateDefaultSubobject<USpringArmComponent>(TEXT("Inside Spring Arm"));
	SpringArmInside->SetRelativeLocation(FVector(0.0f, 0.0f, 40.0f));
	SpringArmInside->SetupAttachment(RootComponent);
	SpringArmInside->SetComponentTickEnabled(false);
	SpringArmInside->SocketOffset = FVector(0.0f, 0.0f, 5.0f);
	SpringArmInside->TargetArmLength = 25.0f;
	SpringArmInside->bUsePawnControlRotation = false;
	SpringArmInside->bInheritPitch = true;
	SpringArmInside->bInheritYaw = true;
	SpringArmInside->bInheritRoll = false;
	SpringArmInside->bEnableCameraLag = true;
	SpringArmInside->bEnableCameraRotationLag = true;
	SpringArmInside->CameraLagSpeed = 5.0f;
	SpringArmInside->CameraRotationLagSpeed = 20.0f;
	SpringArmInside->CameraLagMaxDistance = 2.5f;

	CameraInside = CreateDefaultSubobject<UCameraComponent>(TEXT("Internal Camera"));
	CameraInside->SetupAttachment(SpringArmInside, USpringArmComponent::SocketName);
	CameraInside->SetRelativeRotation(FRotator(-5.0f, 0.0f, 0.0f));
	CameraInside->SetComponentTickEnabled(false);
	CameraInside->SetIsReplicated(true);
	CameraInside->bUsePawnControlRotation = false;

	WidgetDashboard = CreateDefaultSubobject<UWidgetComponent>(TEXT("Dashboard Widget"));
	WidgetDashboard->SetupAttachment(GetMesh());
	WidgetDashboard->SetComponentTickEnabled(false);
	WidgetDashboard->SetRelativeLocation(FVector(35.0f, 0.0f, 20.0f));
	WidgetDashboard->SetRelativeRotation(FRotator(0.0f, -180.0f, 0.0f));
	WidgetDashboard->SetRelativeScale3D(FVector(0.05f, 0.05f, 0.05f));
	WidgetDashboard->SetDrawSize(FVector2D(300.0f, 150.0f));
	WidgetDashboard->SetGenerateOverlapEvents(false);
	WidgetDashboard->CanCharacterStepUpOn = ECB_No;

	ResetRotationTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Reset Rotation Timeline"));

	// Initialize variables
	LookRightValue = LookUpValue = 0.0f;
	bAutoAdjustCamera = true;
	bInCarCamera = false;
	bAlwaysUpdateDashboard = false;
	bDoOnceResetRotation = true;
	bCanToggleCamera = true;
	ToggleCameraCooldown = 0.3f;
}

void APlayerWheeledVehiclePawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate to everyone
	DOREPLIFETIME(APlayerWheeledVehiclePawn, bInCarCamera);
}

void APlayerWheeledVehiclePawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerWheeledVehiclePawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerWheeledVehiclePawn::MoveRight);
	
	PlayerInputComponent->BindAxis("LookUp", this, &APlayerWheeledVehiclePawn::LookUp);
	PlayerInputComponent->BindAxis("LookRight", this, &APlayerWheeledVehiclePawn::LookRight);
	
	PlayerInputComponent->BindAction("FireWeapon", IE_Pressed, this, &APlayerWheeledVehiclePawn::StartFireWeapon);
	PlayerInputComponent->BindAction("FireWeapon", IE_Released, this, &APlayerWheeledVehiclePawn::StopFireWeapon);

	PlayerInputComponent->BindAction("Handbrake", IE_Pressed, this, &APlayerWheeledVehiclePawn::OnHandbrakePressed);
	PlayerInputComponent->BindAction("Handbrake", IE_Released, this, &APlayerWheeledVehiclePawn::OnHandbrakeReleased);
	
	PlayerInputComponent->BindAction("SwitchCamera", IE_Pressed, this, &APlayerWheeledVehiclePawn::ToggleCamera);

	PlayerInputComponent->BindAction("SwitchToPrimary", IE_Pressed, this, &APlayerWheeledVehiclePawn::SwitchToPrimary);
	PlayerInputComponent->BindAction("SwitchToSecondary", IE_Pressed, this, &APlayerWheeledVehiclePawn::SwitchToSecondary);
}

void APlayerWheeledVehiclePawn::BeginPlay()
{
	Super::BeginPlay();

	CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	CameraManager->ViewPitchMax = 25.0f;
	CameraManager->ViewPitchMin = -20.0f;

	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		if (bAutoAdjustCamera == false)
		{
			SpringArmOutside->bUsePawnControlRotation = true;
			// SpringArmInside->bUsePawnControlRotation = true;	// TODO - Need to be fixed
		}

		PlayerControllerRef = Cast<APlayerController>(GetController());
		if (PlayerControllerRef)
		{
			PlayerHUDRef = Cast<APlayerHUD>(PlayerControllerRef->GetHUD());
			if (PlayerHUDRef)
			{
				PlayerHUDRef->Initialize();
			}
		}

		WidgetRef = Cast<UInCarWidget>(WidgetDashboard->GetWidget());

		if (ResetRotationCurve)
		{
			FOnTimelineFloat TimelineProgress{};
			TimelineProgress.BindUFunction(this, FName("ResetControlRotation"));
			ResetRotationTimeline->AddInterpFloat(ResetRotationCurve, TimelineProgress, FName("Alpha"));
			FOnTimelineEvent TimelineFinishEvent{};
			TimelineFinishEvent.BindUFunction(this, FName("SetUseRotation"));
			ResetRotationTimeline->SetTimelineFinishedFunc(TimelineFinishEvent);
		}
	}
}

void APlayerWheeledVehiclePawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = GetInstigator();
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
	if (PrimaryWeaponToAdd)
	{
		AWeaponPickupActor* Weapon1 = GetWorld()->SpawnActor<AWeaponPickupActor>(PrimaryWeaponToAdd, GetActorLocation(), GetActorRotation(), SpawnParameters);
		if (Weapon1)
		{
			ServerAddWeapon(Weapon1);
		}
	}

	if (SecondaryWeaponToAdd)
	{
		AWeaponPickupActor* Weapon2 = GetWorld()->SpawnActor<AWeaponPickupActor>(PrimaryWeaponToAdd, GetActorLocation(), GetActorRotation(), SpawnParameters);
		if (Weapon2)
		{
			ServerAddWeapon(Weapon2);
		}
	}
}

void APlayerWheeledVehiclePawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (CurrentWeapon && GetLocalRole() == ROLE_Authority)
	{
		if (bInCarCamera == false)	// TODO - Add ease
		{
			const FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(CurrentWeapon->GetActorLocation(), UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetCameraLocation());
			CurrentWeapon->SetActorRotation(FRotator(0.0f, Rotation.Yaw + 90.0f, Rotation.Pitch));
		}
	}
	
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		if (bAutoAdjustCamera)
		{
			if (bDoOnceResetRotation && LookUpValue == 0.0f && LookRightValue == 0.0f)
			{
				bDoOnceResetRotation = false;
				GetWorld()->GetTimerManager().SetTimer(ResetRotationTimer, this, &APlayerWheeledVehiclePawn::PlayResetTimeline, 1.0f);
			}
			else if (bDoOnceResetRotation != true && (LookUpValue != 0.0f || LookRightValue != 0.0f))
			{
				PlayerControllerRef->SetControlRotation(SpringArmOutside->GetComponentRotation());
				SpringArmOutside->bUsePawnControlRotation = true;
				GetWorld()->GetTimerManager().ClearTimer(ResetRotationTimer);
				ResetRotationTimeline->Stop();
				bDoOnceResetRotation = true;
			}
		}

		UpdateUI();
	}
}

void APlayerWheeledVehiclePawn::MoveForward(float Val)
{
	if (Val > 0)
	{
		GetVehicleMovementComponent()->SetThrottleInput(Val);
		GetVehicleMovementComponent()->SetBrakeInput(0.0f);
		GetVehicleMovementComponent()->UpdatedPrimitive->WakeAllRigidBodies();
	}
	else
	{
		GetVehicleMovementComponent()->SetThrottleInput(0.0f);
		GetVehicleMovementComponent()->SetBrakeInput(-Val);
		GetVehicleMovementComponent()->UpdatedPrimitive->WakeAllRigidBodies();
	}
}

void APlayerWheeledVehiclePawn::MoveRight(float Val)
{
	GetVehicleMovementComponent()->SetSteeringInput(Val);
}

void APlayerWheeledVehiclePawn::LookUp(float Value)
{
	LookUpValue = Value;
	AddControllerPitchInput(Value);
}

void APlayerWheeledVehiclePawn::LookRight(float Value)
{
	// if (bInCarCamera)	// TODO - Oh No, not math again
	// {
	// 	// clamp view yaw between -50 and 50
	// 	float ControlYaw = PlayerControllerRef->GetControlRotation().Yaw;
	// 	if (ControlYaw > 180.0f)
	// 	{
	// 		ControlYaw = ControlYaw - 360.0f;
	// 	}
	//
	// 	const float ActorYaw = GetActorRotation().Yaw;
	// 	if (ActorYaw > ControlYaw - (50.0f - Value) && ActorYaw < ControlYaw + (50.0f - Value))
	// 	{
	// 		LookRightValue = Value;
	// 		AddControllerYawInput(Value);
	// 	}
	//
	// 	if (ActorYaw > ControlYaw - 60.0f && ActorYaw < ControlYaw + 60.0f)
	// 	{
	// 		SpringArmInside->bUsePawnControlRotation = false;
	// 	}
	// }
	// else
	// {
		LookRightValue = Value;
		AddControllerYawInput(Value);
	// }
}

void APlayerWheeledVehiclePawn::StartFireWeapon()
{
	if (CanFireWeapon())
	{
		AddRecoil();
		ServerStartFireWeapon();

		if (CurrentWeapon->bIsAutomatic)
		{
			GetWorld()->GetTimerManager().SetTimer(RecoilTimer, this, &APlayerWheeledVehiclePawn::AddRecoil, CurrentWeapon->TimeBetweenShots, true);
		}
	}
}

void APlayerWheeledVehiclePawn::StopFireWeapon()
{
	if (CurrentWeapon)
	{
		GetWorld()->GetTimerManager().ClearTimer(RecoilTimer);
		ServerStopFireWeapon();
	}
}

void APlayerWheeledVehiclePawn::ClientUpdateWeaponState_Implementation(EWeaponState WeaponState)
{
	if (PlayerHUDRef)
	{
		PlayerHUDRef->UpdateWeaponState(WeaponState);
	}
}

void APlayerWheeledVehiclePawn::ClientUpdateAmmo_Implementation(int32 CurrentMagAmmo)
{
	if (PlayerHUDRef)
	{
		PlayerHUDRef->UpdateAmmo(CurrentMagAmmo);
	}
}

void APlayerWheeledVehiclePawn::AddRecoil()
{
	if (CanFireWeapon())
	{
		PlayerControllerRef->ClientStartCameraShake(CurrentWeapon->Effects.CameraShake);
		PlayerHUDRef->AddCrosshairRecoil(CurrentWeapon->RecoilData.CrosshairRecoil, CurrentWeapon->RecoilData.ControlTime);
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(RecoilTimer);
	}
}

void APlayerWheeledVehiclePawn::PlayResetTimeline()
{
	ResetRotationTimeline->PlayFromStart();
}

void APlayerWheeledVehiclePawn::ResetControlRotation(float Value)
{
	PlayerControllerRef->SetControlRotation(UKismetMathLibrary::RLerp(PlayerControllerRef->GetControlRotation(), GetActorRotation(), Value, true));
}

void APlayerWheeledVehiclePawn::SetUseRotation()
{
	SpringArmOutside->bUsePawnControlRotation = false;
}

void APlayerWheeledVehiclePawn::OnHandbrakePressed()
{
	GetVehicleMovementComponent()->SetHandbrakeInput(true);
}

void APlayerWheeledVehiclePawn::OnHandbrakeReleased()
{
	GetVehicleMovementComponent()->SetHandbrakeInput(false);
}

void APlayerWheeledVehiclePawn::OnRep_CurrentWeapon()
{
	if (PlayerHUDRef)
	{
		PlayerHUDRef->UpdateWeaponState(EWeaponState::Idle);

		if (CurrentWeapon)
		{
			PlayerHUDRef->UpdateCurrentWeapon(CurrentWeapon->WeaponName);
			PlayerHUDRef->UpdateAmmo(CurrentWeapon->CurrentMagazineAmmo);
			
			if (CurrentWeapon->CurrentMagazineAmmo <= 0)
			{
				PlayerHUDRef->UpdateWeaponState(EWeaponState::NoAmmo);
			}
		}
		else
		{
			PlayerHUDRef->UpdateCurrentWeapon(EWeaponName::NoName);
		}
	}
}

void APlayerWheeledVehiclePawn::SwitchToPrimary()
{
	if (CurrentWeaponSlot != EWeaponToDo::Primary && PlayerStateRef->PrimaryWeapon)
	{
		ServerSwitchWeapon(EWeaponToDo::Primary);
	}
}

void APlayerWheeledVehiclePawn::SwitchToSecondary()
{
	if (CurrentWeaponSlot != EWeaponToDo::Secondary && PlayerStateRef->SecondaryWeapon)
	{
		ServerSwitchWeapon(EWeaponToDo::Secondary);
	}
}

void APlayerWheeledVehiclePawn::ToggleCamera()
{
	if (bCanToggleCamera)
	{
		ServerToggleCamera();

		bCanToggleCamera = false;
		GetWorld()->GetTimerManager().SetTimer(ToggleCameraTimer, this, &APlayerWheeledVehiclePawn::ResetToggleCamera, ToggleCameraCooldown);
	}
}

void APlayerWheeledVehiclePawn::ResetToggleCamera()
{
	bCanToggleCamera = true;
}

void APlayerWheeledVehiclePawn::ServerToggleCamera_Implementation()
{
	bInCarCamera = !bInCarCamera;
	if (bInCarCamera)
	{
		if (CurrentWeapon && CurrentWeaponSlot != EWeaponToDo::NoWeapon)
		{
			CurrentWeapon->SetActorRelativeRotation(FRotator::ZeroRotator);
		}
		
		ClientUpdateCurrentCamera(true);
		// CameraManager->ViewYawMax = 50.0f;
		// CameraManager->ViewYawMin = -50.0f;
		CameraManager->ViewPitchMax = 20.0f;
		CameraManager->ViewPitchMin = -20.0f;
	}
	else
	{
		ClientUpdateCurrentCamera(false);
		// CameraManager->ViewYawMax = 359.99f;
		// CameraManager->ViewYawMin = 0.0f;
		CameraManager->ViewPitchMax = 25.0f;
		CameraManager->ViewPitchMin = -20.0f;
	}
}

void APlayerWheeledVehiclePawn::ClientUpdateCurrentCamera_Implementation(bool bInCar)
{
	if (bInCar)
	{
		if (PlayerHUDRef)
		{
			PlayerHUDRef->UpdateUI(true);
		}
		
		CameraOutside->Deactivate();
		CameraInside->Activate();
		CameraManager->ViewPitchMax = 20.0f;
		CameraManager->ViewPitchMin = -20.0f;
	}
	else
	{
		CameraInside->Deactivate();
		CameraOutside->Activate();
		CameraManager->ViewPitchMax = 25.0f;
		CameraManager->ViewPitchMin = -20.0f;
		
		if (PlayerHUDRef)
		{
			PlayerHUDRef->UpdateUI(false);
		}
	}
}

void APlayerWheeledVehiclePawn::UpdateUI() const
{
	const float KPH = FMath::Abs(GetVehicleMovement()->GetForwardSpeed()) * 0.036f;
	if (bAlwaysUpdateDashboard || bInCarCamera)
	{
		WidgetRef->UpdateSpeedAndGear(FMath::FloorToInt(KPH), GetVehicleMovement()->GetCurrentGear());
	}
	if (bInCarCamera == false)
	{
		PlayerHUDRef->UpdateSpeedAndGear(FMath::FloorToInt(KPH), GetVehicleMovement()->GetCurrentGear());
	}
}

void APlayerWheeledVehiclePawn::ClientUpdateHealth_Implementation(float NewHealth)
{
	if (PlayerHUDRef)
	{
		PlayerHUDRef->UpdateHealth(NewHealth);
	}
}

void APlayerWheeledVehiclePawn::Destroyed()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		// In case of player leave the session without getting killed
		if (bDoOnceDeath)
		{
			bDoOnceDeath = false;
		}

		// Get player controller reference before destroying the player
		AController* RespawnControllerRef = GetController();
	
		Super::Destroyed();

		// Get the World and GameMode in the world to invoke its restart player function.
		ACustomGameMode* GameMode = Cast<ACustomGameMode>(GetWorld()->GetAuthGameMode());
		if (GameMode)
		{
			GameMode->ServerStartRespawn(RespawnControllerRef);
		}
	}
}

PRAGMA_ENABLE_DEPRECATION_WARNINGS