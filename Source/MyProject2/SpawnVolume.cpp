// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnVolume.h"
#include "PickUp.h"
#include "MyProject2Character.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Public/TimerManager.h"
#include "Engine/Public/EngineUtils.h"
#include "Engine/Engine.h"

#include "Kismet/KismetMathLibrary.h"

// Sets default values
ASpawnVolume::ASpawnVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	if (Role = ROLE_Authority)
	{
		BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawningBox"));
		RootComponent = BoxComp;
	}

	PickUpAmount = 0;
}

// Called when the game starts or when spawned
void ASpawnVolume::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(SpawnTimer, this, &ASpawnVolume::SpawnPickUp, SpawnDeltaTime, true);
}

// Called every frame
void ASpawnVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASpawnVolume::SpawnPickUp()
{
	if (Role == ROLE_Authority && PickUpAmount <= 10 && SpawnTarget != NULL)
	{
		if (UWorld * World = GetWorld())
		{
			FActorSpawnParameters Param;
			Param.Owner = this;
			Param.Instigator = Instigator;

			FVector SpawnLocation = UKismetMathLibrary::RandomPointInBoundingBox(BoxComp->Bounds.Origin, BoxComp->Bounds.BoxExtent);
			FRotator SpawnRotation = FRotator(FMath::FRand() * 360.0f, FMath::FRand() * 360.0f, FMath::FRand() * 360.0f);

			TActorIterator<AMyProject2Character> Itr(GetWorld());
			++Itr;
			++Itr;

			APickUp * PickUp = World->SpawnActor<APickUp>(SpawnTarget, SpawnLocation, SpawnRotation, Param);
			PickUp->SetOwner((*Itr)->GetController());

			PickUpAmount++;
		}		
	}
}

