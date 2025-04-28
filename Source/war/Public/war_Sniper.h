// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseUnit.h"
#include "war_Sniper.generated.h"

/**
 * 
 */
UCLASS()
class WAR_API Awar_Sniper : public ABaseUnit
{
	GENERATED_BODY()
	
public:
	Awar_Sniper();
	int32 GetRandomDamage() const;
	int32 GetRandomCounter() const;
};
