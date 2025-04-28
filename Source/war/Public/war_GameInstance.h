// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include <Components/ScrollBox.h>
#include "war_GameInstance.generated.h"


/**
 * 
 */
UCLASS()
class WAR_API Uwar_GameInstance : public UGameInstance
{
	GENERATED_BODY()
public:

	// message to show every turn
	UPROPERTY(EditAnywhere)
	FString CurrentTurnMessage = "Current Player";
	UPROPERTY(EditAnywhere)
	FString UnitStatsText= "Placing units..";
	UPROPERTY(EditAnywhere)
	FString EndMessage = "";
	UPROPERTY(EditAnywhere)
	FString MessageLog="";
	// get the current turn message
	UFUNCTION(BlueprintCallable)
	FString GetTurnMessage();

	// set the turn message
	UFUNCTION(BlueprintCallable)
	void SetTurnMessage(FString Message);
	UFUNCTION(BlueprintCallable)
	void UpdateUnitStatsText();
	void ResetUnitStatsText();
	UFUNCTION(BlueprintCallable)
	FString GetStats();
	UFUNCTION(BlueprintCallable)
	FString GetEndMessage();
	UFUNCTION(BlueprintCallable)
	void SetEndMessage(FString Message);
	UFUNCTION(BlueprintCallable)
	FString GetMessageLog();
	UFUNCTION(BlueprintCallable)
	void SetMessageLog(FString Message);
};
