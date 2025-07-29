// Copyright � Pedro Costa, 2021. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Actors/QuestManager.h"
#include "QuestComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnteredLocation, FGameplayTag, LocationEntered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLeftLocation, FGameplayTag, LocationLeft);

/**
 *
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class PCQUESTSYSTEM_API UQuestComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuestComponent();
    ~UQuestComponent();

    UFUNCTION(BlueprintPure)
    static UQuestComponent* GetQuestComponent(const AActor* Pawn);
    
    UPROPERTY(BlueprintAssignable, Category = "QuestManager")
        FOnEnteredLocation OnEnteredLocation;
    UPROPERTY(BlueprintAssignable, Category = "QuestManager")
        FOnLeftLocation OnLeftLocation;

    UFUNCTION(BlueprintCallable, Category = "QuestManager")
        void OnArrivedToPlace(FGameplayTag ArrivedPlace);
    UFUNCTION(BlueprintCallable, Category = "QuestManager")
        void OnLeftPlace(FGameplayTag PlaceLeft);
    UFUNCTION(BlueprintCallable, Category = "QuestManager")
        void OnEntityTalkedTo(FGameplayTag TalkedEntity);
    UFUNCTION(BlueprintCallable, Category = "QuestManager")
        void OnEntityKilled(FGameplayTag EntityKilled);
    UFUNCTION(BlueprintCallable, Category = "QuestManager")
        void OnItemGathered(FGameplayTag ItemGathered, float amountGathered);

	UFUNCTION(Exec, BlueprintCallable)
		void ActivateQuestDebug(int QuestID);
private:
	AQuestManager* QuestManager;
};
