// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Bullet.generated.h"

UCLASS()
class MYPROJECT2_API ABullet : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABullet();

	UPROPERTY(EditAnyWhere, Category = "Movement", Meta = (AllowPrivateAccess = "true"))
	float Velocity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent * Mesh;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = Collision, meta = (AllowPrivateAccess = "true"))
	class UMaterialInterface * DecalMaterial;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = Collision, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem * Particle;

	UFUNCTION(BlueprintCallable)
	void PlayDurable(FVector& point);

	AActor * IgnoredCollisionActor;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FVector Direction;
	FHitResult HitResult;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
};
