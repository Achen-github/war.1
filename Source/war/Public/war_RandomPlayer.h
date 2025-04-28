// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "war_Interface.h"
#include "war_GameInstance.h"
#include "war_GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "war_RandomPlayer.generated.h"

UCLASS()
class WAR_API Awar_RandomPlayer : public APawn, public Iwar_Interface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	Awar_RandomPlayer();
	Uwar_GameInstance* GameInstance;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void OnTurn() override;
	virtual void OnWin() override;
	virtual void OnLose() override;
	void MoveNextAIUnit(Awar_GameMode* GameMode, const TArray<ABaseUnit*>& Units, int32 Index);
	void MoveNextAIUnitSmart(Awar_GameMode* GameMode, const TArray<ABaseUnit*>& Units, int32 Index);
};
