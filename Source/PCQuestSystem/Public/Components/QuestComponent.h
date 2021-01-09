// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Actors/QuestManager.h"
#include "QuestComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnteredLocation, EPlaces, LocationEntered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLeftLocation, EPlaces, LocationLeft);

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
};
