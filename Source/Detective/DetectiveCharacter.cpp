// Copyright Epic Games, Inc. All Rights Reserved.

#include "DetectiveCharacter.h"
#include "DetectiveProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "DrawDebugHelpers.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"

//#define NULL  nullptr

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// ADetectiveCharacter

ADetectiveCharacter::ADetectiveCharacter()
{
	// Character doesnt have a rifle at start
	bHasRifle = false;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));
		
	//NEW
	HoldingComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HoldingComponent"));
	HoldingComponent->SetRelativeLocation(FVector(50,0,0)); //GetRelativeLocation.X = 50.0f;
	//const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	//HoldingComponent->AttachToComponent(PlayerCharacter->GetMesh1P(), AttachmentRules, FName(TEXT("GripPoint")));
	HoldingComponent->SetupAttachment(RootComponent);

	CurrentItem = nullptr;
	bCanMove = true;
	bInspecting = false;

}

void ADetectiveCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	//NEW
	PitchMax = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewPitchMax;
	PitchMin = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewPitchMin;

}


////////////////////////////////////////////////////////////////////////// Tick

void ADetectiveCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); // Call parent class tick function

	Start = FirstPersonCameraComponent->GetComponentLocation(); // Get Camera Location
	ForwardVector = FirstPersonCameraComponent->GetForwardVector(); // Get Camera Forward Vector
	End = ((ForwardVector * 200.f) + Start); // Calculate End Location

	//DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1, 0, 1); // Draw Debug Line

	if(!bHoldingItem) // If not holding an item
	{
		if(GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, DefaultComponentQueryParams, FCollisionResponseParams::DefaultResponseParam)) // Line Trace
		{
			if(Hit.GetActor()->GetClass()->IsChildOf(APickupAndRotateActor::StaticClass())) // If Hit Actor is a child of APickupAndRotateActor
			{				
				CurrentItem = Cast<APickupAndRotateActor>(Hit.GetActor()); // Set Current Item to the Hit Actor
				//FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 20.f)); 
			}
		}
		else
		{
			CurrentItem = nullptr; // Set Current Item to NULL
		}
	}

	if(bInspecting) // If Inspecting
	{
		if(bHoldingItem) // If Holding an Item
		{
			FirstPersonCameraComponent->SetFieldOfView(FMath::Lerp(FirstPersonCameraComponent->FieldOfView, 120.0f, 0.1f)); // Set Camera FOV to 90
			HoldingComponent->SetRelativeLocation(FVector(120.0f, 0.0f, 50.0f)); // Set Holding Component Location
			GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewPitchMax = 179.9000002f; // Set Camera Pitch Max
			GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewPitchMin = -179.9000002f; // Set Camera Pitch Min
			CurrentItem->RotateActor(); // Rotate Item
		}
		else
		{
			FirstPersonCameraComponent->SetFieldOfView(FMath::Lerp(FirstPersonCameraComponent->FieldOfView, 45.0f, 0.1f)); // Set Camera FOV to 45
		}
	}
	else  // If not Inspecting
	{
		FirstPersonCameraComponent->SetFieldOfView(FMath::Lerp(FirstPersonCameraComponent->FieldOfView, 90.0f, 0.1f)); // Set Camera FOV to 90

		if(bHoldingItem) 
		{
			HoldingComponent->SetRelativeLocation(FVector(50.0f, 0.0f, 0.f)); // Set Holding Component Location
			//HoldingComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f)); // Set Holding Component Rotation
		}
	}
}


//////////////////////////////////////////////////////////////////////////// Input

void ADetectiveCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (bCanMove)
		{
				
			//Jumping
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	
			//Moving
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADetectiveCharacter::Move);
			
	
			//Looking
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADetectiveCharacter::Look);
		}
	}
	PlayerInputComponent->BindAction("Action", IE_Pressed, this, &ADetectiveCharacter::OnAction); // Bind Action Input
	PlayerInputComponent->BindAction("Inspect", IE_Pressed, this, &ADetectiveCharacter::OnInspect); // Bind Inspect Input
	PlayerInputComponent->BindAction("Inspect", IE_Released, this, &ADetectiveCharacter::OnInspectReleased); // Bind Inspect Released Input
}


void ADetectiveCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void ADetectiveCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ADetectiveCharacter::SetHasRifle(bool bNewHasRifle)
{
	bHasRifle = bNewHasRifle;
}

bool ADetectiveCharacter::GetHasRifle()
{
	return bHasRifle;
}

void ADetectiveCharacter::OnAction() // Action Button Pressed
{
	if(CurrentItem && !bInspecting) // If there is a Current Item and not Inspecting
	{
		ToggleItemPickup(); // Toggle Item Pickup
	}	
}

void ADetectiveCharacter::OnInspect() // Inspect Button Pressed
{
	if(bHoldingItem) // If Holding an Item
	{
		LastRotation = GetControlRotation(); // Set Last Rotation
		ToggleMovement(); // Toggle Movement
	}
	else // If not Holding an Item
	{
		bInspecting = true; // Set Inspecting to true
	}
}

void ADetectiveCharacter::OnInspectReleased() // Inspect Button Released
{
	if (bInspecting && bHoldingItem) // If Inspecting and Holding an Item
	{
		GetController()->SetControlRotation(LastRotation); // Set Control Rotation to Last Rotation
		GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewPitchMax = PitchMax; // Set Camera Pitch Max
		GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewPitchMin = PitchMin; // Set Camera Pitch Min
		ToggleMovement(); // Toggle Movement
	}
	else 
	{
		bInspecting = false; // Set Inspecting to false
	}
}

void ADetectiveCharacter::ToggleMovement() // Toggle Movement
{
	bCanMove = !bCanMove;
	bInspecting = !bInspecting;
	FirstPersonCameraComponent->bUsePawnControlRotation = !FirstPersonCameraComponent->bUsePawnControlRotation; // Toggle Camera Rotation
	bUseControllerRotationYaw = !bUseControllerRotationYaw; // Toggle Controller Rotation
}

void ADetectiveCharacter::ToggleItemPickup() // Toggle Item Pickup
{
	if(CurrentItem) // If there is a Current Item
	{
		bHoldingItem = !bHoldingItem;
		CurrentItem->Pickup(); // Pickup Item

		if(!bHoldingItem) // If not Holding an Item
		{
			CurrentItem = nullptr; // Set Current Item to NULL
		}
	}
}