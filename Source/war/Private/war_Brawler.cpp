// Fill out your copyright notice in the Description page of Project Settings.


#include "war_Brawler.h"
#include "Kismet/KismetMathLibrary.h"
// Sets default values
Awar_Brawler::Awar_Brawler()
{
    MaxHealth = 40;
    CurrentHealth = MaxHealth;
    MaxMoveRange = 6;
    AttackRange = 1;
    MinDamage = 1;
    MaxDamage = 6;
    MinCounter = 1;
    MaxCounter = 3;
}
int32 Awar_Brawler::GetRandomDamage() const
{
    return UKismetMathLibrary::RandomIntegerInRange(MinDamage, MaxDamage);
}
int32 Awar_Brawler::GetRandomCounter() const
{
    return UKismetMathLibrary::RandomIntegerInRange(MinCounter, MaxCounter);
}