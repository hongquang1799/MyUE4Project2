// Fill out your copyright notice in the Description page of Project Settings.


#include "Bullet.h"
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DecalComponent.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

#include "Engine/DecalActor.h"

// Sets default values
ABullet::ABullet()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Velocity = 1000.f;
	IgnoredCollisionActor = NULL;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->RelativeScale3D = FVector(0.1f, 0.02f, 0.02f);
	Mesh->SetupAttachment(RootComponent);
}

void ABullet::PlayDurable(FVector& point)
{
	point = -point;
}

// Called when the game starts or when spawned
void ABullet::BeginPlay()
{
	Super::BeginPlay();
	
	Direction = GetActorForwardVector();

	SetLifeSpan(3.0f);
}

// Called every frame
void ABullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector StartTrace = GetActorLocation();
	FVector EndTrace = StartTrace + Direction * Velocity * DeltaTime;

	/*FCollisionQueryParams CollisionQueryParam;
	CollisionQueryParam.AddIgnoredActor(this);*/

	//DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Green, false, 3.0f);
	UWorld * World = GetWorld();
	if (World->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_WorldStatic))
	{
		AActor * HitActor = HitResult.GetActor();
		if (HitActor != NULL && HitActor != IgnoredCollisionActor)
		{
			//DrawDebugSolidBox(GetWorld(), HitResult.ImpactPoint, FVector(3.f), FColor::Red, false, 5.0f);
			if (DecalMaterial != NULL)
			{
				UDecalComponent * DecalComponent = UGameplayStatics::SpawnDecalAtLocation(World, DecalMaterial, FVector(20, 20, 20), HitResult.ImpactPoint,
					HitResult.ImpactNormal.ToOrientationRotator(), 4.0f);
				DecalComponent->SetFadeScreenSize(0.0001f);
			}

			if (Particle != NULL)
			{
				UGameplayStatics::SpawnEmitterAtLocation(World, Particle, HitResult.ImpactPoint,
					HitResult.ImpactNormal.ToOrientationRotator());
			}

			Destroy();
		}	

		SetActorLocation(EndTrace);
		//DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Green, false, 5.f);
	}
	else
	{
		SetActorLocation(EndTrace);
		//DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Green, false, 5.f);
	}
}

