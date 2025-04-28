// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "war_GameInstance.h"
#include "war_Interface.h"
#include "Tile.h"
#include "BaseUnit.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "war_HumanPlayer.generated.h"

UCLASS()
class WAR_API Awar_HumanPlayer : public APawn, public Iwar_Interface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	Awar_HumanPlayer();
	// camera component attacched to player pawn
	UCameraComponent* Camera;

	// game instance reference
	Uwar_GameInstance* GameInstance;

	FTimerHandle MovementTimerHandle;
	bool bIsCurrentlyMoving = false;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool IsMyTurn = false;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void OnTurn() override;
	virtual void OnWin() override;
	virtual void OnLose() override;


	// called on left mouse click (binding)
	UFUNCTION()
	void OnClick();
	void HandlePlacement(AActor* HitActor);
	void HandleSelection(AActor* HitActor);
	void HandleDeselection();
	void HandleMovement(ATile* CurrTile);
	void HandleAttack(ABaseUnit* CurrTile);
	UFUNCTION()
	void OnUnitMoveFinished(ABaseUnit* Unit);
	void ShowMovementRange();
	void ClearHighlightsMove();
	void ClearHighlightsAttack();
	void ShowAttackRange();
	UPROPERTY()
	TArray<ATile*> HighlightedMoveTiles; // Tile evidenziate per il movimento

	UPROPERTY()
	TArray<ATile*> HighlightedAttackTiles; // Tile evidenziate per l'attacco

};
