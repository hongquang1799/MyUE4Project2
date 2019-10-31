// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUp.h"
#include "MyProject2Character.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "Classes/Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

// Sets default values
APickUp::APickUp()
{
	bReplicates = true;
	bReplicateMovement = true;
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetSimulatePhysics(true);
	RootComponent = Mesh;
}

void APickUp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

// Called when the game starts or when spawned
void APickUp::BeginPlay()
{
	Super::BeginPlay();
	
	//SetOwner(GetWorld()->GetFirstPlayerController());

	if (Mesh->GetMaterial(0))
	{
		MaterialDynamic = UMaterialInstanceDynamic::Create(Mesh->GetMaterial(0), this);
		Mesh->SetMaterial(0, MaterialDynamic);
	}
}

// Called every frame
void APickUp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APickUp::ChangeColorNetMulticast_Implementation(const FLinearColor & Color)
{
	if (MaterialDynamic != NULL)
	{
		MaterialDynamic->SetVectorParameterValue(TEXT("MainColor"), Color);
		
		FString RoleString = (Role == ROLE_Authority) ? TEXT("SERVER") : TEXT("CLIENT");

		AMyProject2Character::AddDebugMessage(FString(TEXT("ChangColor: ")) + RoleString
			, FColor::Green, 3.0f, GetWorld());
	}
}

bool APickUp::ChangeColorServer_Validate(const FLinearColor & Color)
{
	return true;
}

void APickUp::ChangeColorServer_Implementation(const FLinearColor & Color)
{
	if (MaterialDynamic != NULL)
	{
		MaterialDynamic->SetVectorParameterValue(TEXT("MainColor"), Color);

		FString RoleString = (Role == ROLE_Authority) ? TEXT("SERVER") : TEXT("CLIENT");

		AMyProject2Character::AddDebugMessage(FString(TEXT("ChangColor: ")) + RoleString
			, FColor::Green, 3.0f, GetWorld());
	}
}

void APickUp::ChangeColorClient_Implementation(const FLinearColor & Color)
{
	FString RoleString = (Role == ROLE_Authority) ? TEXT("SERVER") : TEXT("CLIENT");
	AMyProject2Character::AddDebugMessage(FString(TEXT("ChangColor: ")) + RoleString
		, FColor::Green, 3.0f, GetWorld());

	if (MaterialDynamic != NULL)
	{
		MaterialDynamic->SetVectorParameterValue(TEXT("MainColor"), Color);
	}
}


