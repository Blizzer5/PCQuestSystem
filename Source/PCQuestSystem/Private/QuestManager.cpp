// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestManager.h"
#include "Internationalization/Internationalization.h"
#include "IconMarkerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "LocationTrigger.h"

UQuestManager::UQuestManager()
{
}

UQuestManager::~UQuestManager()
{
    CurrentActiveQuest.Reset();
    for (TSharedPtr<FQuest> quest : ActiveQuests)
    {
        quest.Reset();
    }
    for (TSharedPtr<FQuest> quest : AllQuests)
    {
        quest.Reset();
    }
    ActiveQuests.Empty();
    AllQuests.Empty();
}

void UQuestManager::OnArrivedToPlace(EPlaces ArrivedPlace)
{
    OnEnteredLocation.Broadcast(ArrivedPlace);

    for (TSharedPtr<FQuest> quest : ActiveQuests)
    {
        for (TSharedPtr<FQuestStepObjective> QuestObjective : quest.Get()->ObjectivesArray)
        {
            if (!QuestObjective->IsCompleted() && QuestObjective->QuestStepType == EQuestStepType::GoTo)
            {
                TSharedPtr<FQuestStepGoToObjective> GoToObjective = StaticCastSharedPtr<FQuestStepGoToObjective>(QuestObjective);
                if (GoToObjective && GoToObjective->PlaceToGo == ArrivedPlace)
                {
                    GoToObjective->OnArrivedToPlace();

                    if (GoToObjective->IsCompleted())
                    {
                        OnStepQuestCompleted(GoToObjective, quest);
                    }

                    if (quest->IsQuestCompleted())
                    {
                        OnQuestCompleted(quest);
                    }
                }
            }
        }
    }
}

void UQuestManager::OnLeftPlace(EPlaces PlaceLeft)
{
    OnLeftLocation.Broadcast(PlaceLeft);
}

void UQuestManager::OnEntityTalkedTo(EEntityType TalkedEntity)
{
    for (TSharedPtr<FQuest> quest : ActiveQuests)
    {
        for (TSharedPtr<FQuestStepObjective> QuestObjective : quest.Get()->ObjectivesArray)
        {
            if (!QuestObjective->IsCompleted() && QuestObjective->QuestStepType == EQuestStepType::TalkWith)
            {
                TSharedPtr<FQuestStepTalkWithObjective> GoToObjective = StaticCastSharedPtr<FQuestStepTalkWithObjective>(QuestObjective);
                if (GoToObjective && GoToObjective->EntityToTalkWith == TalkedEntity)
                {
                    GoToObjective->OnTalkedWithEntity();

                    if (GoToObjective->IsCompleted())
                    {
                        OnStepQuestCompleted(GoToObjective, quest);
                    }

                    if (quest->IsQuestCompleted())
                    {
                        OnQuestCompleted(quest);
                    }

                    GoToObjective.Reset();
                }
            }
        }
    }
}

void UQuestManager::OnEntityKilled(EEntityType EntityKilled)
{
    for (TSharedPtr<FQuest> quest : ActiveQuests)
    {
        for (TSharedPtr<FQuestStepObjective> QuestObjective : quest.Get()->ObjectivesArray)
        {
            if (!QuestObjective->IsCompleted() && QuestObjective->QuestStepType == EQuestStepType::Kill)
            {
                TSharedPtr<FQuestStepKillObjective> GoToObjective = StaticCastSharedPtr<FQuestStepKillObjective>(QuestObjective);
                if (GoToObjective && GoToObjective->EntityToKill == EntityKilled)
                {
                    GoToObjective->OnEntityKilled();

                    if (GoToObjective->IsCompleted())
                    {
                        OnStepQuestCompleted(GoToObjective, quest);
                    }

                    if (quest->IsQuestCompleted())
                    {
                        OnQuestCompleted(quest);
                    }
                }
            }
        }
    }
}

void UQuestManager::OnItemGathered(EQuestItemTypes ItemGathered, float amountGathered)
{
    for (TSharedPtr<FQuest> quest : ActiveQuests)
    {
        for (TSharedPtr<FQuestStepObjective> QuestObjective : quest.Get()->ObjectivesArray)
        {
            if (!QuestObjective->IsCompleted() && QuestObjective->QuestStepType == EQuestStepType::Gather)
            {
                TSharedPtr<FQuestStepGatherObjective> GoToObjective = StaticCastSharedPtr<FQuestStepGatherObjective>(QuestObjective);
                if (GoToObjective && GoToObjective->ItemToGather == ItemGathered)
                {
                    GoToObjective->OnItemGathered(amountGathered);

                    if (GoToObjective->IsCompleted())
                    {
                        OnStepQuestCompleted(GoToObjective, quest);
                    }

                    if (quest->IsQuestCompleted())
                    {
                        OnQuestCompleted(quest);
                    }
                }
            }
        }
    }
}

void UQuestManager::ActivateQuest(int QuestIDToActivate)
{
    TSharedPtr<FQuest> QuestToActivate = GetQuestByID(QuestIDToActivate);

    if (QuestToActivate.IsValid())
    {
        if (QuestToActivate->IsQuestCompleted())
        {
            QuestToActivate->ResetQuest();
        }
        ActiveQuests.Add(QuestToActivate);
        ActivateQuestObjectives(QuestToActivate);

        if (!CurrentActiveQuest.IsValid())
        {
            CurrentActiveQuest = QuestToActivate;
        }

        OnQuestActivated.Broadcast(*QuestToActivate.Get());
    }
}

void UQuestManager::RemoveActiveQuest(int QuestIDToRemove)
{
    ActiveQuests.Remove(*AllQuests.FindByPredicate([&](const TSharedPtr<FQuest> quest)
    {
        return quest->QuestID == QuestIDToRemove;
    }));
}

const TArray<FQuest> UQuestManager::GetActiveQuests()
{
    TArray<FQuest> ActiveQuestsPointers;
    for (TSharedPtr<FQuest> quest : ActiveQuests)
    {
        ActiveQuestsPointers.Add(*quest.Get());
    }
    return ActiveQuestsPointers;
}

const TArray<FQuest> UQuestManager::GetAllQuests()
{
    TArray<FQuest> AllQuestsPointers;
    for (TSharedPtr<FQuest> quest : AllQuests)
    {
        AllQuestsPointers.Add(*quest.Get());
    }
    return AllQuestsPointers;
}

const FQuestStepObjective UQuestManager::GetCurrentQuestCurrentObjective()
{
    if (CurrentActiveQuest.IsValid())
    {
        return CurrentActiveQuest->GetCurrentObjective();
    }
    return FQuestStepObjective();
}

FQuestStepObjective UQuestManager::GetCurrentQuestCurrentObjective() const
{
    if (CurrentActiveQuest.IsValid())
    {
        return CurrentActiveQuest->GetCurrentObjective();
    }
    return FQuestStepObjective();
}



const FQuest UQuestManager::GetCurrentActiveQuest()
{
    return CurrentActiveQuest.IsValid() ? *CurrentActiveQuest.Get() : FQuest();
}

bool UQuestManager::IsQuestCompleted(FQuest QuestToCheck)
{
    return QuestToCheck.IsQuestCompleted();
}

bool UQuestManager::IsCurrentActiveQuest(FQuest QuestToCompare)
{
    return CurrentActiveQuest.IsValid() ? QuestToCompare.QuestID == CurrentActiveQuest->QuestID : false;
}

bool UQuestManager::IsCurrentQuestStepObjective(FQuestStepObjective QuestStepToCompare)
{
    return GetCurrentQuestCurrentObjective().StepObjectiveID == QuestStepToCompare.StepObjectiveID;
}

FString UQuestManager::GetStepObjectiveDescription(FQuestStepObjective QuestStep)
{
    return QuestStep.GetStepDescription();
}

void UQuestManager::BeginPlay()
{
    Super::BeginPlay();

    if (DataTable)
    {
        const FString Context;
        TArray<FQuest*> Quests;
        DataTable->GetAllRows(*Context, Quests);
        int QuestId = 1;
        for (FQuest* quest : Quests)
        {
            AllQuests.Add(MakeShareable(new
                FQuest(QuestId++, quest->QuestType, quest->Name, quest->QuestRewards, quest->GoToObjectives, quest->TalkWithObjectives, quest->KillObjectives, quest->GatherObjectives)
            ));
        }
    }
}

TSharedPtr<FQuest> UQuestManager::GetQuestByID(int IDToGet)
{
    TSharedPtr<FQuest> QuestToReturn = *AllQuests.FindByPredicate([&](const TSharedPtr<FQuest> quest)
    {
        return quest->QuestID == IDToGet;
    });

    return QuestToReturn;
}

void UQuestManager::ActivateQuestObjectives(TSharedPtr<FQuest> Quest)
{
    for (TSharedPtr<FQuestStepObjective> QuestObjective : Quest.Get()->ObjectivesArray)
    {
        QuestObjective->Activate(GetWorld());
    }
    Quest.Get()->ObjectivesArray[0].Get()->ActivateActorMarker();
}

void UQuestManager::OnQuestCompleted(TSharedPtr<FQuest> CompletedQuest)
{
    FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &UQuestManager::OnQuestCompletedNextTick, CompletedQuest);
    GetWorld()->GetTimerManager().SetTimerForNextTick(TimerDelegate);
}

void UQuestManager::OnQuestCompletedNextTick(TSharedPtr<FQuest> CompletedQuest)
{
    RemoveActiveQuest(CompletedQuest->QuestID);
    OnQuestCompletedDelegate.Broadcast(*CompletedQuest.Get());

    // TODO: Maybe we want to put the next main quest as the current quest?
}

void UQuestManager::OnStepQuestCompleted(TSharedPtr<FQuestStepObjective> CompletedStepQuest, TSharedPtr<FQuest> QuestWhereStepBelongs)
{
    CompletedStepQuest->Deactivate();
    OnQuestStepCompletedDelegate.Broadcast(*CompletedStepQuest.Get(), *QuestWhereStepBelongs.Get());
    FQuestStepObjective NextObjective = GetCurrentQuestCurrentObjective();
    if (NextObjective.IsValid())
    {
        NextObjective.ActivateActorMarker();
    }
}

FString FQuestStepObjective::SplitEnumString(FString EnumString)
{
    FString LeftSplit, RightSplit;
    EnumString.Split("::", &LeftSplit, &RightSplit);
    return RightSplit;
}

void FQuestStepObjective::AddIconMarkerToAssociatedActor()
{
    UIconMarkerComponent* iconMarkerComponent = NewObject<UIconMarkerComponent>(ActorAssociated, "");
    iconMarkerComponent->SetMarkerUMGToUse(ObjectiveMarkerUMGClass);
    iconMarkerComponent->RegisterComponent();
}

void FQuestStepGoToObjective::SetDescription()
{
    Description = FText::Format(NSLOCTEXT("QuestNamespace", "QuestStepGoTo", "Go to {0}"), FText::FromString(SplitEnumString(UEnum::GetValueAsString(PlaceToGo)))).ToString();
}

void FQuestStepGoToObjective::Activate(UWorld* WorldContext)
{
    TArray<AActor*> GoToLocations;
    UGameplayStatics::GetAllActorsOfClass(WorldContext, ALocationTrigger::StaticClass(), GoToLocations);
    for (AActor* LocationTrigger : GoToLocations)
    {
        if (ALocationTrigger* LocTrigger = Cast<ALocationTrigger>(LocationTrigger))
        {
            if (LocTrigger->GetLocation() == PlaceToGo)
            {
                ActorAssociated = LocTrigger;
                if(UIconMarkerComponent* LocationIconMarker =  ActorAssociated->FindComponentByClass<UIconMarkerComponent>())
                {
                    LocationIconMarker->ActivateMarker();
                }
                else
                {
                    AddIconMarkerToAssociatedActor();
                }
            }
        }
    }
}

void FQuestStepTalkWithObjective::SetDescription()
{
    Description = FText::Format(NSLOCTEXT("QuestNamespace", "QuestStepTalkWith", "Talk with {0}"), FText::FromString(SplitEnumString(UEnum::GetValueAsString(EntityToTalkWith)))).ToString();
}

void FQuestStepTalkWithObjective::Activate(UWorld* WorldContext)
{
    FActorSpawnParameters SpawnParameters;
    ActorAssociated = WorldContext->SpawnActor<AActor>(PawnToSpawnWhenActive, WorldPositionToSpawn, WorldRotationToSpawn, SpawnParameters);
    AddIconMarkerToAssociatedActor();
}

void FQuestStepKillObjective::SetDescription()
{
    if (AmountToKill <= 1)
    {
        Description = FText::Format(NSLOCTEXT("QuestNamespace", "QuestStepKill", "Kill {0}"), FText::FromString(SplitEnumString(UEnum::GetValueAsString(EntityToKill)))).ToString();
    }
    else
    {
        Description = FText::Format(NSLOCTEXT("QuestNamespace", "QuestStepKill", "Kill {0} {1}/{2}"), FText::FromString(SplitEnumString(UEnum::GetValueAsString(EntityToKill))), FText::AsNumber(CurrentlyKilled), FText::AsNumber(AmountToKill)).ToString();
    }
}

void FQuestStepGatherObjective::SetDescription()
{
    if (AmountToGather <= 1)
    {
        Description = FText::Format(NSLOCTEXT("QuestNamespace", "QuestStepGather", "Collect {0}"), FText::FromString(SplitEnumString(UEnum::GetValueAsString(ItemToGather)))).ToString();
    }
    else
    {
        Description = FText::Format(NSLOCTEXT("QuestNamespace", "QuestStepGather", "Collect {0} {1}/{2}"), FText::FromString(SplitEnumString(UEnum::GetValueAsString(ItemToGather))), FText::AsNumber(CurrentlyGathered), FText::AsNumber(AmountToGather)).ToString();
    }
}

void FQuest::ActivateCurrentObjective(UWorld* ContextWorld)
{
    TSharedPtr<FQuestStepObjective> NextObjective = GetCurrentObjectiveSharedPtr();
    if (NextObjective.IsValid())
    {
        switch (NextObjective->QuestStepType)
        {
        case EQuestStepType::None:
        case EQuestStepType::GoTo:
        case EQuestStepType::Kill:
        case EQuestStepType::Gather:
            NextObjective->Activate(ContextWorld);
            break;
        case EQuestStepType::TalkWith:
            TSharedPtr<FQuestStepTalkWithObjective> TalkWithObjective = StaticCastSharedPtr<FQuestStepTalkWithObjective>(NextObjective);
            TalkWithObjective->Activate(ContextWorld);
            break;
        }
    }
}

void FQuest::DeactivateObjective(TSharedPtr<FQuestStepObjective> ObjectiveToDeactivate)
{
    if (ObjectiveToDeactivate.IsValid())
    {
        ObjectiveToDeactivate->Deactivate();
    }
}
