// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "FirstPersonCharacter.h"
#include "Bullet.h"
#include "DebugHUD.h"

#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

#include "GameFramework/InputSettings.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/Public/EngineUtils.h"
#include "Engine/Public/TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"



DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

void AFirstPersonCharacter::AddDebugMessage(const FString & Text, const FColor & Color, float Duration, UWorld * World)
{
	ADebugHUD * DebugHUD = Cast<ADebugHUD>(World->GetFirstPlayerController()->GetHUD());
	if (DebugHUD != NULL)
		DebugHUD->AddText(Text, Color, Duration);
}

AFirstPersonCharacter::AFirstPersonCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	GetCapsuleComponent()->SetCollisionProfileName("Capsule");

	FireRate = 10;
	bIsFiring = false;
	FireDeltaTime = 0.0f;

	bUseControllerRotationYaw = true;
	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetMesh());
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	SphereMesh = CreateDefaultSubobject<UStaticMeshComponent>("Sphere");
	SphereMesh->SetupAttachment(GetCapsuleComponent());
}

void AFirstPersonCharacter::BeginPlay()
{
	Super::BeginPlay();

	FirePoint = Cast<USceneComponent>(GetDefaultSubobjectByName(TEXT("PointFire")));
	ArmRifle = Cast<USkeletalMeshComponent>(GetDefaultSubobjectByName(TEXT("Arm_Rifle")));
	HandRifle = Cast<USkeletalMeshComponent>(GetDefaultSubobjectByName(TEXT("Hand_Rifle")));

	
	FVector PrevPoint;
	for(float X = MinAxisX; X <= MaxAxisX; X += 0.01f)
	{
		float Y = F(X);

		FVector Point = OriginPoint + FVector(X * UnitAxisX, 0.0f, Y * UnitAxisZ);
		
		if (X > MinAxisX)
		{
			DrawDebugLine(GetWorld(), PrevPoint, Point, FColor::Green, 20.0f);
		}

		PrevPoint = Point;
	}

	DrawDebugLine(GetWorld(), FVector(MinAxisX * UnitAxisX, 0.0f, OriginPoint.Z),
		FVector(MaxAxisX * UnitAxisX, 0.0f, OriginPoint.Z), FColor::White, 20.0f);
	DrawDebugLine(GetWorld(), FVector(OriginPoint.X, OriginPoint.Y, -10000.0f),
		FVector(OriginPoint.X, OriginPoint.Y, 10000.0f), FColor::White, 20.0f);

	LoopCount = 0;

	GetWorld()->GetTimerManager().SetTimer(TimeHandle, this, &AFirstPersonCharacter::GradientDescent, IterationTimeRate, true);
}

inline float AFirstPersonCharacter::F(float X)
{
	float x = X;
	//return x * x * x * x - 3 * x * x * x + x * x + x - 3;
	return x * x + 10 * FMath::Sin(x);
}

inline float AFirstPersonCharacter::F_Gradient(float X)
{
	float x = X;
	//return 4 * x * x * x - 9 * x * x + 2 * x + 1;

	return 2 * x + 10 * FMath::Cos(x);
}

void AFirstPersonCharacter::GradientDescent()
{
	//if (LoopCount >= 20) return;
	
	if (LoopCount > 0)
	{
		float Velocity = Gamma * PreviousVelocity + LearningRate * F_Gradient(PreviousX - Gamma * PreviousVelocity);
		CurrentX = CurrentX - Velocity;

		PreviousX = CurrentX;
		PreviousVelocity = Velocity;
	}
	else
	{
		PreviousX  = CurrentX;

		float Velocity = LearningRate * F_Gradient(CurrentX);
		PreviousVelocity = Velocity;

		CurrentX = CurrentX - Velocity;
	}
	
	SphereMesh->SetWorldLocation(OriginPoint + FVector(CurrentX * UnitAxisX, 0, F(CurrentX) * UnitAxisZ));

	LoopCount++;
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, FString::Printf(TEXT("V %f"), PreviousVelocity));
}

void AFirstPersonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsLocallyControlled())
	{
		FirstPersonCameraComponent->RelativeRotation.Pitch = ControlPitch;
	}
	else
	{
		RPCServerUpdatePitch(GetControlRotation().Pitch);

		float ConstFireDeltaTime = 1.0f / FireRate;
		FireDeltaTime += DeltaTime;
		if (bIsFiring && FireDeltaTime >= ConstFireDeltaTime)
		{
			FireDeltaTime = FireDeltaTime - ConstFireDeltaTime;
			OnFire();
		}

		if (CurrentRecoil.X > 0.01f || CurrentRecoil.Y > 0.01f)
		{
			FRotator ControlRotation = GetControlRotation();
			
			float LerpAlpha = 0.1f;
			float RecoilScale = 0.5f;

			float ExtraPitch = CurrentRecoil.Y * LerpAlpha;
			CurrentRecoil.Y -= ExtraPitch;
			float ExtraYaw = CurrentRecoil.X * LerpAlpha;
			CurrentRecoil.X -= ExtraYaw;

			AddControllerPitchInput(-ExtraPitch * RecoilScale);
			AddControllerYawInput(ExtraYaw * RecoilScale);

			//AddDebugMessage(FString::Printf(TEXT("Apply Recoil %f %f"), ControlRotation.Pitch, ControlRotation.Yaw), FColor::Red, 1.0f, GetWorld());
		}
	}
}

void AFirstPersonCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFirstPersonCharacter::OnFireStart);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AFirstPersonCharacter::OnFireStop);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AFirstPersonCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFirstPersonCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AFirstPersonCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AFirstPersonCharacter::LookUpAtRate);
}

void AFirstPersonCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFirstPersonCharacter, ControlPitch);
}

bool AFirstPersonCharacter::RPCServerUpdatePitch_Validate(float Pitch)
{
	return true;
}

void AFirstPersonCharacter::RPCServerUpdatePitch_Implementation(float Pitch)
{
	ControlPitch = Pitch;
}

bool AFirstPersonCharacter::IsFiring()
{
	return bIsFiring;
}

void AFirstPersonCharacter::OnFireStart()
{
	bIsFiring = true;
	FireDeltaTime = 0.0f;
	
	OnFire();
}

void AFirstPersonCharacter::OnFireStop()
{
	bIsFiring = false;
	FireDeltaTime = 0.0f;
}

void AFirstPersonCharacter::OnFire()
{
	CurrentRecoil = FVector2D(FMath::RandRange(-RecoilRangeX, RecoilRangeX), FMath::RandRange(RecoilMinY, RecoilMaxY));

	//RPCServerFire(Recoil);	

	SpawnBulletWithRecoil(GetControlRotation());
}

void AFirstPersonCharacter::SpawnBulletWithRecoil(FRotator Rotation)
{
	if (BulletClass != NULL)
	{
		if (UWorld * World = GetWorld())
		{	
			FVector Location = FirePoint->GetComponentLocation();

			ABullet * Bullet = World->SpawnActor<ABullet>(BulletClass, Location, Rotation);
			Bullet->IgnoredCollisionActor = this;
		
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, Location, 1.0f, FMath::RandRange(0.9f, 1.0f));
		}
	}
}

bool AFirstPersonCharacter::RPCServerFire_Validate()
{
	return true;
}

void AFirstPersonCharacter::RPCServerFire_Implementation()
{
	RPCMulticastFire(GetControlRotation());
}

bool AFirstPersonCharacter::RPCMulticastFire_Validate(FRotator Rotation)
{
	return true;
}

void AFirstPersonCharacter::RPCMulticastFire_Implementation(FRotator Rotation)
{
	if (Controller == NULL || Role == ROLE_Authority || (!IsLocallyControlled()))
	{
		SpawnBulletWithRecoil(Rotation);

		AddDebugMessage("Client Fire", FColor::Green, 1.0f, GetWorld());
	}
}

void AFirstPersonCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{

	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AFirstPersonCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

void AFirstPersonCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AFirstPersonCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AFirstPersonCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AFirstPersonCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool AFirstPersonCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AFirstPersonCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AFirstPersonCharacter::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AMyProject3Character::TouchUpdate);
		return true;
	}

	return false;
}

