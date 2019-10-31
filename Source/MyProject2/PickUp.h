// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickUp.generated.h"

UCLASS()
class MYPROJECT2_API APickUp : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickUp();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FORCEINLINE class UMaterialInstanceDynamic * GetMaterialDynamic() { return MaterialDynamic; }

	UFUNCTION(NetMulticast, Reliable)
	void ChangeColorNetMulticast(const FLinearColor& Color);

	void ChangeColorNetMulticast_Implementation(const FLinearColor& Color);

	UFUNCTION(Server, Reliable, WithValidation)
	void ChangeColorServer(const FLinearColor& Color);

	bool ChangeColorServer_Validate(const FLinearColor& Color);

	void ChangeColorServer_Implementation(const FLinearColor& Color);

	UFUNCTION(Client, Reliable)
	void ChangeColorClient(const FLinearColor& Color);

	void ChangeColorClient_Implementation(const FLinearColor& Color);
private:
	UPROPERTY(VisibleAnyWhere, Category = "Static Mesh", Meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent * Mesh;

	UPROPERTY(VisibleAnyWhere, Category = "Static Mesh", Meta = (AllowPrivateAccess = "true"))
		class UMaterialInstanceDynamic * MaterialDynamic;
};
