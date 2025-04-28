// Fill out your copyright notice in the Description page of Project Settings.


#include "war_HumanPlayer.h"
#include "GameField.h"
#include "war_GameMode.h"
#include "war_Brawler.h"
#include "war_Sniper.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
// Sets default values
Awar_HumanPlayer::Awar_HumanPlayer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//// Set this pawn to be controlled by the lowest-numbered player
	//AutoPossessPlayer = EAutoReceiveInput::Player0;
	// create a camera component
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	//set the camera as RootComponent
	SetRootComponent(Camera);
	// get the game instance reference
	GameInstance = Cast<Uwar_GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	// default init values
	PlayerNumber = -1;
	Sign = ESign::A;
}

// Called when the game starts or when spawned
void Awar_HumanPlayer::BeginPlay()
{
	Super::BeginPlay();
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		PC->bShowMouseCursor = true;
		PC->SetInputMode(FInputModeGameAndUI());  // Permette UI + mondo 3D
		UE_LOG(LogTemp, Warning, TEXT("InputMode impostato su Game and UI"));
	}

}

// Called every frame
void Awar_HumanPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void Awar_HumanPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (PlayerInputComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerInputComponent trovato, assegnando il Click"));

		PlayerInputComponent->BindAction("Click", IE_Pressed, this, &Awar_HumanPlayer::OnClick);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerInputComponent è NULL!"));
	}
}
void Awar_HumanPlayer::OnTurn()
{
	IsMyTurn = true;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Your Turn"));
	GameInstance->SetTurnMessage(TEXT("Human Turn"));
}

void Awar_HumanPlayer::OnWin()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("You Win!"));
	GameInstance->SetEndMessage(TEXT("You Won!"));
}

void Awar_HumanPlayer::OnLose()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("You Lose!"));
	GameInstance->SetEndMessage(TEXT("You Lost!"));
}

void Awar_HumanPlayer::OnClick()
{
	FHitResult Hit;
	APlayerController* PC = GetWorld()->GetFirstPlayerController();

	if (PC)
	{
		bool bHit = PC->GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);
		if (bHit)
		{
			AActor* HitActor = Hit.GetActor();
			UE_LOG(LogTemp, Warning, TEXT("Click rilevato su: %s"), *HitActor->GetName());

			// Ottieni il GameMode
			Awar_GameMode* GameMode = Cast<Awar_GameMode>(GetWorld()->GetAuthGameMode());
			if (!GameMode) return;

			if (GameMode->bSetupPhase)
			{
				HandlePlacement(HitActor);
				return;
			}

			// Controlla se abbiamo già un'unità selezionata
			ABaseUnit* SelectedUnit = Cast<ABaseUnit>(GameMode->GetSelectedUnit());
			if (SelectedUnit)
			{
				// Se clicchiamo sulla stessa unità selezionata, deselezioniamola
				if (HitActor == SelectedUnit)
				{
					HandleDeselection();
					return;
				}

				// Se clicchiamo su una tile, controlliamo movimento/attacco
				if (ATile* ClickedTile = Cast<ATile>(HitActor))
				{
					HandleMovement(ClickedTile);
					return;
				}

				// Se clicchiamo su un'unità nemica dentro il range di attacco, attacchiamo
				if (ABaseUnit* ClickedUnit = Cast<ABaseUnit>(HitActor))
				{
					HandleAttack(ClickedUnit);
					return;
				}

				// Se clicchiamo su un'area vuota, deselezioniamo l'unità
				HandleDeselection();
			}
			else // Nessuna unità selezionata, quindi proviamo a selezionare un'unità del giocatore
			{
				if (ABaseUnit* ClickedUnit = Cast<ABaseUnit>(HitActor))
				{
					if (GameMode->IsUnitOwnedByPlayer(ClickedUnit))
					{
						HandleSelection(HitActor);
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("Unità nemica non selezionabile!"));
					}
					return;
				}
			}

			UE_LOG(LogTemp, Warning, TEXT("Clic su oggetto non valido."));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Nessun oggetto cliccato!"));
		}
	}
}

void Awar_HumanPlayer::HandlePlacement(AActor* HitActor) {

	if (ATile* CurrTile = Cast<ATile>(HitActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cliccato su una Tile valida"));
		UE_LOG(LogTemp, Warning, TEXT("Tile Status Attuale: %d"), CurrTile->GetTileStatus());
		if (CurrTile->GetTileStatus() == ETileStatus::EMPTY)
		{

			FVector SpawnPosition = CurrTile->GetActorLocation();
			Awar_GameMode* GameMode = Cast<Awar_GameMode>(GetWorld()->GetAuthGameMode());
			if (GameMode)
			{
				GameMode->PlaceUnit(SpawnPosition);
				IsMyTurn = false;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("GameMode è NULL!"));
			}
			if (GameMode->SelectedUnitClass == nullptr)
			{
				UE_LOG(LogTemp, Warning, TEXT("Nessuna unità selezionata!"));
				return; // Esce dalla funzione senza cambiare lo stato della tile
			}
			CurrTile->SetTileStatus(PlayerNumber, ETileStatus::OCCUPIED);
			UE_LOG(LogTemp, Warning, TEXT("Player Number Attuale: %d"), CurrTile->GetOwner());
		}
	}
}
void Awar_HumanPlayer::HandleSelection(AActor* HitActor) {
	Awar_GameMode* GameMode = Cast<Awar_GameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode) return;

	ABaseUnit* ClickedUnit = Cast<ABaseUnit>(HitActor);
	
	if (GameMode->GetSelectedUnit() == ClickedUnit) {
		UE_LOG(LogTemp, Warning, TEXT("Unità deselezionata: %s"), *ClickedUnit->GetName());
		ClearHighlightsMove();
		ClearHighlightsAttack();
		GameMode->SetSelectedUnit(nullptr);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Unità selezionata: %s"), *ClickedUnit->GetName());
		GameMode->SetSelectedUnit(ClickedUnit);
		if (!ClickedUnit->bHasAttacked) {
			ShowAttackRange();
		}
		if (!ClickedUnit->bHasMoved) {
			ShowMovementRange(); // Mostra le tile di movimento
		}
	}
}

void Awar_HumanPlayer::HandleDeselection()
{
	Awar_GameMode* GameMode = Cast<Awar_GameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode) return;

	if (GameMode->GetSelectedUnit())
	{
		UE_LOG(LogTemp, Warning, TEXT("Unità deselezionata: %s"), *GameMode->GetSelectedUnit()->GetName());
		ClearHighlightsMove();
		ClearHighlightsAttack();
		GameMode->SetSelectedUnit(nullptr);
	}
}
void Awar_HumanPlayer::HandleMovement(ATile* CurrTile)
{
	if (!CurrTile) return;
	Awar_GameMode* GameMode = Cast<Awar_GameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode) return;

	ABaseUnit* SelectedUnit = Cast<ABaseUnit>(GameMode->GetSelectedUnit());
	if (!SelectedUnit) return;
	// Se l'unità ha già mosso, non può muoversi di nuovo
	if (SelectedUnit->bHasMoved)
	{
		ClearHighlightsMove();
		UE_LOG(LogTemp, Warning, TEXT("L'unità ha già mosso questo turno!"));
		return;
	}

	// Se la tile è nel range di movimento, muove l'unità
	if (HighlightedMoveTiles.Contains(CurrTile))
	{
		GameMode->MoveUnitToTile(SelectedUnit, CurrTile);
		OnUnitMoveFinished(SelectedUnit);
		SelectedUnit->bHasMoved = true;
		if (GameMode->AreAllPlayerUnitsDone())
		{
			GameMode->TurnNextPlayer();
		}
	}
}

void Awar_HumanPlayer::HandleAttack(ABaseUnit* ClickedUnit)
{
	if (!ClickedUnit) return;
	Awar_GameMode* GameMode = Cast<Awar_GameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode) return;

	ABaseUnit* SelectedUnit = Cast<ABaseUnit>(GameMode->GetSelectedUnit());
	if (!SelectedUnit) return;

	if (SelectedUnit->bHasAttacked)
	{
		UE_LOG(LogTemp, Warning, TEXT("L'unità ha già attaccato questo turno!"));
		return;
	}
	if (!GameMode->IsUnitOwnedByPlayer(ClickedUnit)) // Se è un nemico
	{
		ATile* EnemyTile = GameMode->GField->GetTileAt(ClickedUnit->GetActorLocation());
		if (HighlightedAttackTiles.Contains(EnemyTile)) // Se è in range
		{
			UE_LOG(LogTemp, Warning, TEXT("Attaccando unità nemica: %s"), *ClickedUnit->GetName());
			SelectedUnit->AttackUnit(ClickedUnit);
			ClearHighlightsAttack();
			ClearHighlightsMove();
			SelectedUnit->bHasAttacked = true;
			SelectedUnit->bHasMoved = true;
			GameMode->SetSelectedUnit(nullptr);
			if (GameMode->AreAllPlayerUnitsDone())
			{
				GameMode->TurnNextPlayer();
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Unità nemica fuori dal range di attacco!"));
		}
	}
}


void Awar_HumanPlayer::OnUnitMoveFinished(ABaseUnit* SelectedUnit)
{
	ClearHighlightsMove();
	ClearHighlightsAttack();
	Awar_GameMode* GameMode = Cast<Awar_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode) {
		GameMode->SetSelectedUnit(nullptr);
	}
}
void Awar_HumanPlayer::ShowMovementRange()
{
	ClearHighlightsMove(); // Pulisce evidenziazioni precedenti

	Awar_GameMode* GameMode = Cast<Awar_GameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode || !GameMode->GetSelectedUnit()) return;

	ATile* UnitTile = GameMode->GField->GetTileAt(GameMode->GetSelectedUnit()->GetActorLocation());
	if (!UnitTile) return;

	int32 MovementRange = 0;

	// Controlla se è un Brawler o uno Sniper e prendi il valore corretto
	if (Awar_Brawler* Brawler = Cast<Awar_Brawler>(GameMode->GetSelectedUnit()))
	{
		MovementRange = Brawler->MaxMoveRange;
		UE_LOG(LogTemp, Warning, TEXT("Unità Brawler selezionata con MovementRange: %d"), MovementRange);
	}
	else if (Awar_Sniper* Sniper = Cast<Awar_Sniper>(GameMode->GetSelectedUnit()))
	{
		MovementRange = Sniper->MaxMoveRange;
		UE_LOG(LogTemp, Warning, TEXT("Unità Sniper selezionata con MovementRange: %d"), MovementRange);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Errore: L'unità selezionata non è né Brawler né Sniper!"));
		return;
	}

	// Recupera le tile e evidenziale
	TArray<ATile*> TilesInRange = GameMode->GField->GetTilesInRangeBFS(UnitTile, MovementRange);
	UE_LOG(LogTemp, Warning, TEXT("Tile trovate nel range: %d"), TilesInRange.Num());

	for (ATile* Tile : TilesInRange)
	{
		if (Tile->GetTileStatus() == ETileStatus::EMPTY)
		{
			UE_LOG(LogTemp, Warning, TEXT("Tile evidenziata a: X=%d, Y=%d"),
				(int32)Tile->GetGridPosition().X, (int32)Tile->GetGridPosition().Y);
			Tile->HighlightTile(true);
			HighlightedMoveTiles.Add(Tile);
		}
	}
}
void Awar_HumanPlayer::ClearHighlightsMove()
{
	for (ATile* Tile : HighlightedMoveTiles)
	{
		Tile->HighlightTile(false); // Disattiva l'highlight
	}
	HighlightedMoveTiles.Empty();
}
void Awar_HumanPlayer::ClearHighlightsAttack()
{
	for (ATile* Tile : HighlightedAttackTiles)
	{
		Tile->HighlightTile(false); // Disattiva l'highlight
	}
	HighlightedAttackTiles.Empty();
}

void Awar_HumanPlayer::ShowAttackRange()
{
	ClearHighlightsAttack();
	Awar_GameMode* GameMode = Cast<Awar_GameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode || !GameMode->GetSelectedUnit()) return;

	AGameField* GameField = GameMode->GField;
	if (!GameField) return;

	ATile* UnitTile = GameField->GetTileAt(GameMode->GetSelectedUnit()->GetActorLocation());
	if (!UnitTile) return;

	int32 AttackRange = 0;

	// Determina il range di attacco in base all'unità selezionata
	if (Awar_Brawler* Brawler = Cast<Awar_Brawler>(GameMode->GetSelectedUnit()))
	{
		AttackRange = Brawler->AttackRange;
	}
	else if (Awar_Sniper* Sniper = Cast<Awar_Sniper>(GameMode->GetSelectedUnit()))
	{
		AttackRange = Sniper->AttackRange;
	}

	// Recupera la posizione dell'unità sulla griglia
	FVector2D UnitPos = GameField->GetXYPositionByRelativeLocation(UnitTile->GetActorLocation());

	// Iteriamo su tutte le tile nel raggio massimo
	for (int32 dx = -AttackRange; dx <= AttackRange; dx++)
	{
		for (int32 dy = -AttackRange; dy <= AttackRange; dy++)
		{
			// Distanza di Manhattan: deve essere <= AttackRange
			if (FMath::Abs(dx) + FMath::Abs(dy) > AttackRange) continue;

			FVector2D TilePos = FVector2D(UnitPos.X + dx, UnitPos.Y + dy);
			ATile* Tile = GameField->GetTileAt(GameField->GetRelativeLocationByXYPosition(TilePos.X, TilePos.Y));

			if (Tile)
			{
				ABaseUnit* TargetUnit = Tile->GetOccupyingUnit();
				// Escludiamo la tile dell'unità stessa
				if (Tile == UnitTile) continue;

				// Escludiamo le tile con unità alleate
				if (TargetUnit && GameMode->IsUnitOwnedByPlayer(TargetUnit)) continue;
				Tile->HighlightAttackTile(true);
				HighlightedAttackTiles.Add(Tile);

				
				if (TargetUnit && !GameMode->IsUnitOwnedByPlayer(TargetUnit))
				{
					UE_LOG(LogTemp, Warning, TEXT("Tile attaccabile a: X=%d, Y=%d"),
						(int32)Tile->GetGridPosition().X, (int32)Tile->GetGridPosition().Y);
				}
			}
		}
	}
}
