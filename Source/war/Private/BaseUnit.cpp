// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseUnit.h"
#include "war_PlayerController.h"
#include "war_Sniper.h"
#include "war_Brawler.h"
#include "BaseUnit.h"
#include "Tile.h"
#include "war_GameMode.h"
#include "Kismet/GameplayStatics.h"
// Sets default values
ABaseUnit::ABaseUnit()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bIsSelected = false;
    GameInstance = Cast<Uwar_GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
    //CurrentTile = nullptr;
}

ATile* ABaseUnit::GetCurrentTile() const
{
    Awar_GameMode* GameMode = Cast<Awar_GameMode>(GetWorld()->GetAuthGameMode());
    if (GameMode && GameMode->GField)
    {
        return GameMode->GField->GetTileAt(GetActorLocation());
    }
    return nullptr;
}

// Called when the game starts or when spawned
void ABaseUnit::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABaseUnit::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsMoving && MovementPath.IsValidIndex(CurrentPathIndex))
    {
        FVector TargetLoc = MovementPath[CurrentPathIndex]->GetActorLocation();
        TargetLoc.Z = GetActorLocation().Z;

        FVector NewLocation = FMath::VInterpConstantTo(GetActorLocation(), TargetLoc, DeltaTime, MoveSpeed);

        SetActorLocation(NewLocation);

        if (FVector::Dist(NewLocation, TargetLoc) < 1.f)
        {
            CurrentPathIndex++;

            if (CurrentPathIndex >= MovementPath.Num())
            {
                bIsMoving = false;

                // Snap finale alla posizione esatta della tile
                if (TargetTile)
                {
                    FVector SnapLocation = TargetTile->GetActorLocation();
                    SnapLocation.Z = GetActorLocation().Z;
                    SetActorLocation(SnapLocation);

                    Awar_GameMode* GameMode = Cast<Awar_GameMode>(GetWorld()->GetAuthGameMode());
                    TargetTile->SetTileStatus(GameMode->CurrentPlayer, ETileStatus::OCCUPIED);
                    TargetTile->SetOccupyingUnit(this);
                }

                MovementPath.Empty();
                TargetTile = nullptr;

                // Notifica a chi ascolta l'evento
                OnUnitFinishedMoving.Broadcast(this);
            }
        }
    }
}
// Called to bind functionality to input
void ABaseUnit::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABaseUnit::SetGridPosition(FVector2D NewPosition)
{
    GridPosition = NewPosition;
}

FVector2D ABaseUnit::GetGridPosition() const
{
    return GridPosition;
}
//// Getter e Setter per la Tile corrente
//ATile* ABaseUnit::GetCurrentTile() const {
//    return CurrentTile; 
//}
//void ABaseUnit::SetCurrentTile(ATile* NewTile) { 
//    CurrentTile = NewTile; 
//}
void ABaseUnit::AttackUnit(ABaseUnit* TargetUnit)
{
    if (!TargetUnit) return;

    int32 Damage = 0;

    if (Awar_Sniper* Sniper = Cast<Awar_Sniper>(this))
    {
        Damage = Sniper->GetRandomDamage();
    }
    else if (Awar_Brawler* Brawler = Cast<Awar_Brawler>(this))
    {
        Damage = Brawler->GetRandomDamage();
    }

    TargetUnit->CurrentHealth -= Damage;

    UE_LOG(LogTemp, Warning, TEXT("%s ha inflitto %d danni a %s, vita rimanente di %s = %d"),
        *GetName(), Damage, *TargetUnit->GetName(), *TargetUnit->GetName(), TargetUnit->CurrentHealth);
    // Aggiorno i log
    Awar_GameMode* GameMode = Cast<Awar_GameMode>(GetWorld()->GetAuthGameMode());
    FString FromLabel = GameMode->GField->CoordsToLabel(this->GetGridPosition());
    FString ToLabel = GameMode->GField->CoordsToLabel(TargetUnit->GetGridPosition());
    FString Name = this->GetName();
    if (Name == "BP_SniperA_C_0")
        Name = "Sniper";
    if (Name == "BP_BrawlerA_C_0")
        Name = "Brawler";
    if (Name == "BP_SniperB_C_0")
        Name = "Sniper";
    if (Name == "BP_BrawlerB_C_0")
        Name = "Brawler";
    FString MoveText = FString::Printf(TEXT("\n%s Attacked at %s dealing %d DMG"),
         *Name,*ToLabel, Damage);
    FString Log = GameInstance->GetMessageLog();
    GameInstance->SetMessageLog(MoveText + Log);

    if (TargetUnit->CurrentHealth <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s è stato sconfitto!"), *TargetUnit->GetName());
        FString Enemy = TargetUnit->GetName();
        if (Enemy == "BP_SniperA_C_0")
            Enemy = "Sniper";
        if (Enemy == "BP_BrawlerA_C_0")
            Enemy = "Brawler";
        if (Enemy == "BP_SniperB_C_0")
            Enemy = "Sniper";
        if (Enemy == "BP_BrawlerB_C_0")
            Enemy = "Brawler";
        MoveText = FString::Printf(TEXT("\n%s at %s Defeated"),
            *Enemy,*ToLabel, Damage);
        Log = GameInstance->GetMessageLog();
        GameInstance->SetMessageLog(MoveText + Log);
        TargetUnit->CurrentHealth = 0;
        if (ATile* Tile = TargetUnit->GetCurrentTile())
        {
            Tile->SetTileStatus(-1, ETileStatus::EMPTY);
            Tile->SetOccupyingUnit(nullptr);
        }
        // Rimuovi l'unità dagli array nel GameMode
        
            GameMode->PlayerUnits.Remove(TargetUnit);
            GameMode->AIUnits.Remove(TargetUnit);
        
        TargetUnit->Destroy();
    }

    int32 Distance = FMath::Abs(TargetUnit->GetGridPosition().X - this->GetGridPosition().X) +
        FMath::Abs(TargetUnit->GetGridPosition().Y - this->GetGridPosition().Y);
    UE_LOG(LogTemp, Warning, TEXT("%s dista da %s di: %d"), *TargetUnit->GetName(), *this->GetName(), Distance);
    //UE_LOG(LogTemp, Warning, TEXT("Unita in X: %.1f, Y: %.1f"), SelectedUnit->GetGridPosition().X, SelectedUnit->GetGridPosition().Y);
    //UE_LOG(LogTemp, Warning, TEXT("Nemico in X: %.1f, Y: %.1f"), ClickedUnit->GetGridPosition().X, ClickedUnit->GetGridPosition().Y);
    // Se soppravvive ed è in range contrattacca
    if (TargetUnit->CurrentHealth > 0 && Distance <= TargetUnit->GetAttackRange())
    {
        UE_LOG(LogTemp, Warning, TEXT("Contrattacco dell' unità nemica: %s"), *TargetUnit->GetName());
        TargetUnit->CounterAttackUnit(this);
    }
    else UE_LOG(LogTemp, Warning, TEXT("Unità non è in range per contrattaccare: %s"), *TargetUnit->GetName());


    GameInstance->UpdateUnitStatsText();
}
void ABaseUnit::CounterAttackUnit(ABaseUnit* TargetUnit)
{
    if (!TargetUnit) return;

    int32 Damage = 0;

    if (Awar_Sniper* Sniper = Cast<Awar_Sniper>(this))
    {
        Damage = Sniper->GetRandomCounter();
    }
    else if (Awar_Brawler* Brawler = Cast<Awar_Brawler>(this))
    {
        Damage = Brawler->GetRandomCounter();
    }

    TargetUnit->CurrentHealth -= Damage;

    UE_LOG(LogTemp, Warning, TEXT("%s ha contrattaccato infliggendo %d danni a %s, vita rimanente di %s = %d"),
        *GetName(), Damage, *TargetUnit->GetName(), *TargetUnit->GetName(), TargetUnit->CurrentHealth);
    // Aggiorno i log
    Awar_GameMode* GameMode = Cast<Awar_GameMode>(GetWorld()->GetAuthGameMode());
    FString FromLabel = GameMode->GField->CoordsToLabel(this->GetGridPosition());
    FString ToLabel = GameMode->GField->CoordsToLabel(TargetUnit->GetGridPosition());
    FString Name = this->GetName();
    if (Name == "BP_SniperA_C_0")
        Name = "Sniper";
    if (Name == "BP_BrawlerA_C_0")
        Name = "Brawler";
    if (Name == "BP_SniperB_C_0")
        Name = "Sniper";
    if (Name == "BP_BrawlerB_C_0")
        Name = "Brawler";
    FString MoveText = FString::Printf(TEXT("\n%s Counterattacked dealing %d DMG"),
         *Name,Damage);
    FString Log = GameInstance->GetMessageLog();
    GameInstance->SetMessageLog(MoveText + Log);

    if (TargetUnit->CurrentHealth <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s è stato sconfitto!"), *TargetUnit->GetName());
        FString Enemy = TargetUnit->GetName();
        if (Enemy == "BP_SniperA_C_0")
            Enemy = "Sniper";
        if (Enemy == "BP_BrawlerA_C_0")
            Enemy = "Brawler";
        if (Enemy == "BP_SniperB_C_0")
            Enemy = "Sniper";
        if (Enemy == "BP_BrawlerB_C_0")
            Enemy = "Brawler";
        MoveText = FString::Printf(TEXT("\n%s at %s Defeated"),
            *Enemy,*ToLabel,Damage);
        Log = GameInstance->GetMessageLog();
        GameInstance->SetMessageLog(MoveText + Log);
        TargetUnit->CurrentHealth = 0;
        if (ATile* Tile = TargetUnit->GetCurrentTile())
        {
            Tile->SetTileStatus(-1, ETileStatus::EMPTY);
            Tile->SetOccupyingUnit(nullptr);
        }
            GameMode->PlayerUnits.Remove(TargetUnit);
            GameMode->AIUnits.Remove(TargetUnit);
        TargetUnit->Destroy();
    }

    GameInstance->UpdateUnitStatsText();
}
int32 ABaseUnit::GetMaxMoveRange() const
{
    return MaxMoveRange;
}

int32 ABaseUnit::GetAttackRange() const
    {
        return AttackRange;
    }