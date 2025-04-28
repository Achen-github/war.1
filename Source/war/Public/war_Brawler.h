// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseUnit.h"
#include "war_Brawler.generated.h"

/**
 * 
 */
UCLASS()
class WAR_API Awar_Brawler : public ABaseUnit
{
	GENERATED_BODY()
public:
	Awar_Brawler();
	int32 GetRandomDamage() const;
	int32 GetRandomCounter() const;
};
