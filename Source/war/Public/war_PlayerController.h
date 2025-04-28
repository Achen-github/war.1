// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "war_HumanPlayer.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"

#include "EnhancedInputSubsystems.h"
#include "war_PlayerController.generated.h"

/**
 * 
 */
UCLASS()
class WAR_API Awar_PlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	Awar_PlayerController();

	UPROPERTY(EditAnywhere, Category = Input)
	UInputMappingContext* warContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UUserWidget> UnitStatsClass;

	UUserWidget* UnitStats;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* ClickAction;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	//virtual void Tick(float DeltaTime) override;
private:
	class ABaseUnit* SelectedUnit; // La pedina attualmente selezionata

	void SelectUnit(); // Seleziona la pedina con il click
	void MoveUnit();   // Comanda il movimento della pedina

	
	

};
