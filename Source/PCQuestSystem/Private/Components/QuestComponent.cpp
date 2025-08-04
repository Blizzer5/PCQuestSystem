// Copyright � Pedro Costa, 2021. All rights reserved

#include "Components/QuestComponent.h"
#include "PCQSBlueprintFunctionLibrary.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"

UQuestComponent::UQuestComponent()
{
}

UQuestComponent::~UQuestComponent()
{
}

void UQuestComponent::BeginPlay()
{
    Super::BeginPlay();

    if (GetOwner() && GetOwner()->GetLocalRole() == ROLE_Authority)
    {
        QuestManager = UPCQSBlueprintFunctionLibrary::GetWorldQuestManager(this);
    }
    GetController();
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
        if (QuestManager && GetController())
        {
            QuestManager->OnArrivedToPlace(ArrivedPlace, OwnerPlayerController);
        }
    }
}

void UQuestComponent::OnLeftPlace(FGameplayTag PlaceLeft)
{
    OnLeftLocation.Broadcast(PlaceLeft);
}

void UQuestComponent::OnEntityTalkedTo(FGameplayTag TalkedEntity)
{
    if (QuestManager && GetController())
    {
        QuestManager->OnEntityTalkedTo(TalkedEntity, OwnerPlayerController);
    }
}

void UQuestComponent::OnEntityKilled(FGameplayTag EntityKilled)
{
    if (QuestManager && GetController())
    {
        QuestManager->OnEntityKilled(EntityKilled, OwnerPlayerController);
    }
}

void UQuestComponent::OnItemGathered(FGameplayTag ItemGathered, float amountGathered)
{
    if (QuestManager && GetController())
    {
        QuestManager->OnItemGathered(ItemGathered, amountGathered, OwnerPlayerController);
    }
}

void UQuestComponent::OnCatch(FGameplayTag CatchTag)
{
    if (QuestManager && GetController())
    {
        QuestManager->OnCatch(CatchTag, OwnerPlayerController);
    }
}

APlayerController* UQuestComponent::GetController()
{
    if (!OwnerPlayerController)
    {
        if (const ACharacter* Character = Cast<ACharacter>(GetOwner()))
        {
            if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
            {
                OwnerPlayerController = PlayerController;
            }
        }
    }
    return OwnerPlayerController;
}

void UQuestComponent::ActivateQuestDebug(int QuestID)
{
    UPCQSBlueprintFunctionLibrary::GetWorldQuestManager(this)->ActivateQuest(QuestID);
}
