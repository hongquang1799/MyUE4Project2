// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FirstPersonCharacter.generated.h"

UCLASS()
class MYPROJECT2_API AFirstPersonCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* FirePoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* ArmRifle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* HandRifle;
public:
	AFirstPersonCharacter();

	static void	AddDebugMessage(const FString & Text, const FColor & Color, float Duration, class UWorld * World);
protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;
public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseLookUpRate;

	UPROPERTY(EditAnyWhere, Category = "Shooting", Meta = (AllowPrivateAccess = "true"))
		TSubclassOf<class ABullet> BulletClass;

	UPROPERTY(EditAnyWhere, Category = "Shooting", Meta = (AllowPrivateAccess = "true"))
		int FireRate;

	UPROPERTY(EditAnyWhere, Category = "Shooting", Meta = (AllowPrivateAccess = "true"))
		float RecoilRangeX;

	UPROPERTY(EditAnyWhere, Category = "Shooting", Meta = (AllowPrivateAccess = "true"))
		float RecoilMinY;

	UPROPERTY(EditAnyWhere, Category = "Shooting", Meta = (AllowPrivateAccess = "true"))
		float RecoilMaxY;
	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		class USoundBase* FireSound;

	UPROPERTY(Replicated)
		float ControlPitch;

	void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable, WithValidation)
	void RPCServerUpdatePitch(float Pitch);

	bool RPCServerUpdatePitch_Validate(float Pitch);

	void RPCServerUpdatePitch_Implementation(float Pitch);

	UFUNCTION(BlueprintCallable)
	bool IsFiring();

	UPROPERTY(EditAnyWhere, Category = "Shooting", Meta = (AllowPrivateAccess = "true"))
	bool bIsFiring;

	float FireDeltaTime;

	FVector2D CurrentRecoil;
protected:
	UFUNCTION(BlueprintCallable)
	void OnFireStart();

	UFUNCTION(BlueprintCallable)
	void OnFireStop();

	void OnFire();

	void SpawnBulletWithRecoil(FRotator Rotation);

	UFUNCTION(Server, Reliable, WithValidation)
	void RPCServerFire();

	bool RPCServerFire_Validate();

	void RPCServerFire_Implementation();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void RPCMulticastFire(FRotator Rotation);

	bool RPCMulticastFire_Validate(FRotator Rotation);

	void RPCMulticastFire_Implementation(FRotator Rotation);

	void MoveForward(float Val);

	void MoveRight(float Val);

	void TurnAtRate(float Rate);

	void LookUpAtRate(float Rate);

	struct TouchData
	{
		TouchData() { bIsPressed = false; Location = FVector::ZeroVector; }
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};
	void BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);
	TouchData	TouchItem;

protected:
	
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	/*
	 * Configures input for touchscreen devices if there is a valid touch interface for doing so
	 *
	 * @param	InputComponent	The input component pointer to bind controls to
	 * @returns true if touch controls were enabled.
	 */
	bool EnableTouchscreenMovement(UInputComponent* InputComponent);

public:

	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	UPROPERTY(EditAnyWhere, Category = "Gradient Descent", Meta = (AllowPrivateAccess = "true"))
	FVector OriginPoint;
	UPROPERTY(EditAnyWhere, Category = "Gradient Descent", Meta = (AllowPrivateAccess = "true"))
	float MinAxisX;
	UPROPERTY(EditAnyWhere, Category = "Gradient Descent", Meta = (AllowPrivateAccess = "true"))
	float MaxAxisX;
	UPROPERTY(EditAnyWhere, Category = "Gradient Descent", Meta = (AllowPrivateAccess = "true"))
	float UnitAxisX;
	UPROPERTY(EditAnyWhere, Category = "Gradient Descent", Meta = (AllowPrivateAccess = "true"))
	float UnitAxisZ;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent * SphereMesh;

	UPROPERTY(EditAnyWhere, Category = "Gradient Descent", Meta = (AllowPrivateAccess = "true"))
	float CurrentX;

	UPROPERTY(EditAnyWhere, Category = "Gradient Descent", Meta = (AllowPrivateAccess = "true"))
	float LearningRate;

	UPROPERTY(EditAnyWhere, Category = "Gradient Descent", Meta = (AllowPrivateAccess = "true"))
	float IterationTimeRate;

	UPROPERTY(EditAnyWhere, Category = "Gradient Descent", Meta = (AllowPrivateAccess = "true"))
	float Gamma;

	float CurrentY;

	float PreviousVelocity;
	float PreviousGradient;
	float PreviousX;

	unsigned int LoopCount;

	FTimerHandle TimeHandle;

	float F(float X);

	float F_Gradient(float X);

	void GradientDescent();
};
