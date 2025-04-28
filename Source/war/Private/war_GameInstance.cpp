// Fill out your copyright notice in the Description page of Project Settings.


#include "war_GameInstance.h"
#include "BaseUnit.h"
#include "war_GameMode.h"



FString Uwar_GameInstance::GetTurnMessage()
{
	return CurrentTurnMessage;
}

void Uwar_GameInstance::SetTurnMessage(FString Message)
{
	CurrentTurnMessage = Message;
}
void Uwar_GameInstance::UpdateUnitStatsText()
{
    FString Result = "Player's Units:\n";

    Awar_GameMode* GameMode = Cast<Awar_GameMode>(GetWorld()->GetAuthGameMode());
    if (!GameMode) return;
    GameMode->IsGameOver();
    for (ABaseUnit* Unit : GameMode->PlayerUnits)
    {
        if (!Unit) continue;
        FString Name = *Unit->GetName();
        if (Name == "BP_SniperA_C_0")
            Name = "Sniper";
        if (Name == "BP_BrawlerA_C_0")
            Name = "Brawler";
        if (!Unit) continue;
        Result += Name+FString::Printf(TEXT("\nHP: %d/%d    ATK: %d~%d\n"),
            Unit->CurrentHealth, Unit->MaxHealth,
            Unit->MinDamage, Unit->MaxDamage);
    }

    Result += "\nAI's Units:\n";

    for (ABaseUnit* Unit : GameMode->AIUnits)
    {
        if (!Unit) continue;
        FString Name = *Unit->GetName();
        if (Name == "BP_SniperB_C_0")
            Name = "Sniper";
        if (Name == "BP_BrawlerB_C_0")
            Name = "Brawler";
        if (!Unit) continue;
        Result += Name+FString::Printf(TEXT("\nHP: %d/%d    ATK: %d~%d\n"),
            Unit->CurrentHealth, Unit->MaxHealth,
            Unit->MinDamage, Unit->MaxDamage);
    }

    UnitStatsText = Result;
}
void Uwar_GameInstance::ResetUnitStatsText() {
    UnitStatsText = "Placing units..";
}
FString Uwar_GameInstance::GetStats()
{
    return UnitStatsText;
}
FString Uwar_GameInstance::GetEndMessage()
{
    return EndMessage;
}

void Uwar_GameInstance::SetEndMessage(FString Message)
{
    EndMessage = Message;
}

FString Uwar_GameInstance::GetMessageLog()
{
    return MessageLog;
}

void Uwar_GameInstance::SetMessageLog(FString Message)
{
    MessageLog = Message;
}