// Copyright � Pedro Costa, 2021. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include <UI/IconMarkerUMG.h>
#include <Components/IconMarkerComponent.h>

#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameStateBase.h"
#include "Interface/QuestObject.h"
#include "Kismet/KismetSystemLibrary.h"
#include "QuestManager.generated.h"

class IQuestObject;

UENUM(BlueprintType)
enum class EQuestType : uint8
{
    Main,
    Side,
    Errand,
};

UENUM(BlueprintType)
enum class EQuestStepType : uint8
{
    None,
    GoTo,
    TalkWith,
    Kill,
    Gather,
    Catch,
};

UENUM(BlueprintType)
enum class ERewardTypes : uint8
{
    None,
    XP,
    Gold,
    Skill,
};

USTRUCT(BlueprintType)
struct PCQUESTSYSTEM_API FIconMarkerInformation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
    bool bCreateMarker;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective, meta = (EditCondition = "bCreateMarker == true"))
    TSubclassOf<UIconMarkerUMG> ObjectiveMarkerUMGClass;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective, meta = (EditCondition = "bCreateMarker == true"))
    UTexture2D* IconToUse;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective, meta = (EditCondition = "bCreateMarker == true"))
    bool bShowOnCompass;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective, meta = (EditCondition = "bCreateMarker == true"))
    bool bShowOnScreen;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective, meta = (EditCondition = "bCreateMarker == true"))
    FVector MarkerToActorOffset;
};

USTRUCT(BlueprintType)
struct PCQUESTSYSTEM_API FSpawnInformation
{
    GENERATED_BODY()

    /* Actor to spawn when this step is activated */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
        TArray<TSubclassOf<AActor>> PawnsToSpawnWhenActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
        float NumToSpawnOfEachPawn = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
        FVector SpawnCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
        float SpawnRange;
};

USTRUCT(BlueprintType)
struct PCQUESTSYSTEM_API FQuestStepObjective
{
    GENERATED_BODY()


        /** Constructor */
        FQuestStepObjective()
    {
        StepObjectiveID = FGuid();
        bIsCompleted = false;
        StepObjectiveInsideQuestOrder = -1;
    }
    FQuestStepObjective(int QuestID, int StepObjectiveOrder, FText QuestDescription, FGameplayTag ReferenceTag, bool bAllPlayers, TMap<ERewardTypes, float> Rewards, EQuestStepType StepType, FIconMarkerInformation MarkerInfo)
        : StepObjectiveInsideQuestOrder(StepObjectiveOrder),
        ActorReference(ReferenceTag),
        bRequiresAllPlayers(bAllPlayers),
        QuestStepRewards(Rewards),
        QuestStepType(StepType),
        ObjectiveMarkerUMGInformation(MarkerInfo),
        Description(QuestDescription),
        ParentQuestID(QuestID)
    {
        StepObjectiveID = FGuid();
        bIsCompleted = false;
    }
    virtual ~FQuestStepObjective()
    {

    }

    FGuid StepObjectiveID;

    /* Order that this objective will appear*/
    int StepObjectiveInsideQuestOrder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Quest)
    FGameplayTag ActorReference = FGameplayTag::EmptyTag;

    /** If we need all players to complete the quest */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Quest)
    bool bRequiresAllPlayers = false;

    /** Quest Step XP to give */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
        TMap<ERewardTypes, float> QuestStepRewards;

    /** Quest Step Type */
    EQuestStepType QuestStepType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
        FIconMarkerInformation ObjectiveMarkerUMGInformation;

    /* Step description */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
    FText Description;

    int ParentQuestID;

    FString SplitEnumString(FString EnumString);
    FText GetStepDescription() { return Description; };
    /* Spawn/collect necessary actors */

    virtual void Activate(UWorld* WorldContext, AQuestManager* QuestManager)
    {
        ActivateActorMarker();
    };
    
    virtual void Deactivate()
    {
        for (AActor* AssociatedActor : ActorsAssociated)
        {
            if (UIconMarkerComponent* ActorMarkerComponent = AssociatedActor->FindComponentByClass<UIconMarkerComponent>())
            {
                ActorMarkerComponent->DeactivateMarker();
            }
            
            if (IQuestObject* ActorAsQuestObject = Cast<IQuestObject>(AssociatedActor))
            {
                ActorAsQuestObject->DeactivateObject();
                ActorAsQuestObject->Execute_BP_DeactivateObject(AssociatedActor);
            }
        }
    };
    
    virtual void OnCompleted(APlayerController* CompletedBy)
    {
        if (CompletedBy == nullptr)
        {
            return;
        }
        
        if (!bRequiresAllPlayers)
        {
            bIsCompleted = true;
            return;
        }
        CompletedControllers.AddUnique(CompletedBy);
        bIsCompleted = CompletedControllers.Num() == CompletedBy->GetWorld()->GetGameState()->PlayerArray.Num();
    }

    void ResetStepQuest()
    {
        Deactivate();
        bIsCompleted = false;
    }

    virtual void ActivateActorMarker()
    {
        for (AActor* AssociatedActor : ActorsAssociated)
        {
            if (UIconMarkerComponent* ActorMarkerComponent = AssociatedActor->FindComponentByClass<UIconMarkerComponent>())
            {
                ActorMarkerComponent->ActivateMarker();
            }
        }
    };
    bool IsCompleted() { return bIsCompleted; };
    // This is so we can tell to the clients that this is done
    // instead of having them control when should a step be completed which is server's job
    // Also used when we load a quest and we want to activate an objective that is not the first one
    void SetCompleted()
    {
        bIsCompleted = true;
        Deactivate();
    };

    void AddIconMarkerToAssociatedActor();

    void AddAssociatedActor(AActor* actorToAdd)
    {
        ActorsAssociated.Add(actorToAdd);
    }

    bool IsValid()
    {
        return StepObjectiveInsideQuestOrder > 0 && QuestStepType != EQuestStepType::None;
    }
protected:
    bool bIsCompleted;
    UPROPERTY()
    TArray<AActor*> ActorsAssociated;
    UPROPERTY()
    TArray<APlayerController*> CompletedControllers;
};

USTRUCT(BlueprintType)
struct PCQUESTSYSTEM_API FQuestStepGoToObjective : public FQuestStepObjective
{
    GENERATED_BODY()


        /** Constructor */
        FQuestStepGoToObjective()
    {
        QuestStepType = EQuestStepType::GoTo;
    }
    FQuestStepGoToObjective(int QuestID, int StepObjectiveOrder, FText QuestDescription, FGameplayTag ReferenceTag, bool bAllPlayers, TMap<ERewardTypes, float> rewards, FIconMarkerInformation MarkerInfo, FGameplayTag Place)
        :Super(QuestID, StepObjectiveOrder, QuestDescription, ReferenceTag, bAllPlayers, rewards, EQuestStepType::GoTo, MarkerInfo),
        PlaceToGo(Place)
    {
    }

    /** Place that we want to go */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
        FGameplayTag PlaceToGo;

    void OnArrivedToPlace(APlayerController* ArrivedBy) { OnCompleted(ArrivedBy); };

    void Activate(UWorld* WorldContext, AQuestManager* QuestManager) override;

};

USTRUCT(BlueprintType)
struct PCQUESTSYSTEM_API FQuestStepTalkWithObjective : public FQuestStepObjective
{
    GENERATED_BODY()


        /** Constructor */
        FQuestStepTalkWithObjective()
    {
        QuestStepType = EQuestStepType::TalkWith;
    }
    FQuestStepTalkWithObjective(int QuestID, int StepObjectiveOrder, FText QuestDescription, FGameplayTag ReferenceTag, bool bAllPlayers, TMap<ERewardTypes, float> rewards, FIconMarkerInformation MarkerInfo, TSubclassOf<AActor> PawnToSpawnClass, FVector SpawnWorldPosition, FRotator SpawnWorldRotation, FGameplayTag EntityToTalk)
        :Super(QuestID, StepObjectiveOrder, QuestDescription, ReferenceTag, bAllPlayers, rewards, EQuestStepType::TalkWith, MarkerInfo),
        PawnToSpawnWhenActive(PawnToSpawnClass),
        WorldPositionToSpawn(SpawnWorldPosition),
        WorldRotationToSpawn(SpawnWorldRotation),
        EntityToTalkWith(EntityToTalk)
    {
    }

    /* Actor to spawn when this step is activated */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective, meta = (EditCondition = "ActorReference == FGameplayTag::Empty"))
        TSubclassOf<AActor> PawnToSpawnWhenActive;
    /* Where to spawn the actor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective, meta = (EditCondition = "ActorReference == FGameplayTag::Empty"))
        FVector WorldPositionToSpawn;
    /* Which rotation should the actor have */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective, meta = (EditCondition = "ActorReference == FGameplayTag::Empty"))
        FRotator WorldRotationToSpawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
        FGameplayTag EntityToTalkWith;

    void OnTalkedWithEntity(APlayerController* TalkedBy) { OnCompleted(TalkedBy); }

    void Activate(UWorld* WorldContext, AQuestManager* QuestManager) override;

};

USTRUCT(BlueprintType)
struct PCQUESTSYSTEM_API FQuestStepKillObjective : public FQuestStepObjective
{
    GENERATED_BODY()


        /** Constructor */
        FQuestStepKillObjective()
    {
        QuestStepType = EQuestStepType::Kill;
    }
    FQuestStepKillObjective(int QuestID, int StepObjectiveOrder, FText QuestDescription, FGameplayTag ReferenceTag, bool bAllPlayers, TMap<ERewardTypes, float> rewards, FIconMarkerInformation MarkerInfo, FSpawnInformation SpawnInfo, FGameplayTag EntityKill, int KillAmount)
        :Super(QuestID, StepObjectiveOrder, QuestDescription, ReferenceTag, bAllPlayers, rewards, EQuestStepType::Kill, MarkerInfo),
        SpawnInformation(SpawnInfo),
        EntityToKill(EntityKill),
        AmountToKill(KillAmount)
    {
    }

    /* Actor to spawn when this step is activated */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective, meta = (EditCondition = "ActorReference == FGameplayTag::Empty"))
        FSpawnInformation SpawnInformation;

    /** Who/what we want to kill */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
        FGameplayTag EntityToKill;

    /* How many we want to kill */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
        int AmountToKill;

    void OnEntityKilled(APlayerController* KilledBy)
    {
        bIsCompleted = ++CurrentlyKilled >= AmountToKill;
        if (bIsCompleted)
        {
            OnCompleted(KilledBy);
        }
    }
private:
    int CurrentlyKilled = 0;
public:
    void Activate(UWorld* WorldContext, AQuestManager* QuestManager) override;

};

USTRUCT(BlueprintType)
struct PCQUESTSYSTEM_API FQuestStepGatherObjective : public FQuestStepObjective
{
    GENERATED_BODY()


        /** Constructor */
        FQuestStepGatherObjective()
    {
        QuestStepType = EQuestStepType::Gather;
    }
    FQuestStepGatherObjective(int QuestID, int StepObjectiveOrder, FText QuestDescription, FGameplayTag ReferenceTag, bool bAllPlayers, TMap<ERewardTypes, float> rewards, FIconMarkerInformation MarkerInfo, FGameplayTag ItemGather, int GaterAmount)
        :Super(QuestID, StepObjectiveOrder, QuestDescription, ReferenceTag, bAllPlayers, rewards, EQuestStepType::Gather, MarkerInfo),
        ItemToGather(ItemGather),
        AmountToGather(GaterAmount)
    {
    }

    /** What we want to gather */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
        FGameplayTag ItemToGather;

    /* How many we want to gather */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
        float AmountToGather;

    void OnItemGathered(APlayerController* GatheredBy, float amountGathered)
    {
        CurrentlyGathered += amountGathered;
        bIsCompleted = CurrentlyGathered >= AmountToGather;
        if (bIsCompleted)
        {
            OnCompleted(GatheredBy);
        }
    }
private:
    float CurrentlyGathered = 0;

};

USTRUCT(BlueprintType)
struct PCQUESTSYSTEM_API FQuestStepCatchObjective : public FQuestStepObjective
{
    GENERATED_BODY()


    /** Constructor */
    FQuestStepCatchObjective()
    {
        QuestStepType = EQuestStepType::Catch;
    }
    FQuestStepCatchObjective(int QuestID, int StepObjectiveOrder, FText QuestDescription, FGameplayTag ReferenceTag, bool bAllPlayers, TMap<ERewardTypes, float> rewards, FIconMarkerInformation MarkerInfo, TArray<FGameplayTag> TagCatch)
        :Super(QuestID, StepObjectiveOrder, QuestDescription, ReferenceTag, bAllPlayers, rewards, EQuestStepType::Catch, MarkerInfo),
        AllowedTagToCatch(TagCatch)
    {
    }

    /** What we want to gather */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
    TArray<FGameplayTag> AllowedTagToCatch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
    int AmountNeeded = 1;

    void OnCatched(APlayerController* CatchedBy)
    {
        bIsCompleted = ++CurrentlyCatched >= AmountNeeded;
        if (bIsCompleted)
        {
            OnCompleted(CatchedBy);
        }
    }
private:
    int CurrentlyCatched = 0;

};

USTRUCT(BlueprintType)
struct PCQUESTSYSTEM_API FQuest : public FTableRowBase
{
    GENERATED_BODY()


    /** Constructor */
    FQuest()
    {
    }

    FQuest(int QuestId, EQuestType Type, FText QuestName, TMap<ERewardTypes, float> rewards,
        TMap<int, FQuestStepGoToObjective> GoToObjectivesMap, TMap<int, FQuestStepTalkWithObjective> TalkWithObjectivesMap,
        TMap<int, FQuestStepKillObjective> KillObjectivesMap, TMap<int, FQuestStepGatherObjective> GatherObjectivesMap, TMap<int, FQuestStepCatchObjective> CatchObjectivesMap)
        : QuestID(QuestId),
        QuestType(Type),
        Name(QuestName),
        QuestRewards(rewards),
        GoToObjectives(GoToObjectivesMap),
        TalkWithObjectives(TalkWithObjectivesMap),
        KillObjectives(KillObjectivesMap),
        GatherObjectives(GatherObjectivesMap),
        CatchObjectives(CatchObjectivesMap)
    {
        Init();
    }

    /** Quest ID */
    int QuestID = -1;

    /** Quest Type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Quest)
        EQuestType QuestType;

    /** Quest Name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Quest)
        FText Name;

    /** Quest XP to give */
    UPROPERTY(NotReplicated, EditAnywhere, BlueprintReadWrite, Category = Quest)
        TMap<ERewardTypes, float> QuestRewards;

    /* Quest Objectives */
    UPROPERTY(NotReplicated, EditAnywhere, BlueprintReadWrite, Category = Quest)
        TMap<int, FQuestStepGoToObjective> GoToObjectives;
    UPROPERTY(NotReplicated,EditAnywhere, BlueprintReadWrite, Category = Quest)
        TMap<int, FQuestStepTalkWithObjective> TalkWithObjectives;
    UPROPERTY(NotReplicated,EditAnywhere, BlueprintReadWrite, Category = Quest)
        TMap<int, FQuestStepKillObjective> KillObjectives;
    UPROPERTY(NotReplicated, EditAnywhere, BlueprintReadWrite, Category = Quest)
        TMap<int, FQuestStepGatherObjective> GatherObjectives;
    UPROPERTY(NotReplicated, EditAnywhere, BlueprintReadWrite, Category = Quest)
        TMap<int, FQuestStepCatchObjective> CatchObjectives;

    /* This is so we can use pointers and cast to our specific data type */
    TArray<TSharedPtr<FQuestStepObjective>> ObjectivesArray;

    void Init()
    {
        TArray<FQuestStepObjective> Objectives;
        for (auto& Elem : GoToObjectives)
        {
            ObjectivesArray.Emplace(MakeShareable(new FQuestStepGoToObjective(QuestID, Elem.Key, Elem.Value.Description, Elem.Value.ActorReference, Elem.Value.bRequiresAllPlayers, Elem.Value.QuestStepRewards, Elem.Value.ObjectiveMarkerUMGInformation, Elem.Value.PlaceToGo)));
        }

        for (auto& Elem : TalkWithObjectives)
        {
            ObjectivesArray.Emplace(MakeShareable(new FQuestStepTalkWithObjective(QuestID, Elem.Key, Elem.Value.Description, Elem.Value.ActorReference, Elem.Value.bRequiresAllPlayers, Elem.Value.QuestStepRewards, Elem.Value.ObjectiveMarkerUMGInformation, Elem.Value.PawnToSpawnWhenActive, Elem.Value.WorldPositionToSpawn, Elem.Value.WorldRotationToSpawn, Elem.Value.EntityToTalkWith)));
        }

        for (auto& Elem : KillObjectives)
        {
            ObjectivesArray.Emplace(MakeShareable(new FQuestStepKillObjective(QuestID, Elem.Key, Elem.Value.Description, Elem.Value.ActorReference, Elem.Value.bRequiresAllPlayers, Elem.Value.QuestStepRewards, Elem.Value.ObjectiveMarkerUMGInformation, Elem.Value.SpawnInformation, Elem.Value.EntityToKill, Elem.Value.AmountToKill)));
        }

        for (auto& Elem : GatherObjectives)
        {
            ObjectivesArray.Emplace(MakeShareable(new FQuestStepGatherObjective(QuestID, Elem.Key, Elem.Value.Description, Elem.Value.ActorReference, Elem.Value.bRequiresAllPlayers, Elem.Value.QuestStepRewards, Elem.Value.ObjectiveMarkerUMGInformation, Elem.Value.ItemToGather, Elem.Value.AmountToGather)));
        }

        for (auto& Elem : CatchObjectives)
        {
            ObjectivesArray.Emplace(MakeShareable(new FQuestStepCatchObjective(QuestID, Elem.Key, Elem.Value.Description, Elem.Value.ActorReference, Elem.Value.bRequiresAllPlayers, Elem.Value.QuestStepRewards, Elem.Value.ObjectiveMarkerUMGInformation, Elem.Value.AllowedTagToCatch)));
        }
        ObjectivesArray.Sort([](TSharedPtr<FQuestStepObjective> StepObjective1, TSharedPtr<FQuestStepObjective> StepObjective2) { return StepObjective1->StepObjectiveInsideQuestOrder < StepObjective2->StepObjectiveInsideQuestOrder; });
    }

    void ResetQuest()
    {
        for (TSharedPtr<FQuestStepObjective> Objective : ObjectivesArray)
        {
            Objective->ResetStepQuest();
        }
    }

    bool IsQuestCompleted() const
    {
        for (TSharedPtr<FQuestStepObjective> Objective : ObjectivesArray)
        {
            if (!Objective->IsCompleted())
            {
                return false;
            }
        }
        return true;
    }

    bool HasQuestStarted()
    {
        for (TSharedPtr<FQuestStepObjective> Objective : ObjectivesArray)
        {
            if (Objective->IsCompleted())
            {
                return true;
            }
        }
        return false;
    }

    FQuestStepObjective GetCurrentObjective() const
    {
        if (!IsQuestCompleted())
        {
            TSharedPtr<FQuestStepObjective> CurrentObjective = *ObjectivesArray.FindByPredicate([](const TSharedPtr<FQuestStepObjective> objective)
            {
                return !objective->IsCompleted();
            });
            return *CurrentObjective.Get();
        }
        return FQuestStepObjective();
    }

    TSharedPtr<FQuestStepObjective> GetCurrentObjectiveSharedPtr() const
    {
        if (!IsQuestCompleted())
        {
            TSharedPtr<FQuestStepObjective> CurrentObjective = *ObjectivesArray.FindByPredicate([](const TSharedPtr<FQuestStepObjective> objective)
            {
                return !objective->IsCompleted();
            });
            return CurrentObjective;
        }
        return TSharedPtr<FQuestStepObjective>();
    }

    TSharedPtr<FQuestStepObjective> GetStepObjectiveById(int StepID)
    {
        TSharedPtr<FQuestStepObjective>* CurrentObjective = ObjectivesArray.FindByPredicate([=](const TSharedPtr<FQuestStepObjective> objective)
        {
            return objective->StepObjectiveInsideQuestOrder == StepID;
        });
        return *CurrentObjective;
    }

    void ActivateCurrentObjective(UWorld* WordContext, AQuestManager* QuestManager);
    void DeactivateObjective(TSharedPtr<FQuestStepObjective> ObjectiveToDeactivate);

    bool IsValid() const
    {
        return QuestID > -1;
    }
};

USTRUCT(BlueprintType)
struct PCQUESTSYSTEM_API FQuestActorReference
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag ReferenceTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* ReferenceActor;
};

USTRUCT(BlueprintType)
struct PCQUESTSYSTEM_API FQuestActorReferences
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FQuestActorReference> QuestActors;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestActivated, FQuest, ActivatedQuest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, FQuest, CompletedQuest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStepCompleted, FQuestStepObjective, CompletedStepQuest, FQuest, QuestWhereStepBelongs);

/**
 *
 */
UCLASS()
class PCQUESTSYSTEM_API AQuestManager : public AActor
{
    
    GENERATED_BODY()

    AQuestManager();
    ~AQuestManager();
public:
    UPROPERTY(BlueprintAssignable, Category = "QuestManager")
        FOnQuestActivated OnQuestActivated;
    UPROPERTY(BlueprintAssignable, Category = "QuestManager")
        FOnQuestCompleted OnQuestCompletedDelegate;
    UPROPERTY(BlueprintAssignable, Category = "QuestManager")
        FOnQuestStepCompleted OnQuestStepCompletedDelegate;

    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "QuestManager")
        void ActivateQuest(int QuestIDToActivate, int StepIDToActivate = 0);
    UFUNCTION(BlueprintCallable, Category = "QuestManager")
        void OnAfterQuestActivated(int QuestIDToActivate, bool bBroadcastEvent);
    UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "QuestManager")
        void ResetQuest(int QuestIDToActivate);
    UFUNCTION(BlueprintCallable, Category = "QuestManager")
        void SetCurrentActiveQuest(int QuestIDToActivate);
    UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "QuestManager")
        void AddActiveQuest(int QuestIDToActivate, int StepIDToActivate = 0, bool bBroadcastEvent = true);
    UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "QuestManager")
        void RemoveActiveQuest(int QuestIDToRemove);
    UFUNCTION(BlueprintPure, Category = "QuestManager")
        const FQuest GetCurrentActiveQuest();
    UFUNCTION(BlueprintPure, Category = "QuestManager")
        const TArray<FQuest> GetActiveQuests();
    UFUNCTION(BlueprintPure, Category = "QuestManager")
        const TArray<FQuest> GetAllQuests();

    UFUNCTION(Server, Reliable, Category = "QuestManager")
        void OnArrivedToPlace(FGameplayTag ArrivedPlace, APlayerController* ArrivedBy);
    UFUNCTION(NetMulticast, Reliable, Category = "QuestManager")
        void OnQuestStepArrivedToPlace(int StepID, int QuestID, FGameplayTag ArrivedPlace, APlayerController* ArrivedBy);
    UFUNCTION(Server, Reliable, Category = "QuestManager")
        void OnEntityTalkedTo(FGameplayTag TalkedEntity, APlayerController* TalkedBy);
    UFUNCTION(NetMulticast, Reliable, Category = "QuestManager")
        void OnQuestStepEntityTalkedTo(int StepID, int QuestID, FGameplayTag TalkedEntity, APlayerController* TalkedBy);
    UFUNCTION(Server, Reliable, Category = "QuestManager")
        void OnEntityKilled(FGameplayTag EntityKilled, APlayerController* KilledBy);
    UFUNCTION(NetMulticast, Reliable, Category = "QuestManager")
        void OnQuestStepEntityKilled(int StepID, int QuestID, FGameplayTag EntityKilled, APlayerController* KilledBy);
    UFUNCTION(Server, Reliable, Category = "QuestManager")
        void OnItemGathered(FGameplayTag ItemGathered, float amountGathered, APlayerController* GatheredBy);
    UFUNCTION(NetMulticast, Reliable, Category = "QuestManager")
        void OnQuestStepItemGathered(int StepID, int QuestID, FGameplayTag ItemGathered, float amountGathered, APlayerController* GatheredBy);
    UFUNCTION(Server, Reliable, Category = "QuestManager")
        void OnCatch(FGameplayTag CatchTag, APlayerController* CatchedBy);
    UFUNCTION(NetMulticast, Reliable, Category = "QuestManager")
        void OnQuestStepCatch(int StepID, int QuestID, FGameplayTag CatchTag, APlayerController* CatchedBy);

    UFUNCTION()
    void OnRep_OnCurrentActiveQuest();
    
    UFUNCTION(Server, Reliable)
        void SpawnActor(TSubclassOf<AActor> ActorToSpawn, FVector WorldPositionToSpawn, FRotator WorldRotationToSpawn);
    AActor* GetStepQuestReference(int QuestID, FGameplayTag ReferenceTag);

    UFUNCTION(BlueprintPure, Category = "QuestManager")
    bool HasCurrentActiveQuest() const;
    UFUNCTION(BlueprintPure, Category = "QuestManager")
        bool IsQuestCompleted(FQuest QuestToCheck);
    UFUNCTION(BlueprintPure, Category = "QuestManager")
        bool IsCurrentActiveQuest(FQuest QuestToCompare);
    UFUNCTION(BlueprintPure, Category = "QuestManager")
        bool IsCurrentQuestStepObjective(FQuestStepObjective QuestStepToCompare);

    UFUNCTION(BlueprintPure, Category = "QuestManager")
        const FQuestStepObjective GetCurrentQuestCurrentObjective();
    UFUNCTION(BlueprintPure, Category = "QuestManager")
        FText GetStepObjectiveDescription(FQuestStepObjective QuestStep);

    void AddAssociatedActorToQuestStep(int StepQuestID, int QuestIDToGet, AActor* ActorToAdd);
    AActor* GetLastSpawnedActor();
private:
    TSharedPtr<FQuest> GetQuestByID(int IDToGet);
    TSharedPtr<FQuest> GetQuestByID(int IDToGet) const;
    TSharedPtr<FQuestStepObjective> GetStepQuestByID(int IDToGet, TSharedPtr<FQuest> QuestToSearch);

    void ActivateQuestObjectives(int QuestID, int StepIDToActivate = 0);
    void ActivateQuestReferences(int QuestID);
    void DeactivateQuestReferences(int QuestID);
    void OnQuestCompleted(TSharedPtr<FQuest> CompletedQuest);
    UFUNCTION(NetMulticast, reliable)
    void OnQuestCompletedNextTick(int CompletedQuestID);
    UFUNCTION(NetMulticast, reliable)
    void OnStepQuestCompleted(int CompletedStepQuestID, int QuestIDWhereStepBelongs);
    FQuestStepObjective GetCurrentQuestCurrentObjective() const;
private:
    UPROPERTY(ReplicatedUsing = OnRep_OnCurrentActiveQuest)
    int CurrentActiveQuest = -1;
    UPROPERTY(Replicated)
    TArray<int> ActiveQuests = {};
    
    TArray<TSharedPtr<FQuest>> AllQuests;
    UPROPERTY()
    AActor* LastSpawnedActor;
    /** Pointer to table where the quests come from */
    UPROPERTY(EditAnywhere, Category = DataTableRowHandle)
    const UDataTable* DataTable;

    /** Things to activate/deactivate when quest is activated or deactivated*/
    UPROPERTY(EditAnywhere, Category = "Quest")
    TMap<int, FQuestActorReferences> QuestReferences;
protected:
    void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
