// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <UI/IconMarkerUMG.h>
#include <Components/IconMarkerComponent.h>
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "QuestManager.generated.h"

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
};

UENUM(BlueprintType)
enum class EPlaces : uint8
{
    None,
    Kanto,
    Johto,
};

UENUM(BlueprintType)
enum class EEntityType : uint8
{
    None,
    // Persons
    Adam,
    Innkeeper,
    // Enemies
    Dinossaur,
};

UENUM(BlueprintType)
enum class ERewardTypes : uint8
{
    None,
    XP,
    Gold,
    Skill,
};

UENUM(BlueprintType)
enum class EQuestItemTypes : uint8
{
    None,
    Arrow,
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
    bool bShowOnScreen;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective, meta = (EditCondition = "bCreateMarker == true"))
    bool bShowOnCompass;
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
    FQuestStepObjective(int QuestID, int StepObjectiveOrder, TMap<ERewardTypes, float> rewards, EQuestStepType StepType, FIconMarkerInformation MarkerInfo)
        : ParentQuestID(QuestID),
        StepObjectiveInsideQuestOrder(StepObjectiveOrder),
        QuestStepRewards(rewards),
        ObjectiveMarkerUMGInformation(MarkerInfo),
        QuestStepType(StepType)
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

    /** Quest Step XP to give */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
        TMap<ERewardTypes, float> QuestStepRewards;

    /** Quest Step Type */
    EQuestStepType QuestStepType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
        FIconMarkerInformation ObjectiveMarkerUMGInformation;

    /* Step description */
    FString Description;

    int ParentQuestID;

    FString SplitEnumString(FString EnumString);
    virtual FString GetStepDescription() { return Description; };
    virtual void SetDescription() {};
    /* Spawn/collect necessary actors */

    virtual void Activate(UWorld* WorldContext, AQuestManager* QuestManager) {};
    virtual void Deactivate() {
        for (AActor* AssociatedActor : ActorsAssociated)
        {
            if (UIconMarkerComponent* ActorMarkerComponent = AssociatedActor->FindComponentByClass<UIconMarkerComponent>())
            {
                ActorMarkerComponent->DeactivateMarker();
            }
        }
    };

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
    TArray<AActor*> ActorsAssociated;
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
    FQuestStepGoToObjective(int QuestID, int StepObjectiveOrder, TMap<ERewardTypes, float> rewards, FIconMarkerInformation MarkerInfo, EPlaces Place)
        :Super(QuestID, StepObjectiveOrder, rewards, EQuestStepType::GoTo, MarkerInfo),
        PlaceToGo(Place)
    {
        SetDescription();
    }

    /** Place that we want to go */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
        EPlaces PlaceToGo;

    void OnArrivedToPlace() { bIsCompleted = true; };

    void SetDescription() override;

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
    FQuestStepTalkWithObjective(int QuestID, int StepObjectiveOrder, TMap<ERewardTypes, float> rewards, FIconMarkerInformation MarkerInfo, TSubclassOf<AActor> PawnToSpawnClass, FVector SpawnWorldPosition, FRotator SpawnWorldRotation, EEntityType EntityToTalk)
        :Super(QuestID, StepObjectiveOrder, rewards, EQuestStepType::TalkWith, MarkerInfo),
        PawnToSpawnWhenActive(PawnToSpawnClass),
        WorldPositionToSpawn(SpawnWorldPosition),
        WorldRotationToSpawn(SpawnWorldRotation),
        EntityToTalkWith(EntityToTalk)
    {
        SetDescription();
    }

    /* Actor to spawn when this step is activated */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
        TSubclassOf<AActor> PawnToSpawnWhenActive;
    /* Where to spawn the actor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
        FVector WorldPositionToSpawn;
    /* Which rotation should the actor have */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
        FRotator WorldRotationToSpawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
        EEntityType EntityToTalkWith;

    void OnTalkedWithEntity() { bIsCompleted = true; }

    void SetDescription() override;

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
    FQuestStepKillObjective(int QuestID, int StepObjectiveOrder, TMap<ERewardTypes, float> rewards, FIconMarkerInformation MarkerInfo, FSpawnInformation SpawnInfo, EEntityType EntityKill, int KillAmount)
        :Super(QuestID, StepObjectiveOrder, rewards, EQuestStepType::Kill, MarkerInfo),
        SpawnInformation(SpawnInfo),
        EntityToKill(EntityKill),
        AmountToKill(KillAmount)
    {
        SetDescription();
    }

    /* Actor to spawn when this step is activated */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
        FSpawnInformation SpawnInformation;

    /** Who/what we want to kill */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
        EEntityType EntityToKill;

    /* How many we want to kill */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
        int AmountToKill;

    void OnEntityKilled() { bIsCompleted = ++CurrentlyKilled >= AmountToKill; }
private:
    int CurrentlyKilled = 0;
public:
    void SetDescription() override;


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
    FQuestStepGatherObjective(int QuestID, int StepObjectiveOrder, TMap<ERewardTypes, float> rewards, FIconMarkerInformation MarkerInfo, EQuestItemTypes ItemGather, int GaterAmount)
        :Super(QuestID, StepObjectiveOrder, rewards, EQuestStepType::Gather, MarkerInfo),
        ItemToGather(ItemGather),
        AmountToGather(GaterAmount)
    {
        SetDescription();
    }

    /** What we want to gather */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
        EQuestItemTypes ItemToGather;

    /* How many we want to gather */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
        float AmountToGather;

    void OnItemGathered(float amountGathered) { CurrentlyGathered += amountGathered; bIsCompleted = CurrentlyGathered >= AmountToGather; }
private:
    float CurrentlyGathered = 0;
public:
    void SetDescription() override;

};

USTRUCT(BlueprintType)
struct PCQUESTSYSTEM_API FQuest : public FTableRowBase
{
    GENERATED_BODY()


        /** Constructor */
        FQuest()
    {
    }

    FQuest(int QuestId, EQuestType Type, FString QuestName, TMap<ERewardTypes, float> rewards,
        TMap<int, FQuestStepGoToObjective> GoToObjectivesMap, TMap<int, FQuestStepTalkWithObjective> TalkWithObjectivesMap,
        TMap<int, FQuestStepKillObjective> KillObjectivesMap, TMap<int, FQuestStepGatherObjective> GatherObjectivesMap)
        : QuestID(QuestId),
        QuestType(Type),
        Name(QuestName),
        QuestRewards(rewards),
        GoToObjectives(GoToObjectivesMap),
        TalkWithObjectives(TalkWithObjectivesMap),
        KillObjectives(KillObjectivesMap),
        GatherObjectives(GatherObjectivesMap)
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
        FString Name;

    /** Quest XP to give */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Quest)
        TMap<ERewardTypes, float> QuestRewards;

    /* Quest Objectives */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Quest)
        TMap<int, FQuestStepGoToObjective> GoToObjectives;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Quest)
        TMap<int, FQuestStepTalkWithObjective> TalkWithObjectives;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Quest)
        TMap<int, FQuestStepKillObjective> KillObjectives;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Quest)
        TMap<int, FQuestStepGatherObjective> GatherObjectives;

    /* This is so we can use pointers and cast to our specific data type */
    TArray<TSharedPtr<FQuestStepObjective>> ObjectivesArray;

    void Init()
    {
        TArray<FQuestStepObjective> Objectives;
        for (auto& Elem : GoToObjectives)
        {
            ObjectivesArray.Emplace(MakeShareable(new FQuestStepGoToObjective(QuestID, Elem.Key, Elem.Value.QuestStepRewards, Elem.Value.ObjectiveMarkerUMGInformation, Elem.Value.PlaceToGo)));
        }

        for (auto& Elem : TalkWithObjectives)
        {
            ObjectivesArray.Emplace(MakeShareable(new FQuestStepTalkWithObjective(QuestID, Elem.Key, Elem.Value.QuestStepRewards, Elem.Value.ObjectiveMarkerUMGInformation, Elem.Value.PawnToSpawnWhenActive, Elem.Value.WorldPositionToSpawn, Elem.Value.WorldRotationToSpawn, Elem.Value.EntityToTalkWith)));
        }

        for (auto& Elem : KillObjectives)
        {
            ObjectivesArray.Emplace(MakeShareable(new FQuestStepKillObjective(QuestID, Elem.Key, Elem.Value.QuestStepRewards, Elem.Value.ObjectiveMarkerUMGInformation, Elem.Value.SpawnInformation, Elem.Value.EntityToKill, Elem.Value.AmountToKill)));
        }

        for (auto& Elem : GatherObjectives)
        {
            ObjectivesArray.Emplace(MakeShareable(new FQuestStepGatherObjective(QuestID, Elem.Key, Elem.Value.QuestStepRewards, Elem.Value.ObjectiveMarkerUMGInformation, Elem.Value.ItemToGather, Elem.Value.AmountToGather)));
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

    bool IsQuestCompleted()
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

    FQuestStepObjective GetCurrentObjective()
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

    TSharedPtr<FQuestStepObjective> GetCurrentObjectiveSharedPtr()
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
        void ActivateQuest(int QuestIDToActivate);
    UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "QuestManager")
        void OnAfterQuestActivated(int QuestIDToActivate);
    UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "QuestManager")
        void ResetQuest(int QuestIDToActivate);
    UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "QuestManager")
        void SetCurrentActiveQuest(int QuestIDToActivate);
    UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "QuestManager")
        void AddActiveQuest(int QuestIDToActivate);
    UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "QuestManager")
        void RemoveActiveQuest(int QuestIDToRemove);
    UFUNCTION(BlueprintPure, Category = "QuestManager")
        const FQuest GetCurrentActiveQuest();
    UFUNCTION(BlueprintPure, Category = "QuestManager")
        const TArray<FQuest> GetActiveQuests();
    UFUNCTION(BlueprintPure, Category = "QuestManager")
        const TArray<FQuest> GetAllQuests();

    UFUNCTION(Server, Reliable, Category = "QuestManager")
        void OnArrivedToPlace(EPlaces ArrivedPlace);
    UFUNCTION(NetMulticast, Reliable, Category = "QuestManager")
        void OnQuestStepArrivedToPlace(int StepID, int QuestID, EPlaces ArrivedPlace);
    UFUNCTION(Server, Reliable, Category = "QuestManager")
        void OnEntityTalkedTo(EEntityType TalkedEntity);
    UFUNCTION(NetMulticast, Reliable, Category = "QuestManager")
        void OnQuestStepEntityTalkedTo(int StepID, int QuestID, EEntityType TalkedEntity);
    UFUNCTION(Server, Reliable, Category = "QuestManager")
        void OnEntityKilled(EEntityType EntityKilled);
    UFUNCTION(NetMulticast, Reliable, Category = "QuestManager")
        void OnQuestStepEntityKilled(int StepID, int QuestID, EEntityType EntityKilled);
    UFUNCTION(Server, Reliable, Category = "QuestManager")
        void OnItemGathered(EQuestItemTypes ItemGathered, float amountGathered);
    UFUNCTION(NetMulticast, Reliable, Category = "QuestManager")
        void OnQuestStepItemGathered(int StepID, int QuestID, EQuestItemTypes ItemGathered, float amountGathered);

    UFUNCTION(Server, Reliable)
        void SpawnActor(TSubclassOf<AActor> ActorToSpawn, FVector WorldPositionToSpawn, FRotator WorldRotationToSpawn);
    UFUNCTION(NetMulticast, reliable)
        void AddAssociatedActorToQuestStepAfterTime(int StepQuestID, int QuestIDToGet, AActor* ActorToAdd);

    UFUNCTION(BlueprintPure, Category = "QuestManager")
        bool IsQuestCompleted(FQuest QuestToCheck);
    UFUNCTION(BlueprintPure, Category = "QuestManager")
        bool IsCurrentActiveQuest(FQuest QuestToCompare);
    UFUNCTION(BlueprintPure, Category = "QuestManager")
        bool IsCurrentQuestStepObjective(FQuestStepObjective QuestStepToCompare);

    UFUNCTION(BlueprintPure, Category = "QuestManager")
        const FQuestStepObjective GetCurrentQuestCurrentObjective();
    UFUNCTION(BlueprintPure, Category = "QuestManager")
        FString GetStepObjectiveDescription(FQuestStepObjective QuestStep);

    void AddAssociatedActorToQuestStep(int StepQuestID, int QuestIDToGet, AActor* ActorToAdd);
    AActor* GetLastSpawnedActor();
private:
    TSharedPtr<FQuest> GetQuestByID(int IDToGet);
    TSharedPtr<FQuestStepObjective> GetStepQuestByID(int IDToGet, TSharedPtr<FQuest> QuestToSearch);
    UFUNCTION(NetMulticast, reliable)
    void ActivateQuestObjectives(int CompletedQuestID);
    void OnQuestCompleted(TSharedPtr<FQuest> CompletedQuest);
    UFUNCTION(NetMulticast, reliable)
    void OnQuestCompletedNextTick(int CompletedQuestID);
    UFUNCTION(NetMulticast, reliable)
    void OnStepQuestCompleted(int CompletedStepQuestID, int QuestIDWhereStepBelongs);
    FQuestStepObjective GetCurrentQuestCurrentObjective() const;
private:
    TSharedPtr<FQuest> CurrentActiveQuest;
    TArray<TSharedPtr<FQuest>> ActiveQuests;
    TArray<TSharedPtr<FQuest>> AllQuests;
    AActor* LastSpawnedActor;
    /** Pointer to table where the quests come from */
    UPROPERTY(EditAnywhere, Category = DataTableRowHandle)
        const UDataTable* DataTable;
protected:
    void BeginPlay() override;

};
