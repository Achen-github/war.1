// Fill out your copyright notice in the Description page of Project Settings.


#include "war_RandomPlayer.h"
#include "war_Brawler.h"
#include "war_Sniper.h"

// Sets default values
Awar_RandomPlayer::Awar_RandomPlayer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GameInstance = Cast<Uwar_GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
}

// Called when the game starts or when spawned
void Awar_RandomPlayer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void Awar_RandomPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void Awar_RandomPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void Awar_RandomPlayer::OnTurn()
{
    if (GameInstance)
    {
        GameInstance->SetTurnMessage(TEXT("AI (Random) Turn"));
    }
    //GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("AI (Random) Turn"));

    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            Awar_GameMode* GameMode = Cast<Awar_GameMode>(GetWorld()->GetAuthGameMode());
            if (!GameMode || !GameMode->GField) return;

            // ----- Fase di piazzamento -----
            if (GameMode->bSetupPhase)
            {
                TArray<ATile*> FreeCells;
                for (auto& Elem : GameMode->GField->TileMap)
                {
                    ATile* Tile = Elem.Value;
                    if (Tile && Tile->GetTileStatus() == ETileStatus::EMPTY)
                    {
                        FreeCells.Add(Tile);
                    }
                }

                if (FreeCells.Num() > 0)
                {
                    int32 RandIdx = FMath::RandRange(0, FreeCells.Num() - 1);
                    ATile* SelectedTile = FreeCells[RandIdx];
                    FVector Location = GameMode->GField->GetRelativeLocationByXYPosition(
                        SelectedTile->GetGridPosition()[0],
                        SelectedTile->GetGridPosition()[1]);

                    GameMode->PlaceUnit(Location);
                    SelectedTile->SetTileStatus(PlayerNumber, ETileStatus::OCCUPIED);

                    UE_LOG(LogTemp, Warning, TEXT("AI ha piazzato un'unità su %s"), *SelectedTile->GetName());
                }

                return;
            }

            // ----- Turno normale -----
            TArray<ABaseUnit*> UnitsToMove;
            for (AActor* UnitActor : GameMode->AIUnits)
            {
                if (ABaseUnit* AiUnit = Cast<ABaseUnit>(UnitActor))
                {
                    AiUnit->bHasMoved = false;
                    AiUnit->bHasAttacked = false;
                    UnitsToMove.Add(AiUnit);
                }
            }
            if (GameMode->bDifficulty) // Se è medium difficulty
            {
                MoveNextAIUnitSmart(GameMode, UnitsToMove, 0);
            }
            else { // è easy altrimenti
                MoveNextAIUnit(GameMode, UnitsToMove, 0);
            }
        }, 2.0f, false);
}

void Awar_RandomPlayer::OnWin()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("AI (Random) Wins!"));
	GameInstance->SetTurnMessage(TEXT("AI Wins!"));
}

void Awar_RandomPlayer::OnLose()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("AI (Random) Loses!"));
	// GameInstance->SetTurnMessage(TEXT("AI Loses!"));
}
void Awar_RandomPlayer::MoveNextAIUnit(Awar_GameMode* GameMode, const TArray<ABaseUnit*>& Units, int32 Index)
{
    if (!GameMode || Index >= Units.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("Fine del turno dell'IA o GameMode non valido."));
        GameMode->TurnNextPlayer(); // Fine turno
        return;
    }

    ABaseUnit* AiUnit = Units[Index];
    if (!AiUnit)
    {
        UE_LOG(LogTemp, Warning, TEXT("Unità IA non valida all'indice %d."), Index);
        return;
    }

    // Funzione lambda per trovare un nemico in range ortogonale
    TFunction<ABaseUnit* (ATile*)> TrovaNemicoInRange = [=](ATile* DaTile) -> ABaseUnit*
        {
            int32 AttackRange = AiUnit->GetAttackRange();
            TQueue<TPair<FVector2D, int32>> Queue;
            TSet<FVector2D> Visited;
            TArray<ATile*> AttackTiles;

            FVector2D StartCoords = DaTile->GetGridPosition();
            Queue.Enqueue(TPair<FVector2D, int32>(StartCoords, 0));
            Visited.Add(StartCoords);

            while (!Queue.IsEmpty())
            {
                TPair<FVector2D, int32> Current;
                Queue.Dequeue(Current);

                FVector2D CurrentPos = Current.Key;
                int32 CurrentDist = Current.Value;

                if (CurrentDist >= AttackRange) continue;

                TArray<FVector2D> Neighbors = {
                    FVector2D(CurrentPos.X + 1, CurrentPos.Y),
                    FVector2D(CurrentPos.X - 1, CurrentPos.Y),
                    FVector2D(CurrentPos.X, CurrentPos.Y + 1),
                    FVector2D(CurrentPos.X, CurrentPos.Y - 1)
                };

                for (const FVector2D& NeighborPos : Neighbors)
                {
                    if (Visited.Contains(NeighborPos)) continue;

                    ATile* NeighborTile = GameMode->GField->TileMap.FindRef(NeighborPos);
                    if (NeighborTile)
                    {
                        Queue.Enqueue(TPair<FVector2D, int32>(NeighborPos, CurrentDist + 1));
                        Visited.Add(NeighborPos);
                        AttackTiles.Add(NeighborTile);
                    }
                }
            }

            for (ATile* Tile : AttackTiles)
            {
                ABaseUnit* Occupant = Tile->GetOccupyingUnit();
                if (Occupant && GameMode->IsUnitOwnedByPlayer(Occupant))
                {
                    return Occupant;
                }
            }

            return nullptr;
        };

    ATile* StartTile = GameMode->GField->GetTileAt(AiUnit->GetActorLocation());
    if (!StartTile)
    {
        UE_LOG(LogTemp, Warning, TEXT("StartTile non trovata per l'unità %s."), *AiUnit->GetName());
        return;
    }

    // Prova ad attaccare prima di muoversi
    ABaseUnit* EnemyBeforeMove = TrovaNemicoInRange(StartTile);
    if (EnemyBeforeMove)
    {
        UE_LOG(LogTemp, Warning, TEXT("Unità IA %s attacca %s senza muoversi."), *AiUnit->GetName(), *EnemyBeforeMove->GetName());
        AiUnit->AttackUnit(EnemyBeforeMove);
        AiUnit->bHasAttacked = true;
        MoveNextAIUnit(GameMode, Units, Index + 1);
        return;
    }

    // Nessun nemico vicino, si muove
    int32 MoveRange = AiUnit->GetMaxMoveRange();
    TArray<ATile*> WalkableTiles = GameMode->GField->GetTilesInRangeBFS(StartTile, MoveRange);

    if (WalkableTiles.Num() > 0)
    {
        ATile* DestinationTile = WalkableTiles[FMath::RandRange(0, WalkableTiles.Num() - 1)];
        UE_LOG(LogTemp, Warning, TEXT("Unità IA %s si muove verso la tile %s."), *AiUnit->GetName(), *DestinationTile->GetName());
        GameMode->MoveUnitToTile(AiUnit, DestinationTile);

        FTimerHandle NextTimer;
        GetWorld()->GetTimerManager().SetTimer(NextTimer, [=, this]()
            {
                ABaseUnit* EnemyAfterMove = TrovaNemicoInRange(DestinationTile);
                if (EnemyAfterMove)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Unità IA %s attacca %s dopo il movimento."), *AiUnit->GetName(), *EnemyAfterMove->GetName());
                    AiUnit->AttackUnit(EnemyAfterMove);
                    AiUnit->bHasAttacked = true;
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Nessun nemico in range dopo il movimento per l'unità IA %s."), *AiUnit->GetName());
                }

                MoveNextAIUnit(GameMode, Units, Index + 1);
            }, 1.0f, false);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Nessuna tile percorribile per l'unità IA %s."), *AiUnit->GetName());
        MoveNextAIUnit(GameMode, Units, Index + 1);
    }
}
void Awar_RandomPlayer::MoveNextAIUnitSmart(Awar_GameMode* GameMode, const TArray<ABaseUnit*>& Units, int32 Index)
{
    if (!GameMode || Index >= Units.Num())
    {
        if (GameMode) GameMode->TurnNextPlayer(); 
        return;
    }

    ABaseUnit* AiUnit = Units[Index];
    if (!AiUnit)
    {
        MoveNextAIUnitSmart(GameMode, Units, Index + 1);
        return;
    }

    ATile* StartTile = GameMode->GField->GetTileAt(AiUnit->GetActorLocation());
    if (!StartTile)
    {
        UE_LOG(LogTemp, Warning, TEXT("IA: StartTile null"));
        MoveNextAIUnitSmart(GameMode, Units, Index + 1);
        return;
    }

    // Trova la mia unità più vicina
    ABaseUnit* NearestPlayerUnit = nullptr;
    int32 BestManhattan = INT_MAX;
    ATile* PlayerTile = nullptr;

    for (ABaseUnit* PU : GameMode->PlayerUnits)
    {
        if (!PU) continue;
        ATile* T = GameMode->GField->GetTileAt(PU->GetActorLocation());
        if (!T) continue;
        FVector2D StartPos = StartTile->GetGridPosition();
        FVector2D TargetPos = T->GetGridPosition();
        int32 d = FMath::Abs((int32)TargetPos.X - (int32)StartPos.X) + FMath::Abs((int32)TargetPos.Y - (int32)StartPos.Y);
        if (d < BestManhattan)
        {
            BestManhattan = d;
            NearestPlayerUnit = PU;
            PlayerTile = T;
        }
    }

    if (!NearestPlayerUnit || !PlayerTile)
    {
        UE_LOG(LogTemp, Warning, TEXT("IA: No nearest player unit found"));
        MoveNextAIUnitSmart(GameMode, Units, Index + 1);
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("IA: nearest player unit = %s  dist = %d"), *NearestPlayerUnit->GetName(), BestManhattan);

    // Trova le tile adiacenti alla PlayerTile che siano vuote
    TArray<ATile*> Candidates;
    FVector2D P = PlayerTile->GetGridPosition();
    TArray<FVector2D> Dirs = { {1,0},{-1,0},{0,1},{0,-1} };
    for (auto& d : Dirs)
    {
        FVector2D np = P + d;
        ATile** Tp = GameMode->GField->TileMap.Find(np);
        if (Tp && *Tp && (*Tp)->GetTileStatus() == ETileStatus::EMPTY)
            Candidates.Add(*Tp);
    }

    if (Candidates.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("IA: Nessuna tile adiacente libera"));
        MoveNextAIUnitSmart(GameMode, Units, Index + 1);
        return;
    }

    // Path più corto
    TArray<ATile*> BestPath;
    for (ATile* Cand : Candidates)
    {
        if (!Cand) continue;  // <-- aggiunto
        TArray<ATile*> path = GameMode->GField->FindPath(StartTile, Cand);
        if (path.Num() > 0 && (BestPath.Num() == 0 || path.Num() < BestPath.Num()))
            BestPath = path;
    }

    if (BestPath.Num() <= 1)
    {
        UE_LOG(LogTemp, Warning, TEXT("IA: Nessun path utile"));
        MoveNextAIUnitSmart(GameMode, Units, Index + 1);
        return;
    }

    // riduco il path, in modo che sia uguale al max move range
    int32 MoveRange = AiUnit->GetMaxMoveRange();
    int32 LastIndex = FMath::Min(MoveRange, BestPath.Num() - 1);
    ATile* Dest = BestPath[LastIndex];

    if (!Dest)
    {
        UE_LOG(LogTemp, Warning, TEXT("IA: Dest Tile è null"));
        MoveNextAIUnitSmart(GameMode, Units, Index + 1);
        return;
    }
    // funzione per trovare nemico nel range di attacco
    TFunction<ABaseUnit* (ATile*)> TrovaNemicoInRange = [=](ATile* DaTile) -> ABaseUnit*
        {
            if (!DaTile) return nullptr; // <-- aggiunto
            int32 AttackRange = AiUnit->GetAttackRange();
            TQueue<TPair<FVector2D, int32>> Queue;
            TSet<FVector2D> Visited;
            TArray<ATile*> AttackTiles;

            FVector2D StartCoords = DaTile->GetGridPosition();
            Queue.Enqueue(TPair<FVector2D, int32>(StartCoords, 0));
            Visited.Add(StartCoords);

            while (!Queue.IsEmpty())
            {
                TPair<FVector2D, int32> Current;
                Queue.Dequeue(Current);

                FVector2D CurrentPos = Current.Key;
                int32 CurrentDist = Current.Value;
                if (CurrentDist >= AttackRange) continue;

                TArray<FVector2D> Neighbors = {
                    FVector2D(CurrentPos.X + 1, CurrentPos.Y),
                    FVector2D(CurrentPos.X - 1, CurrentPos.Y),
                    FVector2D(CurrentPos.X, CurrentPos.Y + 1),
                    FVector2D(CurrentPos.X, CurrentPos.Y - 1)
                };

                for (const FVector2D& NeighborPos : Neighbors)
                {
                    if (Visited.Contains(NeighborPos)) continue;

                    ATile* NeighborTile = GameMode->GField->TileMap.FindRef(NeighborPos);
                    if (NeighborTile)
                    {
                        Queue.Enqueue(TPair<FVector2D, int32>(NeighborPos, CurrentDist + 1));
                        Visited.Add(NeighborPos);
                        AttackTiles.Add(NeighborTile);
                    }
                }
            }

            for (ATile* Tile : AttackTiles)
            {
                if (!Tile) continue; // <-- aggiunto
                ABaseUnit* Occupant = Tile->GetOccupyingUnit();
                if (Occupant && GameMode->IsUnitOwnedByPlayer(Occupant))
                {
                    UE_LOG(LogTemp, Warning, TEXT("IA: Nemico trovato in range -> %s"), *Occupant->GetName());
                    return Occupant;
                }
            }

            UE_LOG(LogTemp, Warning, TEXT("IA: Nessun nemico in range"));
            return nullptr;
        };
    // attacco se c'è qualcuno in range
    ATile* CurrentTile = GameMode->GField->GetTileAt(AiUnit->GetActorLocation());
    if (CurrentTile)
    {
        ABaseUnit* Target = TrovaNemicoInRange(CurrentTile);
        if (Target)
        {
            UE_LOG(LogTemp, Warning, TEXT("IA: nemico già in range, attacco %s senza muovermi"), *Target->GetName());
            AiUnit->AttackUnit(Target);
            AiUnit->bHasAttacked = true;
            MoveNextAIUnitSmart(GameMode, Units, Index + 1);
            return;
        }
    }
    // movimento
    GameMode->MoveUnitToTile(AiUnit, Dest);

    // Delay e attacco
    FTimerHandle Th;
    GetWorld()->GetTimerManager().SetTimer(Th, [=, this]()
        {
            ATile* newStart = GameMode->GField->GetTileAt(AiUnit->GetActorLocation());
            if (!newStart)
            {
                UE_LOG(LogTemp, Warning, TEXT("IA: newStart null dopo il movimento"));
                MoveNextAIUnitSmart(GameMode, Units, Index + 1);
                return;
            }

           

            if (ABaseUnit* e = TrovaNemicoInRange(newStart))
            {
                UE_LOG(LogTemp, Warning, TEXT("IA: Attacco %s"), *e->GetName());
                AiUnit->AttackUnit(e);
                AiUnit->bHasAttacked = true;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("IA: Nessun nemico da attaccare"));
            }

            MoveNextAIUnitSmart(GameMode, Units, Index + 1);

        }, 0.5f, false);
}
