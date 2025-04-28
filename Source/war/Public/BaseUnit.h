// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "war_GameInstance.h"
#include "BaseUnit.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitFinishedMoving, ABaseUnit*, Unit);
class ATile;
UCLASS()
class WAR_API ABaseUnit : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ABaseUnit();

    FORCEINLINE ATile* GetCurrentTile() const;
    // Setter inline
    FORCEINLINE void SetCurrentTile(ATile* NewTile) { CurrentTile = NewTile; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    // game instance reference
    Uwar_GameInstance* GameInstance;
    // Proprietà comuni
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 MaxMoveRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 MinDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 MaxDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 MinCounter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 MaxCounter;

    UPROPERTY(BlueprintReadWrite)
    bool bHasMoved = false;

    UPROPERTY(BlueprintReadWrite)
    bool bHasAttacked = false;

    UPROPERTY()
    TArray<ATile*> MovementPath;

    UPROPERTY()
    ATile* TargetTile;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnUnitFinishedMoving OnUnitFinishedMoving;
    int32 CurrentPathIndex = 0;
    FTimerHandle MovementTimerHandle;

    void SetGridPosition(FVector2D NewPosition);
    FVector2D GetGridPosition() const;

    void AttackUnit(ABaseUnit* TargetUnit);
    void CounterAttackUnit(ABaseUnit* TargetUnit);
    int32 GetMaxMoveRange() const;
    int32 GetAttackRange() const;
    bool bIsMoving = false;
private:
    ATile* CurrentTile;
    bool bIsSelected;
    FVector2D GridPosition;
    FVector TargetPosition;
    
    float MoveSpeed = 60.f; // Velocità di movimento
    //ATile* CurrentTile; // Tile su cui si trova l'unità

};
