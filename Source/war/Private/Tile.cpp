// Fill out your copyright notice in the Description page of Project Settings.


#include "Tile.h"

// Sets default values
ATile::ATile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// template function that creates a components
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));

	// every actor has a RootComponent that defines the transform in the World
	SetRootComponent(Scene);
	StaticMeshComponent->SetupAttachment(Scene);

	Status = ETileStatus::EMPTY;
	PlayerOwner = -1;
	TileGridPosition = FVector2D(0, 0);
	UStaticMeshComponent* Mesh = FindComponentByClass<UStaticMeshComponent>();
	if (Mesh && DefaultMaterial)
	{
		Mesh->SetMaterial(0, DefaultMaterial);
	}

}

void ATile::SetTileStatus(const int32 TileOwner, const ETileStatus TileStatus)
{
	UE_LOG(LogTemp, Warning, TEXT("Tile %s: Cambio stato da %d a %d"), *GetName(), Status, TileStatus);
	PlayerOwner = TileOwner;
	Status = TileStatus;
}

ETileStatus ATile::GetTileStatus()
{
	return Status;
}

int32 ATile::GetOwner()
{
	return PlayerOwner;
}

void ATile::SetGridPosition(const double InX, const double InY)
{
	TileGridPosition.Set(InX, InY);
}

FVector2D ATile::GetGridPosition()
{
	return TileGridPosition;
}

void ATile::SetOccupyingUnit(ABaseUnit* Unit)
{
	OccupyingUnit = Unit;
}
ABaseUnit* ATile::GetOccupyingUnit() const 
{
	return OccupyingUnit;
}
void ATile::HighlightTile(bool bHighlight)
{
	UStaticMeshComponent* Mesh = FindComponentByClass<UStaticMeshComponent>();
	if (Mesh)
	{
		if (bHighlight && HighlightMaterial)
		{
			Mesh->SetMaterial(0, HighlightMaterial);
		}
		else if (!bHighlight && DefaultMaterial)
		{
			Mesh->SetMaterial(0, DefaultMaterial);
		}
	}
}

void ATile::HighlightAttackTile(bool bHighlight)
{
	UStaticMeshComponent* Mesh = FindComponentByClass<UStaticMeshComponent>();
	if (Mesh)
	{
		if (bHighlight && AttackMaterial)
		{
			Mesh->SetMaterial(0, AttackMaterial);
		}
		else if (!bHighlight && DefaultMaterial)
		{
			Mesh->SetMaterial(0, DefaultMaterial);
		}
	}
}
void ATile::ResetHighlight()
{
	UStaticMeshComponent* Mesh = FindComponentByClass<UStaticMeshComponent>();
	if (Mesh)
	{
		Mesh->SetRenderCustomDepth(false);
	}
}

// Called when the game starts or when spawned
void ATile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
//void ATile::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}

