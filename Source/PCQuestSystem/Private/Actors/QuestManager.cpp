// Copyright � Pedro Costa, 2021. All rights reserved

#include "Actors/QuestManager.h"
#include "Actors/LocationTrigger.h"
#include <Components/IconMarkerComponent.h>
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"


AQuestManager::AQuestManager()
{
    PrimaryActorTick.bCanEverTick = false;
    SetReplicates(true);
}

AQuestManager::~AQuestManager()
{
    for (TSharedPtr<FQuest> Quest : AllQuests)
    {
        Quest.Reset();
    }
    ActiveQuests.Empty();
    AllQuests.Empty();
}

void AQuestManager::ActivateQuest_Implementation(int QuestIDToActivate)
{
    TSharedPtr<FQuest> QuestToActivate = GetQuestByID(QuestIDToActivate);

    if (QuestToActivate.IsValid())
    {
        if (QuestToActivate->HasQuestStarted())
        {
            ResetQuest(QuestIDToActivate);
        }
        AddActiveQuest(QuestIDToActivate);
    }
}

void AQuestManager::OnAfterQuestActivated(int QuestIDToActivate)
{
    UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnAfterQuestActivated")));
    TSharedPtr<FQuest> QuestToActivate = GetQuestByID(QuestIDToActivate);
    OnQuestActivated.Broadcast(*QuestToActivate.Get());
}

void AQuestManager::ResetQuest_Implementation(int QuestIDToActivate)
{
    UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ResetQuest")));
    TSharedPtr<FQuest> QuestToActivate = GetQuestByID(QuestIDToActivate);
    QuestToActivate->ResetQuest();
}

void AQuestManager::AddActiveQuest_Implementation(int QuestIDToActivate)
{
    UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("AddActiveQuest")));
    ActiveQuests.Add(QuestIDToActivate);

    ActivateQuestObjectives(QuestIDToActivate);
    ActivateQuestReferences(QuestIDToActivate);
    SetCurrentActiveQuest(QuestIDToActivate);    
    OnAfterQuestActivated(QuestIDToActivate);
}

void AQuestManager::SetCurrentActiveQuest(int QuestIDToActivate)
{
    UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("SetCurrentActiveQuest")));
    CurrentActiveQuest = QuestIDToActivate;
}

void AQuestManager::RemoveActiveQuest_Implementation(int QuestIDToRemove)
{
    auto Quest = AllQuests.FindByPredicate([&](const TSharedPtr<FQuest> quest)
    {
        return quest->QuestID == QuestIDToRemove;
    });
    if (Quest)
    {
        ActiveQuests.Remove(Quest->Get()->QuestID);
    }
}

void AQuestManager::OnArrivedToPlace_Implementation(FGameplayTag ArrivedPlace)
{
    for (int QuestID : ActiveQuests)
    {
        auto Quest = GetQuestByID(QuestID);
        for (TSharedPtr<FQuestStepObjective> QuestObjective : Quest.Get()->ObjectivesArray)
        {
            if (!QuestObjective->IsCompleted() && QuestObjective->QuestStepType == EQuestStepType::GoTo)
            {
                TSharedPtr<FQuestStepGoToObjective> GoToObjective = StaticCastSharedPtr<FQuestStepGoToObjective>(QuestObjective);
                if (GoToObjective && GoToObjective->PlaceToGo == ArrivedPlace)
                {
                    OnQuestStepArrivedToPlace(GoToObjective.Get()->StepObjectiveInsideQuestOrder, Quest.Get()->QuestID, ArrivedPlace);

                    if (GoToObjective->IsCompleted())
                    {
                        OnStepQuestCompleted(GoToObjective.Get()->StepObjectiveInsideQuestOrder, Quest.Get()->QuestID);
                    }

                    if (Quest->IsQuestCompleted())
                    {
                        OnQuestCompleted(Quest);
                    }
                }
            }
        }
    }
}

void AQuestManager::OnEntityTalkedTo_Implementation(FGameplayTag TalkedEntity)
{
    for (int QuestID : ActiveQuests)
    {
        auto Quest = GetQuestByID(QuestID);
        for (TSharedPtr<FQuestStepObjective> QuestObjective : Quest.Get()->ObjectivesArray)
        {
            if (!QuestObjective->IsCompleted() && QuestObjective->QuestStepType == EQuestStepType::TalkWith)
            {
                TSharedPtr<FQuestStepTalkWithObjective> TalkToObjective = StaticCastSharedPtr<FQuestStepTalkWithObjective>(QuestObjective);
                if (TalkToObjective && TalkToObjective->EntityToTalkWith == TalkedEntity)
                {
                    OnQuestStepEntityTalkedTo(TalkToObjective.Get()->StepObjectiveInsideQuestOrder, Quest.Get()->QuestID, TalkedEntity);

                    if (TalkToObjective->IsCompleted())
                    {
                        OnStepQuestCompleted(TalkToObjective.Get()->StepObjectiveInsideQuestOrder, Quest.Get()->QuestID);
                    }

                    if (Quest->IsQuestCompleted())
                    {
                        OnQuestCompleted(Quest);
                    }
                }
            }
        }
    }
}

void AQuestManager::OnEntityKilled_Implementation(FGameplayTag EntityKilled)
{
    for (int QuestID : ActiveQuests)
    {
        auto Quest = GetQuestByID(QuestID);
        for (TSharedPtr<FQuestStepObjective> QuestObjective : Quest.Get()->ObjectivesArray)
        {
            if (!QuestObjective->IsCompleted() && QuestObjective->QuestStepType == EQuestStepType::Kill)
            {
                TSharedPtr<FQuestStepKillObjective> KillObjective = StaticCastSharedPtr<FQuestStepKillObjective>(QuestObjective);
                if (KillObjective && KillObjective->EntityToKill == EntityKilled)
                {
                    OnQuestStepEntityKilled(KillObjective.Get()->StepObjectiveInsideQuestOrder, Quest.Get()->QuestID, EntityKilled);

                    if (KillObjective->IsCompleted())
                    {
                        OnStepQuestCompleted(KillObjective.Get()->StepObjectiveInsideQuestOrder, Quest.Get()->QuestID);
                    }

                    if (Quest->IsQuestCompleted())
                    {
                        OnQuestCompleted(Quest);
                    }
                }
            }
        }
    }
}

void AQuestManager::OnItemGathered_Implementation(FGameplayTag ItemGathered, float amountGathered)
{
    for (int QuestID : ActiveQuests)
    {
        auto Quest = GetQuestByID(QuestID);
        for (TSharedPtr<FQuestStepObjective> QuestObjective : Quest.Get()->ObjectivesArray)
        {
            if (!QuestObjective->IsCompleted() && QuestObjective->QuestStepType == EQuestStepType::Gather)
            {
                TSharedPtr<FQuestStepGatherObjective> ItemGatheredObjective = StaticCastSharedPtr<FQuestStepGatherObjective>(QuestObjective);
                if (ItemGatheredObjective && ItemGatheredObjective->ItemToGather == ItemGathered)
                {
                    OnQuestStepItemGathered(ItemGatheredObjective.Get()->StepObjectiveInsideQuestOrder, Quest.Get()->QuestID, ItemGathered, amountGathered);

                    if (ItemGatheredObjective->IsCompleted())
                    {
                        OnStepQuestCompleted(ItemGatheredObjective.Get()->StepObjectiveInsideQuestOrder, Quest.Get()->QuestID);
                    }

                    if (Quest->IsQuestCompleted())
                    {
                        OnQuestCompleted(Quest);
                    }
                }
            }
        }
    }
}

void AQuestManager::OnCatch_Implementation(FGameplayTag CatchTag)
{
    for (int QuestID : ActiveQuests)
    {
        auto Quest = GetQuestByID(QuestID);
        for (TSharedPtr<FQuestStepObjective> QuestObjective : Quest.Get()->ObjectivesArray)
        {
            if (!QuestObjective->IsCompleted() && QuestObjective->QuestStepType == EQuestStepType::Catch)
            {
                TSharedPtr<FQuestStepCatchObjective> CatchObjective = StaticCastSharedPtr<FQuestStepCatchObjective>(QuestObjective);
                if (CatchObjective && CatchObjective->AllowedTagToCatch.Contains(CatchTag))
                {
                    OnQuestStepCatch(CatchObjective.Get()->StepObjectiveInsideQuestOrder, Quest.Get()->QuestID, CatchTag);

                    if (CatchObjective->IsCompleted())
                    {
                        OnStepQuestCompleted(CatchObjective.Get()->StepObjectiveInsideQuestOrder, Quest.Get()->QuestID);
                    }

                    if (Quest->IsQuestCompleted())
                    {
                        OnQuestCompleted(Quest);
                    }
                }
            }
        }
    }
}

const TArray<FQuest> AQuestManager::GetActiveQuests()
{
    TArray<FQuest> ActiveQuestsPointers;
    for (int QuestID : ActiveQuests)
    {
        ActiveQuestsPointers.Add(*GetQuestByID(QuestID));
    }
    return ActiveQuestsPointers;
}

const TArray<FQuest> AQuestManager::GetAllQuests()
{
    TArray<FQuest> AllQuestsPointers;
    for (TSharedPtr<FQuest> quest : AllQuests)
    {
        AllQuestsPointers.Add(*quest.Get());
    }
    return AllQuestsPointers;
}

const FQuestStepObjective AQuestManager::GetCurrentQuestCurrentObjective()
{
    if (CurrentActiveQuest != -1)
    {
        return GetQuestByID(CurrentActiveQuest)->GetCurrentObjective();
    }
    return FQuestStepObjective();
}

FQuestStepObjective AQuestManager::GetCurrentQuestCurrentObjective() const
{
    if (CurrentActiveQuest != -1)
    {
        return GetQuestByID(CurrentActiveQuest)->GetCurrentObjective();
    }
    return FQuestStepObjective();
}


void AQuestManager::AddAssociatedActorToQuestStep(int StepQuestID, int QuestIDToGet, AActor* ActorToAdd)
{
    FTimerHandle TimerHandle;
    FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &AQuestManager::AddAssociatedActorToQuestStepAfterTime, StepQuestID, QuestIDToGet, ActorToAdd);
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 2.0f, false);
}

const FQuest AQuestManager::GetCurrentActiveQuest()
{
    return CurrentActiveQuest != -1 ? *GetQuestByID(CurrentActiveQuest) : FQuest();
}

void AQuestManager::SpawnActor_Implementation(TSubclassOf<AActor> ActorToSpawn, FVector WorldPositionToSpawn, FRotator WorldRotationToSpawn)
{
    FActorSpawnParameters SpawnParameters;
    SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    LastSpawnedActor = GetWorld()->SpawnActor<AActor>(ActorToSpawn, WorldPositionToSpawn, WorldRotationToSpawn, SpawnParameters);
}

void AQuestManager::OnQuestStepArrivedToPlace_Implementation(int StepID, int QuestID, FGameplayTag ArrivedPlace)
{
    TSharedPtr<FQuest> QuestWhereStepBelongs = GetQuestByID(QuestID);
    TSharedPtr<FQuestStepObjective> StepQuest = GetStepQuestByID(StepID, QuestWhereStepBelongs);
    TSharedPtr<FQuestStepGoToObjective> GoToObjective = StaticCastSharedPtr<FQuestStepGoToObjective>(StepQuest);
    GoToObjective.Get()->OnArrivedToPlace();
}

void AQuestManager::OnQuestStepEntityTalkedTo_Implementation(int StepID, int QuestID, FGameplayTag TalkedEntity)
{
    TSharedPtr<FQuest> QuestWhereStepBelongs = GetQuestByID(QuestID);
    TSharedPtr<FQuestStepObjective> StepQuest = GetStepQuestByID(StepID, QuestWhereStepBelongs);
    TSharedPtr<FQuestStepTalkWithObjective> TalkToObjective = StaticCastSharedPtr<FQuestStepTalkWithObjective>(StepQuest);
    TalkToObjective.Get()->OnTalkedWithEntity();
}

void AQuestManager::OnQuestStepEntityKilled_Implementation(int StepID, int QuestID, FGameplayTag EntityKilled)
{
    TSharedPtr<FQuest> QuestWhereStepBelongs = GetQuestByID(QuestID);
    TSharedPtr<FQuestStepObjective> StepQuest = GetStepQuestByID(StepID, QuestWhereStepBelongs);
    TSharedPtr<FQuestStepKillObjective> KillObjective = StaticCastSharedPtr<FQuestStepKillObjective>(StepQuest);
    KillObjective.Get()->OnEntityKilled();
}

void AQuestManager::OnQuestStepItemGathered_Implementation(int StepID, int QuestID, FGameplayTag ItemGathered, float amountGathered)
{
    TSharedPtr<FQuest> QuestWhereStepBelongs = GetQuestByID(QuestID);
    TSharedPtr<FQuestStepObjective> StepQuest = GetStepQuestByID(StepID, QuestWhereStepBelongs);
    TSharedPtr<FQuestStepGatherObjective> GatherObjective = StaticCastSharedPtr<FQuestStepGatherObjective>(StepQuest);
    GatherObjective.Get()->OnItemGathered(amountGathered);
}

void AQuestManager::OnQuestStepCatch_Implementation(int StepID, int QuestID, FGameplayTag CatchTag)
{
    TSharedPtr<FQuest> QuestWhereStepBelongs = GetQuestByID(QuestID);
    TSharedPtr<FQuestStepObjective> StepQuest = GetStepQuestByID(StepID, QuestWhereStepBelongs);
    TSharedPtr<FQuestStepCatchObjective> GatherObjective = StaticCastSharedPtr<FQuestStepCatchObjective>(StepQuest);
    GatherObjective.Get()->OnCatched();
}

void AQuestManager::OnRep_OnCurrentActiveQuest()
{
    AddActiveQuest(CurrentActiveQuest);
}

void AQuestManager::AddAssociatedActorToQuestStepAfterTime_Implementation(int StepQuestID, int QuestIDToGet, AActor* ActorToAdd)
{
    TSharedPtr<FQuest> QuestWhereStepBelongs = GetQuestByID(QuestIDToGet);
    TSharedPtr<FQuestStepObjective> StepQuest = GetStepQuestByID(StepQuestID, QuestWhereStepBelongs);
    StepQuest.Get()->AddAssociatedActor(ActorToAdd);
    StepQuest.Get()->AddIconMarkerToAssociatedActor();
}

bool AQuestManager::HasCurrentActiveQuest() const
{
    return CurrentActiveQuest != -1;
}

bool AQuestManager::IsQuestCompleted(FQuest QuestToCheck)
{
    return QuestToCheck.IsQuestCompleted();
}

bool AQuestManager::IsCurrentActiveQuest(FQuest QuestToCompare)
{
    return CurrentActiveQuest != -1 ? QuestToCompare.QuestID == CurrentActiveQuest : false;
}

bool AQuestManager::IsCurrentQuestStepObjective(FQuestStepObjective QuestStepToCompare)
{
    return GetCurrentQuestCurrentObjective().StepObjectiveID == QuestStepToCompare.StepObjectiveID;
}

FText AQuestManager::GetStepObjectiveDescription(FQuestStepObjective QuestStep)
{
    return QuestStep.GetStepDescription();
}

AActor* AQuestManager::GetLastSpawnedActor()
{
    return LastSpawnedActor;
}

void AQuestManager::BeginPlay()
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
                FQuest(QuestId++, quest->QuestType, quest->Name, quest->QuestRewards, quest->GoToObjectives, quest->TalkWithObjectives, quest->KillObjectives, quest->GatherObjectives, quest->CatchObjectives)
            ));
        }
    }

    for (auto QuestReference : QuestReferences)
    {
        DeactivateQuestReferences(QuestReference.Key);
    }
}

void AQuestManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(AQuestManager, CurrentActiveQuest, COND_InitialOnly, REPNOTIFY_Always);
    DOREPLIFETIME(AQuestManager, ActiveQuests);
}


TSharedPtr<FQuest> AQuestManager::GetQuestByID(const int IDToGet)
{
    TSharedPtr<FQuest> QuestToReturn = *AllQuests.FindByPredicate([&](const TSharedPtr<FQuest> quest)
    {
        return quest->QuestID == IDToGet;
    });

    return QuestToReturn;
}

TSharedPtr<FQuest> AQuestManager::GetQuestByID(const int IDToGet) const
{
    TSharedPtr<FQuest> QuestToReturn = *AllQuests.FindByPredicate([&](const TSharedPtr<FQuest> quest)
    {
        return quest->QuestID == IDToGet;
    });

    return QuestToReturn;
}   

TSharedPtr<FQuestStepObjective> AQuestManager::GetStepQuestByID(int IDToGet, TSharedPtr<FQuest> QuestToSearch)
{
    return QuestToSearch.Get()->GetStepObjectiveById(IDToGet);
}

void AQuestManager::ActivateQuestObjectives(int QuestID)
{
    if (QuestID != -1)
    {
        TSharedPtr<FQuest> Quest = GetQuestByID(QuestID);
        for (TSharedPtr<FQuestStepObjective> QuestObjective : Quest.Get()->ObjectivesArray)
        {
            QuestObjective->Activate(GetWorld(), this);
        }
        Quest.Get()->ObjectivesArray[0].Get()->ActivateActorMarker();
    }
}

void AQuestManager::ActivateQuestReferences(int QuestID)
{
    if (QuestID != -1)
    {
        if (QuestReferences.Contains(QuestID))
        {
            for (auto QuestActor : QuestReferences[QuestID].QuestActors)
            {
                QuestActor->SetActorEnableCollision(true);
                QuestActor->SetActorHiddenInGame(false);
            }
        }
    }
}

void AQuestManager::DeactivateQuestReferences(int QuestID)
{
    if (QuestID != -1)
    {
        if (QuestReferences.Contains(QuestID))
        {
            for (auto QuestActor : QuestReferences[QuestID].QuestActors)
            {
                QuestActor->SetActorEnableCollision(false);
                QuestActor->SetActorHiddenInGame(true);
            }
        }
    }
}

void AQuestManager::OnQuestCompleted(TSharedPtr<FQuest> CompletedQuest)
{
    FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &AQuestManager::OnQuestCompletedNextTick, CompletedQuest.Get()->QuestID);
    GetWorld()->GetTimerManager().SetTimerForNextTick(TimerDelegate);
}

void AQuestManager::OnQuestCompletedNextTick_Implementation(int CompletedQuestID)
{
    RemoveActiveQuest(CompletedQuestID);
    TSharedPtr<FQuest> CompletedQuest = GetQuestByID(CompletedQuestID);
    OnQuestCompletedDelegate.Broadcast(*CompletedQuest.Get());
    DeactivateQuestReferences(CompletedQuestID);

    // TODO: Maybe we want to put the next main quest as the current quest?
}

void AQuestManager::OnStepQuestCompleted_Implementation(int CompletedStepQuestID, int QuestIDWhereStepBelongs)
{
    TSharedPtr<FQuest> QuestWhereStepBelongs = GetQuestByID(QuestIDWhereStepBelongs);
    TSharedPtr<FQuestStepObjective> CompletedStepQuest = GetStepQuestByID(CompletedStepQuestID, QuestWhereStepBelongs);
    FQuestStepObjective NextObjective = GetCurrentQuestCurrentObjective();
    if (NextObjective.IsValid())
    {
        NextObjective.ActivateActorMarker();
    }
    CompletedStepQuest->Deactivate();
    OnQuestStepCompletedDelegate.Broadcast(*CompletedStepQuest.Get(), *QuestWhereStepBelongs.Get());
}

FString FQuestStepObjective::SplitEnumString(FString EnumString)
{
    FString LeftSplit, RightSplit;
    EnumString.Split("::", &LeftSplit, &RightSplit);
    return RightSplit;
}

void FQuestStepObjective::AddIconMarkerToAssociatedActor()
{
    if (ObjectiveMarkerUMGInformation.bCreateMarker)
    {
        UIconMarkerComponent* iconMarkerComponent = NewObject<UIconMarkerComponent>(ActorsAssociated[ActorsAssociated.Num() - 1], "");
        iconMarkerComponent->SetMarkerUMGToUse(ObjectiveMarkerUMGInformation.ObjectiveMarkerUMGClass);
        iconMarkerComponent->bShowOnCompass = ObjectiveMarkerUMGInformation.bShowOnCompass;
        iconMarkerComponent->bShowOnScreen = ObjectiveMarkerUMGInformation.bShowOnScreen;
        iconMarkerComponent->MarkerIcon = ObjectiveMarkerUMGInformation.IconToUse;
        iconMarkerComponent->ActorOffset = ObjectiveMarkerUMGInformation.MarkerToActorOffset;
        iconMarkerComponent->RegisterComponent();
        iconMarkerComponent->ActivateMarker();
    }
}

void FQuestStepGoToObjective::Activate(UWorld* WorldContext, AQuestManager* QuestManager)
{
    TArray<AActor*> GoToLocations;
    UGameplayStatics::GetAllActorsOfClass(WorldContext, ALocationTrigger::StaticClass(), GoToLocations);
    for (AActor* LocationTrigger : GoToLocations)
    {
        if (ALocationTrigger* LocTrigger = Cast<ALocationTrigger>(LocationTrigger))
        {
            if (LocTrigger->GetLocation() == PlaceToGo)
            {
                QuestManager->AddAssociatedActorToQuestStep(StepObjectiveInsideQuestOrder, ParentQuestID, LocationTrigger);
                if (UIconMarkerComponent* LocationIconMarker = LocationTrigger->FindComponentByClass<UIconMarkerComponent>())
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

void FQuestStepTalkWithObjective::Activate(UWorld* WorldContext, AQuestManager* QuestManager)
{
    QuestManager->SpawnActor(PawnToSpawnWhenActive, WorldPositionToSpawn, WorldRotationToSpawn);

    if (QuestManager->GetLastSpawnedActor())
    {
        QuestManager->AddAssociatedActorToQuestStep(StepObjectiveInsideQuestOrder, ParentQuestID, QuestManager->GetLastSpawnedActor());
    }
}

void FQuestStepKillObjective::Activate(UWorld* WorldContext, AQuestManager* QuestManager)
{
    for (TSubclassOf<AActor> SubClassActor : SpawnInformation.PawnsToSpawnWhenActive)
    {
        for (int i = 0; i < SpawnInformation.NumToSpawnOfEachPawn; ++i)
        {
            FVector SpawnLocation = SpawnInformation.SpawnCenter;
            SpawnLocation.X += FMath::RandRange(-SpawnInformation.SpawnRange, SpawnInformation.SpawnRange);
            SpawnLocation.Y += FMath::RandRange(-SpawnInformation.SpawnRange, SpawnInformation.SpawnRange);
            QuestManager->SpawnActor(SubClassActor, SpawnLocation, FRotator::ZeroRotator);

            if (QuestManager->GetLastSpawnedActor())
            {
                QuestManager->AddAssociatedActorToQuestStep(StepObjectiveInsideQuestOrder, ParentQuestID, QuestManager->GetLastSpawnedActor());
            }
        }
    }
}

void FQuest::ActivateCurrentObjective(UWorld* ContextWorld, AQuestManager* QuestManager)
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
            NextObjective->Activate(ContextWorld, QuestManager);
            break;
        case EQuestStepType::TalkWith:
            TSharedPtr<FQuestStepTalkWithObjective> TalkWithObjective = StaticCastSharedPtr<FQuestStepTalkWithObjective>(NextObjective);
            TalkWithObjective->Activate(ContextWorld, QuestManager);
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