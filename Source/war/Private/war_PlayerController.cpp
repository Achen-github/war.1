// Fill out your copyright notice in the Description page of Project Settings.


#include "war_PlayerController.h"
#include "BaseUnit.h"
#include "war_GameMode.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/InputComponent.h"


Awar_PlayerController::Awar_PlayerController()
{
    //// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    //PrimaryActorTick.bCanEverTick = true;
	bShowMouseCursor = true;
	bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
}

void Awar_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(warContext, 0);
	}
    if (UnitStatsClass)
    {
        UnitStats = CreateWidget<UUserWidget>(GetWorld(), UnitStatsClass);
        if (UnitStats)
        {
            UnitStats->AddToViewport();
        }
    }

}

void Awar_PlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    InputComponent->BindAction("Select", IE_Pressed, this, &Awar_PlayerController::SelectUnit);
    InputComponent->BindAction("Move", IE_Pressed, this, &Awar_PlayerController::MoveUnit);
}

// Selezione della pedina con il click
void Awar_PlayerController::SelectUnit()
{
    FHitResult HitResult;
    GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

    if (HitResult.bBlockingHit)
    {
        ABaseUnit* ClickedUnit = Cast<ABaseUnit>(HitResult.GetActor());

        if (ClickedUnit)
        {
            SelectedUnit = ClickedUnit;
            UE_LOG(LogTemp, Warning, TEXT("Unità selezionata: %s"), *ClickedUnit->GetName());
        }
    }
}

// Comando per spostare la pedina selezionata
void Awar_PlayerController::MoveUnit()
{
    if (!SelectedUnit) return;

    FHitResult HitResult;
    GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

    if (HitResult.bBlockingHit)
    {
        FVector TargetLocation = HitResult.ImpactPoint;

        // Ottieni la tile su cui il giocatore ha cliccato
        Awar_GameMode* GameMode = Cast<Awar_GameMode>(GetWorld()->GetAuthGameMode());
        if (!GameMode || !GameMode->GField) return;

        ATile* TargetTile = GameMode->GField->GetTileAt(TargetLocation);
        if (!TargetTile) return;

        // Chiedi al GameMode di gestire il movimento sulla griglia
        GameMode->MoveUnitToTile(SelectedUnit, TargetTile);

        UE_LOG(LogTemp, Warning, TEXT("Unità spostata verso la tile a: %s"), *TargetTile->GetActorLocation().ToString());
    }
}