// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Templates/SharedPointer.h"
#include "Engine/DataTable.h"
#include "IconMarkerComponent.h"
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
    FQuestStepObjective(int StepObjectiveOrder, TMap<ERewardTypes, float> rewards, EQuestStepType StepType, TSubclassOf<UIconMarkerUMG> MarkerUMG)
        : StepObjectiveInsideQuestOrder(StepObjectiveOrder),
        QuestStepRewards(rewards),
        ObjectiveMarkerUMGClass(MarkerUMG),
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
    TSubclassOf<UIconMarkerUMG> ObjectiveMarkerUMGClass;

    /** Quest Step Type */
    EQuestStepType QuestStepType;

    /* Step description */
    FString Description;

    FString SplitEnumString(FString EnumString);
    virtual FString GetStepDescription() { return Description; };
    virtual void SetDescription() {};
    /* Spawn/collect necessary actors */
    virtual void Activate(UWorld* WorldContext) {};
    virtual void Deactivate() {
        if (ActorAssociated)
        {
            if (UIconMarkerComponent* ActorMarkerComponent = ActorAssociated->FindComponentByClass<UIconMarkerComponent>())
            {
                ActorMarkerComponent->DeactivateMarker();
            }
        }
    };

    void ResetStepQuest()
    {
        bIsCompleted = false;
    }

    virtual void ActivateActorMarker()
    {
        if (ActorAssociated)
        {
            if (UIconMarkerComponent* ActorMarkerComponent = ActorAssociated->FindComponentByClass<UIconMarkerComponent>())
            {
                ActorMarkerComponent->ActivateMarker();
            }
        }
    };
    bool IsCompleted() { return bIsCompleted; };

    void AddIconMarkerToAssociatedActor();


    bool IsValid()
    {
        return StepObjectiveInsideQuestOrder > 0 && QuestStepType != EQuestStepType::None;
    }
protected:
    bool bIsCompleted;
    AActor* ActorAssociated = nullptr;
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
    FQuestStepGoToObjective(int StepObjectiveOrder, TMap<ERewardTypes, float> rewards, TSubclassOf<UIconMarkerUMG> MarkerUMG, EPlaces Place)
        :Super(StepObjectiveOrder, rewards, EQuestStepType::GoTo, MarkerUMG),
        PlaceToGo(Place)
    {
        SetDescription();
    }

    /** Place that we want to go */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuestStepObjective)
        EPlaces PlaceToGo;

    void OnArrivedToPlace() { bIsCompleted = true; };

    void SetDescription() override;


    void Activate(UWorld* WorldContext) override;

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
    FQuestStepTalkWithObjective(int StepObjectiveOrder, TMap<ERewardTypes, float> rewards, TSubclassOf<UIconMarkerUMG> MarkerUMG, TSubclassOf<AActor> PawnToSpawnClass, FVector SpawnWorldPosition, FRotator SpawnWorldRotation, EEntityType EntityToTalk)
        :Super(StepObjectiveOrder, rewards, EQuestStepType::TalkWith, MarkerUMG),
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


    void Activate(UWorld* WorldContext) override;

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
    FQuestStepKillObjective(int StepObjectiveOrder, TMap<ERewardTypes, float> rewards, TSubclassOf<UIconMarkerUMG> MarkerUMG, EEntityType EntityKill, int KillAmount)
        :Super(StepObjectiveOrder, rewards, EQuestStepType::Kill, MarkerUMG),
        EntityToKill(EntityKill),
        AmountToKill(KillAmount)
    {
        SetDescription();
    }

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
    FQuestStepGatherObjective(int StepObjectiveOrder, TMap<ERewardTypes, float> rewards, TSubclassOf<UIconMarkerUMG> MarkerUMG, EQuestItemTypes ItemGather, int GaterAmount)
        :Super(StepObjectiveOrder, rewards, EQuestStepType::Gather, MarkerUMG),
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
            ObjectivesArray.Emplace(MakeShareable(new FQuestStepGoToObjective(Elem.Key, Elem.Value.QuestStepRewards, Elem.Value.ObjectiveMarkerUMGClass, Elem.Value.PlaceToGo)));
        }

        for (auto& Elem : TalkWithObjectives)
        {
            ObjectivesArray.Emplace(MakeShareable(new FQuestStepTalkWithObjective(Elem.Key, Elem.Value.QuestStepRewards, Elem.Value.ObjectiveMarkerUMGClass, Elem.Value.PawnToSpawnWhenActive, Elem.Value.WorldPositionToSpawn, Elem.Value.WorldRotationToSpawn, Elem.Value.EntityToTalkWith)));
        }

        for (auto& Elem : KillObjectives)
        {
            ObjectivesArray.Emplace(MakeShareable(new FQuestStepKillObjective(Elem.Key, Elem.Value.QuestStepRewards, Elem.Value.ObjectiveMarkerUMGClass, Elem.Value.EntityToKill, Elem.Value.AmountToKill)));
        }

        for (auto& Elem : GatherObjectives)
        {
            ObjectivesArray.Emplace(MakeShareable(new FQuestStepGatherObjective(Elem.Key, Elem.Value.QuestStepRewards, Elem.Value.ObjectiveMarkerUMGClass, Elem.Value.ItemToGather, Elem.Value.AmountToGather)));
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

    void ActivateCurrentObjective(UWorld* WordContext);
    void DeactivateObjective(TSharedPtr<FQuestStepObjective> ObjectiveToDeactivate);

    bool IsValid() const
    {
        return QuestID > -1;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestActivated, FQuest, ActivatedQuest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, FQuest, CompletedQuest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStepCompleted, FQuestStepObjective, CompletedStepQuest, FQuest, QuestWhereStepBelongs);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnteredLocation, EPlaces, LocationEntered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLeftLocation, EPlaces, LocationLeft);

/**
 *
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class PCQUESTSYSTEM_API UQuestManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuestManager();
    ~UQuestManager();

    void BeginPlay() override;

    UPROPERTY(BlueprintAssignable, Category = "QuestManager")
        FOnQuestActivated OnQuestActivated;
    UPROPERTY(BlueprintAssignable, Category = "QuestManager")
        FOnQuestCompleted OnQuestCompletedDelegate;
    UPROPERTY(BlueprintAssignable, Category = "QuestManager")
        FOnQuestStepCompleted OnQuestStepCompletedDelegate;
    UPROPERTY(BlueprintAssignable, Category = "QuestManager")
        FOnEnteredLocation OnEnteredLocation;
    UPROPERTY(BlueprintAssignable, Category = "QuestManager")
        FOnLeftLocation OnLeftLocation;

    UFUNCTION(BlueprintCallable, Category = "QuestManager")
        void OnArrivedToPlace(EPlaces ArrivedPlace);
    UFUNCTION(BlueprintCallable, Category = "QuestManager")
        void OnLeftPlace(EPlaces PlaceLeft);
    UFUNCTION(BlueprintCallable, Category = "QuestManager")
        void OnEntityTalkedTo(EEntityType TalkedEntity);
    UFUNCTION(BlueprintCallable, Category = "QuestManager")
        void OnEntityKilled(EEntityType EntityKilled);
    UFUNCTION(BlueprintCallable, Category = "QuestManager")
        void OnItemGathered(EQuestItemTypes ItemGathered, float amountGathered);
    UFUNCTION(BlueprintCallable, Category = "QuestManager")
        void ActivateQuest(int QuestIDToActivate);
    UFUNCTION(BlueprintCallable, Category = "QuestManager")
        void RemoveActiveQuest(int QuestIDToRemove);
    UFUNCTION(BlueprintPure, Category = "QuestManager")
        const FQuest GetCurrentActiveQuest();
    UFUNCTION(BlueprintPure, Category = "QuestManager")
        const TArray<FQuest> GetActiveQuests();
    UFUNCTION(BlueprintPure, Category = "QuestManager")
        const TArray<FQuest> GetAllQuests();

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

private:
    
    TSharedPtr<FQuest> GetQuestByID(int IDToGet);
    void ActivateQuestObjectives(TSharedPtr<FQuest> Quest);
    void OnQuestCompleted(TSharedPtr<FQuest> CompletedQuest);
    void OnQuestCompletedNextTick(TSharedPtr<FQuest> CompletedQuest);
    void OnStepQuestCompleted(TSharedPtr<FQuestStepObjective> CompletedStepQuest, TSharedPtr<FQuest> QuestWhereStepBelongs);
    FQuestStepObjective GetCurrentQuestCurrentObjective() const;
private:
    TSharedPtr<FQuest> CurrentActiveQuest;
    TArray<TSharedPtr<FQuest>> ActiveQuests;
    TArray<TSharedPtr<FQuest>> AllQuests;

    /** Pointer to table where the quests come from */
    UPROPERTY(EditAnywhere, Category = DataTableRowHandle)
        const UDataTable* DataTable;
};
