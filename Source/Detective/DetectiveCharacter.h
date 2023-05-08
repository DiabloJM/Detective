// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "PickupAndRotate.h"
#include "DetectiveCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UAnimMontage;
class USoundBase;
class APickupAndRotateActor; 

UCLASS(config=Game)
class ADetectiveCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;
	
	/** Holding Component */
	UPROPERTY(EditAnywhere)
	class USceneComponent* HoldingComponent; // Holding component for the player

	
public:
	ADetectiveCharacter();

protected:
	virtual void BeginPlay();
	
	virtual void Tick(float DeltaSeconds) override; 
public:
		
	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	/** Bool for AnimBP to switch to another animation set */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon)
	bool bHasRifle;

	/** Setter to set the bool */
	UFUNCTION(BlueprintCallable, Category = Weapon)
	void SetHasRifle(bool bNewHasRifle);

	/** Getter for the bool */
	UFUNCTION(BlueprintCallable, Category = Weapon)
	bool GetHasRifle();

	UPROPERTY(EditAnywhere) 
	class APickupAndRotateActor* CurrentItem; // Current item the player is holding

	bool bCanMove; // Can the player move?
	bool bHoldingItem; // Is the player holding an item?
	bool bInspecting; // Is the player inspecting an item?

	float PitchMax; // Max pitch for the camera
	float PitchMin; // Min pitch for the camera

	FVector HoldingComp; // Holding component location
	FRotator LastRotation; // Last rotation of the player

	FVector Start; // Start of the line trace
	FVector ForwardVector; // Forward vector of the player
	FVector End; // End of the line trace

	FHitResult Hit; // Hit result of the line trace

	FComponentQueryParams DefaultComponentQueryParams; // Default component query params
	FCollisionResponseParams DefaultResponseParams; // Default response params

protected:

	/** Action Function */
	void OnAction(); // Action button pressed

	/** Inspect Function */
	void OnInspect(); // Inspect an item
	void OnInspectReleased(); // Stop inspecting an item
	
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	// toggle player movement
	void ToggleMovement(); // Toggle player movement

	// toggle holding item pickup
	void ToggleItemPickup(); // Toggle item pickup

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

private:
	ADetectiveCharacter* PlayerCharacter; // Player character


};

