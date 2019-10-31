// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MyProject2Character.h"
#include "PickUp.h"
#include "Bullet.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/Public/EngineUtils.h"
#include "DebugHUD.h"

//////////////////////////////////////////////////////////////////////////
// AMyProject2Character

void AMyProject2Character::AddDebugMessage(const FString & Text, const FColor & Color, float Duration, UWorld * World)
{
	ADebugHUD * DebugHUD = Cast<ADebugHUD>(World->GetFirstPlayerController()->GetHUD());
	if (DebugHUD != NULL)
		DebugHUD->AddText(Text, Color, Duration);
}

AMyProject2Character::AMyProject2Character()
{
	UProjectileMovementComponent;
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	//FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); 
	FollowCamera->SetupAttachment(Cast<USceneComponent>(GetMesh()), FName(TEXT("head")));
	FollowCamera->bUsePawnControlRotation = true;

	//Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	//Sphere->SetupAttachment(RootComponent);
}

void AMyProject2Character::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Shoot", IE_Pressed, this, &AMyProject2Character::Shoot);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMyProject2Character::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyProject2Character::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMyProject2Character::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMyProject2Character::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AMyProject2Character::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AMyProject2Character::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AMyProject2Character::OnResetVR);
}

void AMyProject2Character::Shoot()
{
	if (BulletClass != NULL)
	{
		if (UWorld * World = GetWorld())
		{
			FRotator Rotation = GetControlRotation();
			FVector Location = GetActorLocation();

			ABullet * Bullet = World->SpawnActor<ABullet>(BulletClass, Location, Rotation);
			Bullet->IgnoredCollisionActor = this;

			AddDebugMessage(TEXT("Fire"), FColor::Red, 0.5f, GetWorld());
		}
	}
}

void AMyProject2Character::BeginPlay()
{
	Super::BeginPlay();
}

void AMyProject2Character::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AMyProject2Character::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void AMyProject2Character::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void AMyProject2Character::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMyProject2Character::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMyProject2Character::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AMyProject2Character::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}
