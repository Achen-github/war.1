// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tile.h"
#include "GameFramework/Actor.h"
#include "GameField.generated.h"

// macro declaration for a dynamic multicast delegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReset);


UCLASS()
class WAR_API AGameField : public AActor
{
	GENERATED_BODY()
	
public:
	// keeps track of tiles
	UPROPERTY(Transient)
	TArray<ATile*> TileArray;

	//given a position returns a tile
	UPROPERTY(Transient)
	TMap<FVector2D, ATile*> TileMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float NextCellPositionMultiplier;

	static const int32 NOT_ASSIGNED = -1;

	// size of field
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Size;

	//ostacoli
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<ATile> ObstacleTree1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<ATile> ObstacleTree2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<ATile> ObstacleMountain;


	// TSubclassOf template class that provides UClass type safety
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ATile> TileClass;

	// tile padding percentage
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CellPadding;

	// tile size
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TileSize;

	Uwar_GameInstance* GameInstance;
	// Sets default values for this actor's properties
	AGameField();

	// Called when an instance of this class is placed (in editor) or spawned
	virtual void OnConstruction(const FTransform& Transform) override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// remove all signs from the field
	UFUNCTION(BlueprintCallable)
	void ResetField();

	// generate an empty game field
	void GenerateField();
	// generate obstacles in the field
	void GenerateObstacles();
	//mescola l'array
	void ShuffleArray(TArray<FVector2D>& Array);
//controlla se è tutto connesso
	bool BFS(TArray<TArray<bool>>& Grid);

	// return a (x,y) position given a hit (click) on a field tile
	FVector2D GetPosition(const FHitResult& Hit);

	// return the array of tile pointers
	TArray<ATile*>& GetTileArray();

	// return a relative position given (x,y) position
	FVector GetRelativeLocationByXYPosition(const int32 InX, const int32 InY) const;

	// return (x,y) position given a relative position
	FVector2D GetXYPositionByRelativeLocation(const FVector& Location) const;

	ATile* GetTileAtPosition(FVector2D Position);

	// checking if is a valid field position
	inline bool IsValidPosition(const FVector2D Position) const;

	UFUNCTION()
	ATile* GetTileAt(FVector Location);
	UFUNCTION()
	TArray<ATile*> GetTilesInRangeBFS(ATile* StartTile, int32 MaxDistance);
	TArray<ATile*> FindPath(ATile* StartTile, ATile* EndTile);
	FString CoordsToLabel(const FVector2D& GridPos);
//public:	
//	// Called every frame
//	virtual void Tick(float DeltaTime) override;

};
