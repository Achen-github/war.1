// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "war_Interface.h"
#include "GameField.h"
#include "war_HumanPlayer.h"
#include "BaseUnit.h"
#include "war_GameInstance.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameMode.h"
#include "war_GameMode.generated.h"


/**
 * 
 */
UCLASS()

class WAR_API Awar_GameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	// tracks if the game is over
	void IsGameOver();
	// array of player interfaces
	TArray<Iwar_Interface*> Players;
	int32 CurrentPlayer;
	// tracks the number of moves in order to signal a drawn game
	int32 MoveCounter;

	// TSubclassOf is a template class that provides UClass type safety.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AGameField> GameFieldClass;

	// field size
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 FieldSize;

	// reference to a GameField object
	UPROPERTY(VisibleAnywhere)
	AGameField* GField;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ABaseUnit> SniperA;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ABaseUnit> SniperB;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ABaseUnit> BrawlerA;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ABaseUnit> BrawlerB;
	UPROPERTY()
	APawn* SelectedUnit = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<ABaseUnit*> PlayerUnits;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<ABaseUnit*> AIUnits;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bIsPlayer1Turn;
	int32 UnitsPlaced;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bSetupPhase;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bGameOver;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bDifficulty; // 0 = Easy, 1 = Medium
	UClass* SelectedUnitClass;
	Uwar_GameInstance* GameInstance;
	Awar_GameMode();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// called at the start of the game
	void ChoosePlayerAndStartGame();
	UFUNCTION(BlueprintCallable)
	void SelectUnitToPlace(UClass* UnitClass);
	UFUNCTION(BlueprintCallable)
	bool IsUnitOwnedByPlayer(ABaseUnit* Unit);
	bool AreAllPlayerUnitsDone() const;
	void MoveUnitToTile(ABaseUnit* Unit, ATile* TargetTile);
	// set the cell sign and the position 
	void PlaceUnit(const FVector& SpawnPosition);

	// get the next player index
	int32 GetNextPlayer(int32 Player);

	// called at the end of the game turn
	UFUNCTION(BlueprintCallable)
	void TurnNextPlayer();

	void CheckPlayerController();

	void SetSelectedUnit(APawn* Unit);

	APawn* GetSelectedUnit() const;
	UFUNCTION(BlueprintCallable)
	void SetDifficulty(bool bDiff);
	UFUNCTION(BlueprintCallable)
	bool GetDifficulty() const;

};

