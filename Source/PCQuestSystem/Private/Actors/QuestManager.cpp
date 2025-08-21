// Copyright � Pedro Costa, 2021. All rights reserved

#include "Actors/QuestManager.h"
#include "Actors/LocationTrigger.h"
#include <Components/IconMarkerComponent.h>
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"
#include "Interface/QuestObject.h"
#include "Iris/ReplicationState/ReplicationStateUtil.h"


AQuestManager::AQuestManager()
{
    PrimaryActorTick.bCanEverTick = false;
    SetReplicates(true);
    bAlwaysRelevant = true;
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

void AQuestManager::ActivateQuest_Implementation(int QuestIDToActivate, int StepIDToActivate)
{
    TSharedPtr<FQuest> QuestToActivate = GetQuestByID(QuestIDToActivate);

    if (QuestToActivate.IsValid())
    {
        if (QuestToActivate->HasQuestStarted())
        {
            ResetQuest(QuestIDToActivate);
        }
        AddActiveQuest(QuestIDToActivate, !GetCurrentActiveQuestInfo().IsValid(), StepIDToActivate);
    }
}

void AQuestManager::OnAfterQuestActivated(int QuestIDToActivate, bool bNewQuest)
{
    UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnAfterQuestActivated")));
    TSharedPtr<FQuest> QuestToActivate = GetQuestByID(QuestIDToActivate);
    OnQuestActivated.Broadcast(*QuestToActivate.Get(), bNewQuest);
}

void AQuestManager::ResetQuest_Implementation(int QuestIDToActivate)
{
    UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ResetQuest")));
    TSharedPtr<FQuest> QuestToActivate = GetQuestByID(QuestIDToActivate);
    QuestToActivate->ResetQuest();
}

void AQuestManager::AddActiveQuest_Implementation(int QuestIDToActivate, bool NewCurrentActiveQuest, int StepIDToActivate, bool bNewQuest)
{
    UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("AddActiveQuest")));
    if (!ActiveQuests.FindByPredicate([QuestIDToActivate](const FQuestStateInfo& QuestInfo){ return QuestInfo.QuestID == QuestIDToActivate;  }))
    {
        ActiveQuests.Add({ QuestIDToActivate, StepIDToActivate});
    }

    ActivateQuestReferences(QuestIDToActivate);
    ActivateQuestObjectives(QuestIDToActivate, StepIDToActivate);
    if (NewCurrentActiveQuest)
    {
        SetCurrentActiveQuest(QuestIDToActivate, StepIDToActivate);
    }
    OnAfterQuestActivated(QuestIDToActivate, bNewQuest);
}

void AQuestManager::SetCurrentActiveQuest(int QuestIDToActivate, int StepIDToActivate)
{
    UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("SetCurrentActiveQuest")));
    for (FQuestStateInfo& ActiveQuest : ActiveQuests)
    {
        ActiveQuest.CurrentActive = false;
        if (ActiveQuest.IsValid() && ActiveQuest.QuestID == QuestIDToActivate)
        {
            ActiveQuest.CurrentStepQuestObjectID = StepIDToActivate;
            ActiveQuest.CurrentActive = true;
        }
    }
}

void AQuestManager::RemoveActiveQuest_Implementation(int QuestIDToRemove)
{
    for (int i = ActiveQuests.Num() - 1; i >= 0; i--)
    {
        if (ActiveQuests[i].QuestID == QuestIDToRemove)
        {
            ActiveQuests.RemoveAt(i);
            break;
        }
    }
}

void AQuestManager::OnArrivedToPlace_Implementation(FGameplayTag ArrivedPlace, APlayerController* ArrivedBy)
{
    for (FQuestStateInfo QuestInfo : ActiveQuests)
    {
        auto Quest = GetQuestByID(QuestInfo.QuestID);
        for (TSharedPtr<FQuestStepObjective> QuestObjective : Quest.Get()->ObjectivesArray)
        {
            if (!QuestObjective->IsCompleted() && QuestObjective->QuestStepType == EQuestStepType::GoTo)
            {
                TSharedPtr<FQuestStepGoToObjective> GoToObjective = StaticCastSharedPtr<FQuestStepGoToObjective>(QuestObjective);
                if (GoToObjective && GoToObjective->PlaceToGo == ArrivedPlace)
                {
                    OnQuestStepArrivedToPlace(GoToObjective.Get()->StepObjectiveInsideQuestOrder, Quest.Get()->QuestID, ArrivedPlace, ArrivedBy);

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

void AQuestManager::OnEntityTalkedTo_Implementation(FGameplayTag TalkedEntity, APlayerController* TalkedBy)
{
    for (FQuestStateInfo QuestInfo : ActiveQuests)
    {
        auto Quest = GetQuestByID(QuestInfo.QuestID);
        for (TSharedPtr<FQuestStepObjective> QuestObjective : Quest.Get()->ObjectivesArray)
        {
            if (!QuestObjective->IsCompleted() && QuestObjective->QuestStepType == EQuestStepType::TalkWith)
            {
                TSharedPtr<FQuestStepTalkWithObjective> TalkToObjective = StaticCastSharedPtr<FQuestStepTalkWithObjective>(QuestObjective);
                if (TalkToObjective && TalkToObjective->EntityToTalkWith == TalkedEntity)
                {
                    OnQuestStepEntityTalkedTo(TalkToObjective.Get()->StepObjectiveInsideQuestOrder, Quest.Get()->QuestID, TalkedEntity, TalkedBy);

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

void AQuestManager::OnEntityKilled_Implementation(FGameplayTag EntityKilled, APlayerController* KilledBy)
{
    for (FQuestStateInfo QuestInfo : ActiveQuests)
    {
        auto Quest = GetQuestByID(QuestInfo.QuestID);
        for (TSharedPtr<FQuestStepObjective> QuestObjective : Quest.Get()->ObjectivesArray)
        {
            if (!QuestObjective->IsCompleted() && QuestObjective->QuestStepType == EQuestStepType::Kill)
            {
                TSharedPtr<FQuestStepKillObjective> KillObjective = StaticCastSharedPtr<FQuestStepKillObjective>(QuestObjective);
                if (KillObjective && KillObjective->EntityToKill == EntityKilled)
                {
                    OnQuestStepEntityKilled(KillObjective.Get()->StepObjectiveInsideQuestOrder, Quest.Get()->QuestID, EntityKilled, KilledBy);

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

void AQuestManager::OnItemGathered_Implementation(FGameplayTag ItemGathered, float amountGathered, APlayerController* GatheredBy)
{
    for (FQuestStateInfo QuestInfo : ActiveQuests)
    {
        auto Quest = GetQuestByID(QuestInfo.QuestID);
        for (TSharedPtr<FQuestStepObjective> QuestObjective : Quest.Get()->ObjectivesArray)
        {
            if (!QuestObjective->IsCompleted() && QuestObjective->QuestStepType == EQuestStepType::Gather)
            {
                TSharedPtr<FQuestStepGatherObjective> ItemGatheredObjective = StaticCastSharedPtr<FQuestStepGatherObjective>(QuestObjective);
                if (ItemGatheredObjective && ItemGatheredObjective->ItemToGather == ItemGathered)
                {
                    OnQuestStepItemGathered(ItemGatheredObjective.Get()->StepObjectiveInsideQuestOrder, Quest.Get()->QuestID, ItemGathered, amountGathered, GatheredBy);

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

void AQuestManager::OnCatch_Implementation(FGameplayTag CatchTag, APlayerController* CatchedBy)
{
    for (FQuestStateInfo QuestInfo : ActiveQuests)
    {
        auto Quest = GetQuestByID(QuestInfo.QuestID);
        for (TSharedPtr<FQuestStepObjective> QuestObjective : Quest.Get()->ObjectivesArray)
        {
            if (!QuestObjective->IsCompleted() && QuestObjective->QuestStepType == EQuestStepType::Catch)
            {
                TSharedPtr<FQuestStepCatchObjective> CatchObjective = StaticCastSharedPtr<FQuestStepCatchObjective>(QuestObjective);
                if (CatchObjective && CatchObjective->AllowedTagToCatch.Contains(CatchTag))
                {
                    OnQuestStepCatch(CatchObjective.Get()->StepObjectiveInsideQuestOrder, Quest.Get()->QuestID, CatchTag, CatchedBy);

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
    for (FQuestStateInfo QuestInfo : ActiveQuests)
    {
        ActiveQuestsPointers.Add(*GetQuestByID(QuestInfo.QuestID));
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

TArray<FQuest> AQuestManager::GetAllCompletedQuests()
{
    TArray<FQuest> AllCompletedQuests;
    for (int QuestID : CompletedQuests)
    {
        AllCompletedQuests.Add(*GetQuestByID(QuestID).Get());
    }
    return AllCompletedQuests;
}

const FQuestStepObjective AQuestManager::GetCurrentQuestCurrentObjective()
{
    if (FQuestStateInfo QuestState = GetCurrentActiveQuestInfo(); QuestState.IsValid())
    {
        return GetQuestByID(QuestState.QuestID)->GetCurrentObjective();
    }
    return FQuestStepObjective();
}

FQuestStepObjective AQuestManager::GetCurrentQuestCurrentObjective() const
{
    if (FQuestStateInfo QuestState = GetCurrentActiveQuestInfo(); QuestState.IsValid())
    {
        return GetQuestByID(QuestState.QuestID)->GetCurrentObjective();
    }
    return FQuestStepObjective();
}


void AQuestManager::AddAssociatedActorToQuestStep(int StepQuestID, int QuestIDToGet, AActor* ActorToAdd)
{
    const TSharedPtr<FQuest> QuestWhereStepBelongs = GetQuestByID(QuestIDToGet);
    const TSharedPtr<FQuestStepObjective> StepQuest = GetStepQuestByID(StepQuestID, QuestWhereStepBelongs);
    StepQuest->AddAssociatedActor(ActorToAdd);
    StepQuest->AddIconMarkerToAssociatedActor();
}

void AQuestManager::RemoveAllActiveQuests_Implementation()
{
    for (int i = ActiveQuests.Num() - 1; i >= 0; i--)
    {
        GetQuestByID(ActiveQuests[i].QuestID)->ClearQuest();
        ActiveQuests.RemoveAt(i);
    }
}

const FQuestStateInfo AQuestManager::GetCurrentActiveQuestInfo() const
{
    for (auto ActiveQuest : ActiveQuests)
    {
        if (ActiveQuest.CurrentActive)
        {
            return ActiveQuest;
        }
    }
    return FQuestStateInfo();
}

const TArray<FQuestStateInfo> AQuestManager::GetAllActiveQuestsInfo()
{
    return ActiveQuests;
}

const FQuest AQuestManager::GetCurrentActiveQuest()
{
    FQuestStateInfo QuestState = GetCurrentActiveQuestInfo();
    return QuestState.IsValid() ? *GetQuestByID(QuestState.QuestID) : FQuest();
}

void AQuestManager::SpawnActor_Implementation(TSubclassOf<AActor> ActorToSpawn, FVector WorldPositionToSpawn, FRotator WorldRotationToSpawn)
{
    FActorSpawnParameters SpawnParameters;
    SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    LastSpawnedActor = GetWorld()->SpawnActor<AActor>(ActorToSpawn, WorldPositionToSpawn, WorldRotationToSpawn, SpawnParameters);
}

void AQuestManager::OnQuestStepArrivedToPlace_Implementation(int StepID, int QuestID, FGameplayTag ArrivedPlace, APlayerController* ArrivedBy)
{
    TSharedPtr<FQuest> QuestWhereStepBelongs = GetQuestByID(QuestID);
    TSharedPtr<FQuestStepObjective> StepQuest = GetStepQuestByID(StepID, QuestWhereStepBelongs);
    TSharedPtr<FQuestStepGoToObjective> GoToObjective = StaticCastSharedPtr<FQuestStepGoToObjective>(StepQuest);
    GoToObjective.Get()->OnArrivedToPlace(ArrivedBy);
}

void AQuestManager::OnQuestStepEntityTalkedTo_Implementation(int StepID, int QuestID, FGameplayTag TalkedEntity, APlayerController* TalkedBy)
{
    TSharedPtr<FQuest> QuestWhereStepBelongs = GetQuestByID(QuestID);
    TSharedPtr<FQuestStepObjective> StepQuest = GetStepQuestByID(StepID, QuestWhereStepBelongs);
    TSharedPtr<FQuestStepTalkWithObjective> TalkToObjective = StaticCastSharedPtr<FQuestStepTalkWithObjective>(StepQuest);
    TalkToObjective.Get()->OnTalkedWithEntity(TalkedBy);
}

void AQuestManager::OnQuestStepEntityKilled_Implementation(int StepID, int QuestID, FGameplayTag EntityKilled, APlayerController* KilledBy)
{
    TSharedPtr<FQuest> QuestWhereStepBelongs = GetQuestByID(QuestID);
    TSharedPtr<FQuestStepObjective> StepQuest = GetStepQuestByID(StepID, QuestWhereStepBelongs);
    TSharedPtr<FQuestStepKillObjective> KillObjective = StaticCastSharedPtr<FQuestStepKillObjective>(StepQuest);
    KillObjective.Get()->OnEntityKilled(KilledBy);
}

void AQuestManager::OnQuestStepItemGathered_Implementation(int StepID, int QuestID, FGameplayTag ItemGathered, float amountGathered, APlayerController* GatheredBy)
{
    TSharedPtr<FQuest> QuestWhereStepBelongs = GetQuestByID(QuestID);
    TSharedPtr<FQuestStepObjective> StepQuest = GetStepQuestByID(StepID, QuestWhereStepBelongs);
    TSharedPtr<FQuestStepGatherObjective> GatherObjective = StaticCastSharedPtr<FQuestStepGatherObjective>(StepQuest);
    GatherObjective.Get()->OnItemGathered(GatheredBy, amountGathered);
}

void AQuestManager::OnQuestStepCatch_Implementation(int StepID, int QuestID, FGameplayTag CatchTag, APlayerController* CatchedBy)
{
    TSharedPtr<FQuest> QuestWhereStepBelongs = GetQuestByID(QuestID);
    TSharedPtr<FQuestStepObjective> StepQuest = GetStepQuestByID(StepID, QuestWhereStepBelongs);
    TSharedPtr<FQuestStepCatchObjective> GatherObjective = StaticCastSharedPtr<FQuestStepCatchObjective>(StepQuest);
    GatherObjective.Get()->OnCatched(CatchedBy);
}

void AQuestManager::OnRep_OnActiveQuests()
{
    for (auto ActiveQuest : ActiveQuests)
    {
        AddActiveQuest(ActiveQuest.QuestID, ActiveQuest.CurrentActive, ActiveQuest.CurrentStepQuestObjectID, false);
    }
}

AActor* AQuestManager::GetStepQuestReference(int QuestID, FGameplayTag ReferenceTag)
{
    if (QuestReferences.Contains(QuestID))
    {
        for (const auto QuestReference : QuestReferences[QuestID].QuestActors)
        {
            if (QuestReference.ReferenceTag == ReferenceTag)
            {
                return QuestReference.ReferenceActor;
            }
        }
    }
    return nullptr;
}

bool AQuestManager::HasCurrentActiveQuest() const
{
    return GetCurrentActiveQuestInfo().IsValid();
}

bool AQuestManager::IsQuestCompleted(FQuest QuestToCheck)
{
    return QuestToCheck.IsQuestCompleted();
}

bool AQuestManager::IsCurrentActiveQuest(FQuest QuestToCompare)
{
    FQuestStateInfo CurrentQuestState = GetCurrentActiveQuestInfo();
    return CurrentQuestState.IsValid()? QuestToCompare.QuestID == CurrentQuestState.QuestID : false;
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

    DOREPLIFETIME_CONDITION_NOTIFY(AQuestManager, ActiveQuests, COND_InitialOnly, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(AQuestManager, CompletedQuests, COND_InitialOnly, REPNOTIFY_Always);
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

void AQuestManager::ActivateQuestObjectives(int QuestID, int StepIDToActivate)
{
    if (QuestID != -1)
    {
        TSharedPtr<FQuest> Quest = GetQuestByID(QuestID);
        for (int i = 0; i < StepIDToActivate; ++i)
        {
            const auto Objective = Quest.Get()->ObjectivesArray[i];
            Objective->Activate(GetWorld(), this);
            Objective->SetCompleted();
        }
        const auto Objective = Quest.Get()->ObjectivesArray[StepIDToActivate];
        Objective->Activate(GetWorld(), this);
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
                if (IQuestObject* ActorAsQuestObject = Cast<IQuestObject>(QuestActor.ReferenceActor))
                {
                    ActorAsQuestObject->ActivateObject();
                    IQuestObject::Execute_BP_ActivateObject(QuestActor.ReferenceActor);
                }
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
                if (IQuestObject* ActorAsQuestObject = Cast<IQuestObject>(QuestActor.ReferenceActor))
                {
                    ActorAsQuestObject->DeactivateObject(false);
                    IQuestObject::Execute_BP_DeactivateObject(QuestActor.ReferenceActor, false);
                }
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
    CompletedQuests.Add(CompletedQuestID);

    // TODO: Maybe we want to put the next main quest as the current quest?
}

void AQuestManager::OnStepQuestCompleted_Implementation(int CompletedStepQuestID, int QuestIDWhereStepBelongs)
{
    TSharedPtr<FQuest> QuestWhereStepBelongs = GetQuestByID(QuestIDWhereStepBelongs);
    TSharedPtr<FQuestStepObjective> CompletedStepQuest = GetStepQuestByID(CompletedStepQuestID, QuestWhereStepBelongs);
    auto NextObjective = QuestWhereStepBelongs->GetCurrentObjectiveSharedPtr();
    if (NextObjective.IsValid())
    {
        NextObjective->Activate(GetWorld(), this);

        for (FQuestStateInfo& ActiveQuest : ActiveQuests)
        {
            if (ActiveQuest.QuestID == QuestIDWhereStepBelongs)
            {
                ActiveQuest.CurrentStepQuestObjectID++;
            }
        }
    }
    CompletedStepQuest->SetCompleted();
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
        UIconMarkerComponent* IconMarkerComponent = Cast<UIconMarkerComponent>(ActorsAssociated[ActorsAssociated.Num() - 1]->AddComponentByClass(UIconMarkerComponent::StaticClass(), false, FTransform(), false));
        IconMarkerComponent->SetIsReplicated(true);
        IconMarkerComponent->SetMarkerUMGToUse(ObjectiveMarkerUMGInformation.ObjectiveMarkerUMGClass);
        IconMarkerComponent->bShowOnCompass = ObjectiveMarkerUMGInformation.bShowOnCompass;
        IconMarkerComponent->bShowOnScreen = ObjectiveMarkerUMGInformation.bShowOnScreen;
        IconMarkerComponent->MarkerIcon = ObjectiveMarkerUMGInformation.IconToUse;
        IconMarkerComponent->ActorOffset = ObjectiveMarkerUMGInformation.MarkerToActorOffset;
    }
}

void FQuestStepGoToObjective::Activate(UWorld* WorldContext, AQuestManager* QuestManager)
{
    AActor* ReferenceActor = nullptr;
    if (ActorReference != FGameplayTag::EmptyTag)
    {
        ReferenceActor = QuestManager->GetStepQuestReference(ParentQuestID, ActorReference);
    }

    if (!ReferenceActor)
    {
        TArray<AActor*> GoToLocations;
        UGameplayStatics::GetAllActorsOfClass(WorldContext, ALocationTrigger::StaticClass(), GoToLocations);
        for (AActor* LocationTrigger : GoToLocations)
        {
            if (ALocationTrigger* LocTrigger = Cast<ALocationTrigger>(LocationTrigger))
            {
                if (LocTrigger->GetLocation() == PlaceToGo)
                {
                    ReferenceActor = LocTrigger;
                }
            }
        }
    }

    if (ReferenceActor)
    {
        QuestManager->AddAssociatedActorToQuestStep(StepObjectiveInsideQuestOrder, ParentQuestID, ReferenceActor);
        if (ReferenceActor->FindComponentByClass<UIconMarkerComponent>() == nullptr)
        {
            AddIconMarkerToAssociatedActor();
        }
    }
    Super::Activate(WorldContext, QuestManager);
}

void FQuestStepTalkWithObjective::Activate(UWorld* WorldContext, AQuestManager* QuestManager)
{
    if (PawnToSpawnWhenActive)
    {
        QuestManager->SpawnActor(PawnToSpawnWhenActive, WorldPositionToSpawn, WorldRotationToSpawn);

        if (AActor* SpawnedActor = QuestManager->GetLastSpawnedActor())
        {
            SpawnedActors.Add(SpawnedActor);
            if (IQuestObject* ActorAsQuestObject = Cast<IQuestObject>(SpawnedActor))
            {
                ActorAsQuestObject->SetTag(EntityToTalkWith);
            }
            QuestManager->AddAssociatedActorToQuestStep(StepObjectiveInsideQuestOrder, ParentQuestID, SpawnedActor);
        }
    }

    if (ActorReference != FGameplayTag::EmptyTag)
    {
        if (AActor* ReferenceActor = QuestManager->GetStepQuestReference(ParentQuestID, ActorReference))
        {
            QuestManager->AddAssociatedActorToQuestStep(StepObjectiveInsideQuestOrder, ParentQuestID, ReferenceActor);
        }
    }
    
    Super::Activate(WorldContext, QuestManager);
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
           
            if (AActor* SpawnedActor = QuestManager->GetLastSpawnedActor())
            {
                SpawnedActors.Add(SpawnedActor);
                QuestManager->AddAssociatedActorToQuestStep(StepObjectiveInsideQuestOrder, ParentQuestID, SpawnedActor);
            }
        }
    }

    if (ActorReference != FGameplayTag::EmptyTag)
    {
        if (AActor* ReferenceActor = QuestManager->GetStepQuestReference(ParentQuestID, ActorReference))
        {
            QuestManager->AddAssociatedActorToQuestStep(StepObjectiveInsideQuestOrder, ParentQuestID, ReferenceActor);
        }
    }

    Super::Activate(WorldContext, QuestManager);
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
        ObjectiveToDeactivate->Deactivate(false);
    }
}