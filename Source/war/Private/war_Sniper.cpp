// Fill out your copyright notice in the Description page of Project Settings.


#include "war_Sniper.h"
#include "Kismet/KismetMathLibrary.h"
// Sets default values
Awar_Sniper::Awar_Sniper()
{
    MaxHealth = 20;
    CurrentHealth = MaxHealth;
    MaxMoveRange = 3;
    AttackRange = 10;
    MinDamage = 4;
    MaxDamage = 8;
    MinCounter = 1;
    MaxCounter = 3;
}
int32 Awar_Sniper::GetRandomDamage() const
{
    return UKismetMathLibrary::RandomIntegerInRange(MinDamage, MaxDamage);
}
int32 Awar_Sniper::GetRandomCounter() const
{
    return UKismetMathLibrary::RandomIntegerInRange(MinCounter, MaxCounter);
}