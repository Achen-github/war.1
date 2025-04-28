// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "war_Interface.generated.h"

UENUM()
enum class ESign : uint8
{
	A,
	B,
	O,
	E
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class Uwar_Interface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class WAR_API Iwar_Interface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	AActor* Unit;
	int32 PlayerNumber;
	ESign Sign;

	virtual void OnTurn() {};
	virtual void OnWin() {};
	virtual void OnLose() {};

};
