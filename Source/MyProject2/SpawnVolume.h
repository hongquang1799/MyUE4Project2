// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnVolume.generated.h"

UCLASS()
class MYPROJECT2_API ASpawnVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpawnVolume();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FTimerHandle SpawnTimer;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SpawnPickUp();
private:
	UPROPERTY(EditAnyWhere, Category = "Spawning", Meta = (AllowPrivateAccess = "true"))
		class UBoxComponent * BoxComp;

	UPROPERTY(EditAnyWhere, Category = "Spawning", Meta = (AllowPrivateAccess = "true"))
		TSubclassOf<class APickUp> SpawnTarget;

	UPROPERTY(EditAnyWhere, Category = "Spawning", Meta = (AllowPrivateAccess = "true"))
		float SpawnDeltaTime;

	int PickUpAmount;
};
