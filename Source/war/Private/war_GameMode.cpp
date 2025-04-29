// Fill out your copyright notice in the Description page of Project Settings.


#include "war_GameMode.h"
#include "war_PlayerController.h"
#include "war_RandomPlayer.h"

#include "EngineUtils.h"
#include "Kismet/KismetMathLibrary.h"
#include <Components/TextBlock.h>

Awar_GameMode::Awar_GameMode()
{
	PlayerControllerClass = Awar_PlayerController::StaticClass();
	DefaultPawnClass = Awar_HumanPlayer::StaticClass();
	FieldSize = 25;
	GameInstance = Cast<Uwar_GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
}


void Awar_GameMode::BeginPlay()
{
	Super::BeginPlay();

	MoveCounter = 0;

	bGameOver = false;
	//ATTT_HumanPlayer* HumanPlayer = *TActorIterator<ATTT_HumanPlayer>(GetWorld());
	FTimerHandle TimerHandle;

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &Awar_GameMode::CheckPlayerController, 1.0f, true);
	Awar_HumanPlayer* HumanPlayer = GetWorld()->GetFirstPlayerController()->GetPawn<Awar_HumanPlayer>();

	if (!IsValid(HumanPlayer))
	{
		UE_LOG(LogTemp, Error, TEXT("No player pawn of type '%s' was found."), *Awar_HumanPlayer::StaticClass()->GetName());
		return;
	}

	if (GameFieldClass != nullptr)
	{
		GField = GetWorld()->SpawnActor<AGameField>(GameFieldClass);
		GField->Size = FieldSize;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Game Field is null"));
	}

	float CameraPosX = ((GField->TileSize * FieldSize) + ((FieldSize - 1) * GField->TileSize * GField->CellPadding)) * 0.5f;
	float Zposition = 145.0f;
	FVector CameraPos(CameraPosX, CameraPosX, Zposition);
	HumanPlayer->SetActorLocationAndRotation(CameraPos, FRotationMatrix::MakeFromX(FVector(0, 0, -1)).Rotator());

	// Alternativamente, per avere una telecamera fissa in scena:
	// 1. Aggiungere un Camera Actor nella scena
	// 2. Impostare "Player 0" nel campo "Auto Activate for Player"
	// 3. Togliere la spunta al campo "Constrain Aspect Ratio"
	// 4. Posizionatela e orientatela a vostro piacimento


	// Human player = 0
	Players.Add(HumanPlayer);
	// Random Player
	auto* AI = GetWorld()->SpawnActor<Awar_RandomPlayer>(FVector(), FRotator());

	// MiniMax Player
	//auto* AI = GetWorld()->SpawnActor<ATTT_MinimaxPlayer>(FVector(), FRotator());

	// AI player = 1
	Players.Add(AI);

	this->ChoosePlayerAndStartGame();
}

void Awar_GameMode::ChoosePlayerAndStartGame()
{
	CurrentPlayer = FMath::RandRange(0, Players.Num() - 1);
	bIsPlayer1Turn = (CurrentPlayer == 0);

    for (int32 IndexI = 0; IndexI < Players.Num(); IndexI++)
    {
        Players[IndexI]->PlayerNumber = IndexI;
        Players[IndexI]->Sign = IndexI == CurrentPlayer ? ESign::A : ESign::B;
    }

    MoveCounter = 1;
    bSetupPhase = true;
    UnitsPlaced = 0;
    SelectedUnitClass = nullptr; // Assicuriamoci che sia null

	UE_LOG(LogTemp, Warning, TEXT("Il primo giocatore è: %s"), (bIsPlayer1Turn ? TEXT("Giocatore umano") : TEXT("AI")));

    Awar_HumanPlayer* HumanPlayer = Cast<Awar_HumanPlayer>(Players[CurrentPlayer]);
    if (HumanPlayer)
    {
        // GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Seleziona un'unità da piazzare!"));
        return; // Aspetta la selezione prima di continuare
    }

    Players[CurrentPlayer]->OnTurn(); // Solo se è l'AI, inizia subito
}

void Awar_GameMode::SelectUnitToPlace(UClass* UnitClass)
{
	if (bIsPlayer1Turn && bSetupPhase)
	{
		SelectedUnitClass = UnitClass;
		UE_LOG(LogTemp, Warning, TEXT("Unità selezionata: %s"), *UnitClass->GetName());
	}
}

void Awar_GameMode::PlaceUnit(const FVector& SpawnPosition)
{
	UClass* UnitClass = nullptr;

	if (bIsPlayer1Turn)
	{
		if (!SelectedUnitClass)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Nessuna unità selezionata!"));
			return;
		}
		UnitClass = SelectedUnitClass;
		SelectedUnitClass = nullptr; // Reset per evitare ripiazzamenti errati
	}
	else
	{
		// AI piazza le sue unità fisse
		UnitClass = (AIUnits.Num() == 0) ? SniperB : BrawlerB;
	}

	if (!UnitClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Errore: Nessuna unità disponibile da piazzare."));
		return;
	}

	FVector Location = GField->GetActorLocation() + SpawnPosition + FVector(0, 0, 1); // Z sopra le tile di 1
	AActor* SpawnedUnit = GetWorld()->SpawnActor<AActor>(UnitClass, Location, FRotator::ZeroRotator);
	if (GField)
	{
		FVector2D GridCoords = GField->GetXYPositionByRelativeLocation(SpawnPosition);
		ATile** TilePtr = GField->TileMap.Find(GridCoords);
		ATile* Tile = (TilePtr != nullptr) ? *TilePtr : nullptr;
		if (Tile)
		{
			ABaseUnit* SpawnedBaseUnit = Cast<ABaseUnit>(SpawnedUnit);
			if (SpawnedBaseUnit)
			{
				Tile->SetOccupyingUnit(SpawnedBaseUnit);  // Assegna l'unità alla tile
				SpawnedBaseUnit->SetGridPosition(GridCoords);  // Imposta la posizione della unità
				Tile->SetTileStatus(CurrentPlayer, ETileStatus::OCCUPIED); // Segna la tile come occupata
			}
		}
	}
	
	// Calcola la scala in base alla dimensione della Tile

	float PawnScaleFactor = 5 / 100.f; 
	FVector NewScale = FVector(PawnScaleFactor, PawnScaleFactor, 0.1f); 

	// Applica la scala al Pawn
	SpawnedUnit->SetActorScale3D(NewScale);

	if (!SpawnedUnit)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnActor ha restituito nullptr! Forse c'è una collisione."));
		return;
	}

	// Salva l'unità nel team corretto
	if (bIsPlayer1Turn)
	{
		PlayerUnits.Add(Cast<ABaseUnit>(SpawnedUnit));
	}
	else
	{
		AIUnits.Add(Cast<ABaseUnit>(SpawnedUnit));
	}

	UnitsPlaced++;
	UE_LOG(LogTemp, Warning, TEXT("Unità piazzata, cambio turno..."));
	TurnNextPlayer(); // Cambio turno
	UE_LOG(LogTemp, Warning, TEXT("Nuovo turno per il giocatore: %d"), CurrentPlayer);
	if (UnitsPlaced >= 4) // Setup completo
	{
		bSetupPhase = false;
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Iniziamo il gioco!"));
		UE_LOG(LogTemp, Warning, TEXT("Finito il piazzamento delle unità"));
		GameInstance->UpdateUnitStatsText();
	}
	//Players[CurrentPlayer]->OnTurn();
}

int32 Awar_GameMode::GetNextPlayer(int32 Player)
{
	Player++;
	if (!Players.IsValidIndex(Player))
	{
		Player = 0;
	}
	return Player;
}

void Awar_GameMode::TurnNextPlayer()
{
	SelectedUnit = nullptr;
	
	MoveCounter += 1;
	CurrentPlayer = GetNextPlayer(CurrentPlayer);
	bIsPlayer1Turn = (CurrentPlayer == 0); // Se il nuovo giocatore è 0, tocca al player umano

	UE_LOG(LogTemp, Warning, TEXT("Ora è il turno del giocatore: %d"), CurrentPlayer);
	for (ABaseUnit* Unit : PlayerUnits) // PlayerUnits è una lista delle unità del giocatore
	{
		Unit->bHasMoved = false;
		Unit->bHasAttacked = false;
	}

	UE_LOG(LogTemp, Warning, TEXT("Unità resettate per il nuovo turno."));
	Players[CurrentPlayer]->OnTurn();
}

void Awar_GameMode::IsGameOver()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (PlayerUnits.Num() == 0) {
			Awar_HumanPlayer* HumanPlayer = Cast<Awar_HumanPlayer>(PC->GetPawn());
			HumanPlayer->OnLose();
			bGameOver = true;
			return;
		}
		if (AIUnits.Num() == 0)
		{
			Awar_HumanPlayer* HumanPlayer = Cast<Awar_HumanPlayer>(PC->GetPawn());
			HumanPlayer->OnWin();
			bGameOver = true;
			return;
		}
	}
}
void Awar_GameMode::CheckPlayerController()
{
	FTimerHandle TimerHandle;
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (Awar_HumanPlayer* HumanPlayer = Cast<Awar_HumanPlayer>(PC->GetPawn()))
		{
			//UE_LOG(LogTemp, Warning, TEXT("HumanPlayer trovato: %s"), *HumanPlayer->GetName());
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		}
	}
}

void Awar_GameMode::SetSelectedUnit(APawn* Unit) { 
	SelectedUnit = Unit; 
}

APawn* Awar_GameMode::GetSelectedUnit() const {
	return SelectedUnit; 
}

void Awar_GameMode::SetDifficulty(bool bDiff)
{
	bDifficulty = bDiff;
}

bool Awar_GameMode::GetDifficulty() const
{
	return bDifficulty;
}

bool Awar_GameMode::IsUnitOwnedByPlayer(ABaseUnit* Unit)
{
	if (!Unit) return false;

	// Controlla se l'unità appartiene al giocatore corrente
	return PlayerUnits.Contains(Unit);
}
bool Awar_GameMode::AreAllPlayerUnitsDone() const
{
	// Supponiamo che PlayerUnits contenga le unità umane
	for (ABaseUnit* Unit : PlayerUnits)
	{
		if (Unit && (!Unit->bHasMoved || !Unit->bHasAttacked))
		{
			return false; // Almeno un'unità non ha eseguito tutte le azioni
		}
	}
	return true;
}
void Awar_GameMode::MoveUnitToTile(ABaseUnit* Unit, ATile* TargetTile)
{
	if (!Unit || !TargetTile || !GField) return;

	ATile* StartTile = GField->GetTileAt(Unit->GetActorLocation());
	if (!StartTile) return;

	// Calcolo il path ortogonale
	TArray<ATile*> Path = GField->FindPath(StartTile, TargetTile);
	if (Path.Num() <= 1) return;

	// Pulisci lo stato della tile iniziale
	StartTile->SetTileStatus(-1, ETileStatus::EMPTY);
	StartTile->SetOccupyingUnit(nullptr);

	// Imposta il path di movimento nell'unità
	Unit->MovementPath = Path;
	Unit->CurrentPathIndex = 0;
	Unit->bIsMoving = true;

	// Imposta la tile finale target (per aggiornare lo stato a fine movimento)
	Unit->TargetTile = TargetTile;

	// Aggiorno la posizione dell'unità
	FVector2D GridCoords = GField->GetXYPositionByRelativeLocation(TargetTile->GetActorLocation());
	Unit->SetGridPosition(GridCoords);

	// Disabilita input se serve
	Unit->SetActorTickEnabled(true); // Tick usato per animare movimento

	FString FromLabel = GField->CoordsToLabel(StartTile->GetGridPosition());
	FString ToLabel = GField->CoordsToLabel(TargetTile->GetGridPosition());
	FString Name = *Unit->GetName();
	if (Name == "BP_SniperA_C_0")
		Name = "Sniper";
	if (Name == "BP_BrawlerA_C_0")
		Name = "Brawler";
	if (Name == "BP_SniperB_C_0")
		Name = "Sniper";
	if (Name == "BP_BrawlerB_C_0")
		Name = "Brawler";
	FString MoveText = FString::Printf(TEXT("\n%s Move % s -> % s"),
		 *Name,*FromLabel, *ToLabel);
	FString Log = GameInstance->GetMessageLog();
	GameInstance->SetMessageLog(MoveText+Log);

	// Deseleziona l'unità
	SetSelectedUnit(nullptr);
}
