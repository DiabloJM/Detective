// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupAndRotate.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

// Sets default values
APickupAndRotateActor::APickupAndRotateActor()
{
	MyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("My Mesh")); //create the mesh component
	MyMesh->SetSimulatePhysics(true); //add physics to the mesh
	RootComponent = MyMesh;

	bHolding = false;
	bGravity = true;
}

// Called when the game starts or when spawned
void APickupAndRotateActor::BeginPlay()
{
	Super::BeginPlay();

	MyCharacter = UGameplayStatics::GetPlayerCharacter(this, 0); //get the 1 player character in the scene
	PlayerCamera = MyCharacter->FindComponentByClass<UCameraComponent>(); //get the camera component from the player character

	TArray<USceneComponent*> Components; //create an array of scene components
 
	MyCharacter->GetComponents(Components); //get all the components attached to the player character

	if(Components.Num() > 0) //if there are components attached to the player character
	{
		for (auto& Comp : Components) //loop through the components
		{
			if(Comp->GetName() == "HoldingComponent") //if the component is the holding component
			{
				HoldingComp = Cast<USceneComponent>(Comp); //cast the component to a scene component
			}
		}
	}
}

void APickupAndRotateActor::RotateActor() //rotate the actor to face the camera
{
	ControlRotation = GetWorld()->GetFirstPlayerController()->GetControlRotation(); //get the control rotation of the player controller
	SetActorRotation(FQuat(ControlRotation)); //set the actor rotation to the player controller rotation
}

void APickupAndRotateActor::Pickup() //pickup the actor
{
	bHolding = !bHolding; //toggle the holding bool for the mesh
	bGravity = !bGravity;//toggle the gravity bool for the mesh
	MyMesh->SetEnableGravity(bGravity); //enable or disable gravity for the mesh depending on the bool
	MyMesh->SetSimulatePhysics(bHolding ? false : true); //enable or disable physics for the mesh depending on the bool
	MyMesh->SetCollisionEnabled(bHolding ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics); //enable or disable collision for the mesh depending on the bool
	if(HoldingComp && bHolding)
	{
		MyMesh->AttachToComponent(HoldingComp, FAttachmentTransformRules::KeepWorldTransform); //attach the mesh to the holding component
		SetActorLocation(HoldingComp->GetComponentLocation()); //set the actor location to the holding component location in this case the player character
	}

	if(!bHolding) //if we are not holding the object anymore
	{
		MyMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform); //detach the mesh from the holding component
		ForwardVector = PlayerCamera->GetForwardVector(); //get the forward vector of the camera
		MyMesh->AddForce(ForwardVector*10000*MyMesh->GetMass()); //add force to the mesh
	}
}
