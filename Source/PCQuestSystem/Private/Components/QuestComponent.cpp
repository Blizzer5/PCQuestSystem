// Copyright © Pedro Costa, 2021. All rights reserved

#include "Components/QuestComponent.h"
#include "PCQSBlueprintFunctionLibrary.h"

UQuestComponent::UQuestComponent()
{
    if (GetOwner() && GetOwner()->GetLocalRole() == ROLE_Authority)
    {
        QuestManager = UPCQSBlueprintFunctionLibrary::GetWorldQuestManager(this);
    }
}

UQuestComponent::~UQuestComponent()
{
}

void UQuestComponent::OnArrivedToPlace(EPlaces ArrivedPlace)
{
    OnEnteredLocation.Broadcast(ArrivedPlace);
    UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnArrivedToPlace")));
    if (GetOwner()->HasAuthority())
    {
        if (QuestManager)
        {
            QuestManager->OnArrivedToPlace(ArrivedPlace);
        }
    }
}

void UQuestComponent::OnLeftPlace(EPlaces PlaceLeft)
{
    OnLeftLocation.Broadcast(PlaceLeft);
}

void UQuestComponent::OnEntityTalkedTo(EEntityType TalkedEntity)
{
    if (QuestManager)
    {
        QuestManager->OnEntityTalkedTo(TalkedEntity);
    }
}

void UQuestComponent::OnEntityKilled(EEntityType EntityKilled)
{
    if (QuestManager)
    {
        QuestManager->OnEntityKilled(EntityKilled);
    }
}

void UQuestComponent::OnItemGathered(EQuestItemTypes ItemGathered, float amountGathered)
{
    if (QuestManager)
    {
        QuestManager->OnItemGathered(ItemGathered, amountGathered);
    }
}

void UQuestComponent::ActivateQuestDebug(int QuestID)
{
    UPCQSBlueprintFunctionLibrary::GetWorldQuestManager(this)->ActivateQuest(QuestID);
}
