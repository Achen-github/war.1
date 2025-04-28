// Fill out your copyright notice in the Description page of Project Settings.


#include "GameField.h"
#include "Kismet/GameplayStatics.h"
#include "war_GameMode.h"
#include "war_HumanPlayer.h"
#include "Containers/Queue.h"
// Sets default values
AGameField::AGameField()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	// size of the field (25x25)
	Size = 25;
	// tile dimension
	TileSize = 10.f;
	// tile padding percentage 
	CellPadding = 0.2f;
	GameInstance = Cast<Uwar_GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
}


void AGameField::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	//normalized tilepadding
	NextCellPositionMultiplier = (TileSize + TileSize * CellPadding) / TileSize;
}

// Called when the game starts or when spawned
void AGameField::BeginPlay()
{
	Super::BeginPlay();
	
	GenerateField();
	GenerateObstacles();
	for (const auto& Elem : TileMap)
	{
		UE_LOG(LogTemp, Warning, TEXT("TileMap contiene: X=%d Y=%d Stato=%d"),
			static_cast<int32>(Elem.Key.X),
			static_cast<int32>(Elem.Key.Y),
			static_cast<int32>(Elem.Value->GetTileStatus()));
	}
	

	/*for (auto& Elem : ObsMap)
	{
		UE_LOG(LogTemp, Warning, TEXT("Ostacolo posizionato su: %s, Stato: %d"), *Elem.Value->GetName(), Elem.Value->GetTileStatus());
	}*/
}


void AGameField::ResetField()
{

	Awar_GameMode* GameMode = Cast<Awar_GameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode) return;

	GameMode->GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	for (ABaseUnit* U : GameMode->AIUnits) {
		GetWorld()->GetTimerManager().ClearAllTimersForObject(U);
	}
	UWorld* World = GetWorld();
	if (World)
	{
		FName CurrentLevel = *World->GetName();
		UGameplayStatics::OpenLevel(World, CurrentLevel);
	}
	GameInstance->SetEndMessage(TEXT(""));
	GameInstance->SetMessageLog(TEXT(""));
	GameInstance->ResetUnitStatsText();
	// la soluzione inferiore presenta qualche bug

	////  Distruggi tutte le unità
	//for (AActor* Unit : GameMode->PlayerUnits)
	//{
	//	if (Unit)
	//	{
	//		Unit->Destroy();
	//	}
	//}
	//GameMode->PlayerUnits.Empty();

	//for (AActor* Unit : GameMode->AIUnits)
	//{
	//	if (Unit)
	//	{
	//		Unit->Destroy();
	//	}
	//}
	//GameMode->AIUnits.Empty();

	//// Distruggi tutti gli ostacoli
	//for (AActor* Obstacle : ObsArray)
	//{
	//	if (Obstacle)
	//	{
	//		Obstacle->Destroy();
	//	}
	//}
	//ObsArray.Empty();

	////  Libera tutte le tile
	//for (ATile* Tile : TileArray)
	//{
	//	if (Tile)
	//	{
	//		Tile->SetTileStatus(-1, ETileStatus::EMPTY);
	//		Tile->SetOccupyingUnit(nullptr);
	//	}
	//}
	//GenerateField();
	//// Rigenera nuovi ostacoli
	//GenerateObstacles();

	//// Notifica gli altri sistemi che il campo è stato resettato
	//OnResetEvent.Broadcast();

	//// Reset logica di gioco
	//GameMode->MoveCounter = 0;
	//GameMode->bGameOver = false;
	//GameMode->ChoosePlayerAndStartGame();
}
void AGameField::GenerateField()
{
	for (int32 IndexX = 0; IndexX < Size; IndexX++)
	{
		for (int32 IndexY = 0; IndexY < Size; IndexY++)
		{
			FVector Location = AGameField::GetRelativeLocationByXYPosition(IndexX, IndexY);
			ATile* Obj = GetWorld()->SpawnActor<ATile>(TileClass, Location, FRotator::ZeroRotator);
			const float TileScale = TileSize / 100.f;
			const float Zscaling = 0.2f;
			Obj->SetActorScale3D(FVector(TileScale, TileScale, Zscaling));
			Obj->SetGridPosition(IndexX, IndexY);
			TileArray.Add(Obj);
			TileMap.Add(FVector2D(IndexX, IndexY), Obj);
			Obj->SetTileStatus(-1, ETileStatus::EMPTY);
		}
	}
	
}

//bfs per vedere se la mappa è aperta (senza zone chiuse)
bool AGameField::BFS(TArray<TArray<bool>>& Grid) 
{
	TArray<TArray<int32>> RegionMap;
	RegionMap.SetNum(Size);

	for (int32 i = 0; i < Size; i++)
	{
		RegionMap[i].SetNum(Size);
		for (int32 j = 0; j < Size; j++)
		{
			RegionMap[i][j] = -1; // Inizializziamo tutte le celle senza etichetta
		}
	}

	int32 RegionID = 0;
	TMap<int32, TArray<FVector2D>> Regions;

	// Troviamo le regioni isolate usando BFS
	for (int32 i = 0; i < Size; i++)
	{
		for (int32 j = 0; j < Size; j++)
		{
			if (!Grid[i][j] && RegionMap[i][j] == -1) // Tile libera e non ancora etichettata
			{
				TArray<FVector2D> RegionCells;
				TQueue<FVector2D> Queue;
				Queue.Enqueue(FVector2D(i, j));
				RegionMap[i][j] = RegionID;

				while (!Queue.IsEmpty())
				{
					FVector2D Cell;
					Queue.Dequeue(Cell);
					RegionCells.Add(Cell);

					TArray<FVector2D> Neighbors = {
						FVector2D(Cell.X + 1, Cell.Y),
						FVector2D(Cell.X - 1, Cell.Y),
						FVector2D(Cell.X, Cell.Y + 1),
						FVector2D(Cell.X, Cell.Y - 1)
					};

					for (FVector2D Neighbor : Neighbors)
					{
						if (Neighbor.X >= 0 && Neighbor.X < Size &&
							Neighbor.Y >= 0 && Neighbor.Y < Size &&
							!Grid[Neighbor.X][Neighbor.Y] &&
							RegionMap[Neighbor.X][Neighbor.Y] == -1)
						{
							Queue.Enqueue(Neighbor);
							RegionMap[Neighbor.X][Neighbor.Y] = RegionID;
						}
					}
				}

				Regions.Add(RegionID, RegionCells);
				RegionID++;
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Regioni: %d"), Regions.Num());
	if (Regions.Num() == 1)
	{
		return true; // La griglia è già connessa
	}
	return false;
}

void AGameField::GenerateObstacles()
{
	const int32 TotalCells = Size * Size;
	int32 NumObstacles = TotalCells * 0.2; // 20% delle celle, ricreo la mappa finchè mi viene una senza isole, piu è alta la percentuale piu ci mette a crearla a runtime
	TSubclassOf<ATile> ObstacleTypes[3] = { ObstacleTree1, ObstacleTree2, ObstacleMountain };
	TArray<TArray<bool>> Grid;
	Grid.SetNum(Size);
	do
	{
	for (int32 i = 0; i < Size; i++)
		Grid[i].Init(false, Size); // false = libero, true = ostacolo

	// Selezioniamo 125 celle casuali per gli ostacoli
	TArray<FVector2D> FreeCells;
	for (int32 x = 0; x < Size; x++) {
		for (int32 y = 0; y < Size; y++) {
			FreeCells.Add(FVector2D(x, y));
		}
	}
		ShuffleArray(FreeCells); // Mescoliamo l'array
		for (int32 i = 0; i < NumObstacles; i++)
		{
			FVector2D Cell = FreeCells[i];
			Grid[Cell.X][Cell.Y] = true;
		}
	} while (!BFS(Grid));


	// Spawn degli ostacoli
	for (int32 x = 0; x < Size; x++)
	{
		for (int32 y = 0; y < Size; y++)
		{
			if (Grid[x][y]) // Se è un ostacolo
			{
				FVector Location = AGameField::GetRelativeLocationByXYPosition(x, y);
				TSubclassOf<ATile> ChosenClass = ObstacleTypes[FMath::RandRange(0, 2)];

				ATile* Obs = GetWorld()->SpawnActor<ATile>(ChosenClass, Location, FRotator::ZeroRotator);
				if (Obs)
				{
					const float TileScale = TileSize / 100.f;
					const float Zscaling = 0.2f;
					Obs->SetActorScale3D(FVector(TileScale, TileScale, Zscaling));
					Obs->SetGridPosition(x, y);
					TileMap.Add(FVector2D(x, y), Obs);
					Obs->SetTileStatus(-1, ETileStatus::OCCUPIED);
				}
			}
		}
	}
	int32 OccupiedCount = 0;
	for (auto& Elem : TileMap)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Ostacolo su: %s"), *Elem.Value->GetName());
		if (Elem.Value->GetTileStatus() == (-1,ETileStatus::OCCUPIED))
		{
			OccupiedCount++;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Tile occupate totali: %d"), OccupiedCount);
}
void AGameField::ShuffleArray(TArray<FVector2D>& Array)
{
	int32 LastIndex = Array.Num() - 1;
	for (int32 i = LastIndex; i > 0; --i)
	{
		int32 RandomIndex = FMath::RandRange(0, i);
		Array.Swap(i, RandomIndex);
	}
}
FVector2D AGameField::GetPosition(const FHitResult& Hit)
{
	return Cast<ATile>(Hit.GetActor())->GetGridPosition();
}

TArray<ATile*>& AGameField::GetTileArray()
{
	return TileArray;
}

FVector AGameField::GetRelativeLocationByXYPosition(const int32 InX, const int32 InY) const
{
	return TileSize * NextCellPositionMultiplier * FVector(InX, InY, 0);
}

FVector2D AGameField::GetXYPositionByRelativeLocation(const FVector& Location) const
{
	const double XPos = Location.X / (TileSize * NextCellPositionMultiplier);
	const double YPos = Location.Y / (TileSize * NextCellPositionMultiplier);
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("x=%f,y=%f"), XPos, YPos));
	return FVector2D(XPos, YPos);
}
ATile* AGameField::GetTileAtPosition(FVector2D Position)
{
	for (ATile* Tile : TileArray)
	{
		if (Tile->GetGridPosition() == Position)
		{
			return Tile;
		}
	}
	return nullptr; // Nessuna tile trovata
}

inline bool AGameField::IsValidPosition(const FVector2D Position) const
{
	return 0 <= Position.X && Position.X < Size && 0 <= Position.Y && Position.Y < Size;
}


ATile* AGameField::GetTileAt(FVector Location)
{
	for (ATile* Tile : TileArray)
	{
		if (FVector::Dist(Tile->GetActorLocation(), Location) < TileSize * 0.5f)
		{
			return Tile;
		}
	}
	return nullptr;
}

//mi serve per evidenziare le celle raggiungibili
TArray<ATile*> AGameField::GetTilesInRangeBFS(ATile* StartTile, int32 MaxDistance)
{
	TArray<ATile*> ReachableTiles;
	if (!StartTile) return ReachableTiles;

	FVector2D StartPos = GetXYPositionByRelativeLocation(StartTile->GetActorLocation());

	TQueue<TPair<FVector2D, int32>> Queue;
	TMap<FVector2D, int32> Visited;  // Tiene traccia della distanza dalle tile visitate

	Queue.Enqueue(TPair<FVector2D, int32>(StartPos, 0));
	Visited.Add(StartPos, 0);

	while (!Queue.IsEmpty())
	{
		TPair<FVector2D, int32> Current;
		Queue.Dequeue(Current);

		FVector2D CurrentPos = Current.Key;
		int32 CurrentDist = Current.Value;

		// Se siamo fuori dal range, interrompiamo l'espansione
		if (CurrentDist >= MaxDistance) continue;
		//UE_LOG(LogTemp, Warning, TEXT("BFS: Analizzando Tile a X=%d, Y=%d, Distanza=%d"),
			//static_cast<int32>(CurrentPos.X),
			//static_cast<int32>(CurrentPos.Y),
			//CurrentDist);
		// Controlliamo le 4 direzioni (NO diagonali)
		TArray<FVector2D> Neighbors = {
			FVector2D(CurrentPos.X + 1, CurrentPos.Y),
			FVector2D(CurrentPos.X - 1, CurrentPos.Y),
			FVector2D(CurrentPos.X, CurrentPos.Y + 1),
			FVector2D(CurrentPos.X, CurrentPos.Y - 1)
		};

		for (const FVector2D& NeighborPos : Neighbors)
		{
			if (!TileMap.Contains(NeighborPos))
			{
				//UE_LOG(LogTemp, Error, TEXT("Tile non trovata a X=%d, Y=%d"),
					//static_cast<int32>(NeighborPos.X),
					//static_cast<int32>(NeighborPos.Y));
			}
			// Se la tile è già stata visitata, la ignoriamo
			if (Visited.Contains(NeighborPos)) continue;

			// Troviamo la tile nella mappa
			ATile* NeighborTile = TileMap.FindRef(NeighborPos);
			if (NeighborTile)
			{
				ETileStatus TileState = NeighborTile->GetTileStatus();
				//UE_LOG(LogTemp, Warning, TEXT("Tile trovata a X=%d, Y=%d, Stato=%d"),
					//static_cast<int32>(NeighborPos.X),
					//static_cast<int32>(NeighborPos.Y),
					//static_cast<int32>(TileState));

				if (TileState == ETileStatus::EMPTY)
				{
					Queue.Enqueue(TPair<FVector2D, int32>(NeighborPos, CurrentDist + 1));
					Visited.Add(NeighborPos, CurrentDist + 1);
					ReachableTiles.Add(NeighborTile);
				}
				else
				{
					/*UE_LOG(LogTemp, Error, TEXT("Tile bloccata a X=%d, Y=%d, Stato=%d"),
						static_cast<int32>(NeighborPos.X),
						static_cast<int32>(NeighborPos.Y),
						static_cast<int32>(TileState));*/
				}
			}
			else
			{
				//UE_LOG(LogTemp, Error, TEXT("Tile non trovata a X=%d, Y=%d"),
				//	static_cast<int32>(NeighborPos.X),
				//	static_cast<int32>(NeighborPos.Y));
			}
			
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Tile trovate nel range: %d"), ReachableTiles.Num());

	return ReachableTiles;
}
//mi serve per il movimento delle unità
TArray<ATile*> AGameField::FindPath(ATile* StartTile, ATile* EndTile)
{
	TArray<ATile*> EmptyResult;
	if (!StartTile || !EndTile)
	{
		UE_LOG(LogTemp, Warning, TEXT("BFS: StartTile o EndTile nulli"));
		return EmptyResult;
	}

	UE_LOG(LogTemp, Warning, TEXT("BFS: Calcolo percorso da %s (X=%.0f,Y=%.0f) a %s (X=%.0f,Y=%.0f)"),
		*StartTile->GetName(),
		StartTile->GetGridPosition().X, StartTile->GetGridPosition().Y,
		*EndTile->GetName(),
		EndTile->GetGridPosition().X, EndTile->GetGridPosition().Y);

	TMap<ATile*, ATile*> CameFrom;
	TQueue<ATile*> Queue;
	TSet<ATile*> Visited;

	Queue.Enqueue(StartTile);
	Visited.Add(StartTile);

	while (!Queue.IsEmpty())
	{
		ATile* Current = nullptr;
		Queue.Dequeue(Current);
		check(Current);
		FVector2D CurrPos = Current->GetGridPosition();
		UE_LOG(LogTemp, Warning, TEXT("BFS: Visito tile %s (X=%.0f,Y=%.0f)"),
			*Current->GetName(), CurrPos.X, CurrPos.Y);

		if (Current == EndTile)
		{
			// ricostruzione del percorso
			TArray<ATile*> Path;
			for (ATile* It = EndTile; It != nullptr; It = CameFrom.FindRef(It))
			{
				Path.Insert(It, 0);
			}
			UE_LOG(LogTemp, Warning, TEXT("BFS: Percorso trovato di lunghezza %d"), Path.Num());
			return Path;
		}

		// esploro vicini (no diagonali)
		const FVector2D Dirs[4] = {
			{  1,  0 },
			{ -1,  0 },
			{  0,  1 },
			{  0, -1 }
		};
		for (const FVector2D& d : Dirs)
		{
			FVector2D NPos = CurrPos + d;
			if (!TileMap.Contains(NPos))
			{
				UE_LOG(LogTemp, Warning, TEXT("BFS: Pos (%f,%f) fuori mappa"), NPos.X, NPos.Y);
				continue;
			}

			ATile* Neighbor = TileMap.FindRef(NPos);
			if (!Neighbor)
			{
				UE_LOG(LogTemp, Warning, TEXT("BFS: Tile a (%f,%f) è nullptr"), NPos.X, NPos.Y);
				continue;
			}

			if (Visited.Contains(Neighbor))
			{
				// già visitata
				continue;
			}

			if (Neighbor->GetTileStatus() != ETileStatus::EMPTY)
			{
				UE_LOG(LogTemp, Warning, TEXT("BFS: Tile %s (X=%.0f,Y=%.0f) OCCUPATA, scarto"),
					*Neighbor->GetName(), NPos.X, NPos.Y);
				continue;
			}

			// la aggiungo alla coda
			Queue.Enqueue(Neighbor);
			Visited.Add(Neighbor);
			CameFrom.Add(Neighbor, Current);
			UE_LOG(LogTemp, Warning, TEXT("BFS: Incodo tile %s (X=%.0f,Y=%.0f)"),
				*Neighbor->GetName(), NPos.X, NPos.Y);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("BFS: Nessun percorso trovato"));
	return EmptyResult;
}
FString AGameField::CoordsToLabel(const FVector2D& GridPos)
{
	int32 X = GridPos.X;
	int32 Y = GridPos.Y;

	if (X < 0 || X > 24 || Y < 0 || Y > 24)
		return FString("???");

	TCHAR Letter = 'A' + Y;
	int32 Number = X + 1;

	return FString::Printf(TEXT("%c%d"), Letter, Number);
}