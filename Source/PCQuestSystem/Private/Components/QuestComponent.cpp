// Copyright � Pedro Costa, 2021. All rights reserved

#include "Components/QuestComponent.h"
#include "PCQSBlueprintFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

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

UQuestComponent* UQuestComponent::GetQuestComponent(const AActor* Pawn)
{
    if (Pawn == nullptr)
    {
        return nullptr;
    }

    return Pawn->GetComponentByClass<UQuestComponent>();
}

void UQuestComponent::OnArrivedToPlace(FGameplayTag ArrivedPlace)
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

void UQuestComponent::OnLeftPlace(FGameplayTag PlaceLeft)
{
    OnLeftLocation.Broadcast(PlaceLeft);
}

void UQuestComponent::OnEntityTalkedTo(FGameplayTag TalkedEntity)
{
    if (QuestManager)
    {
        QuestManager->OnEntityTalkedTo(TalkedEntity);
    }
}

void UQuestComponent::OnEntityKilled(FGameplayTag EntityKilled)
{
    if (QuestManager)
    {
        QuestManager->OnEntityKilled(EntityKilled);
    }
}

void UQuestComponent::OnItemGathered(FGameplayTag ItemGathered, float amountGathered)
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
