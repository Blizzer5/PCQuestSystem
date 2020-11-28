// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBase.h"
#include "QuestManager.h"
#include "Engine/TriggerBox.h"
#include "LocationTrigger.generated.h"


/**
 *
 */
UCLASS()
class PCQUESTSYSTEM_API ALocationTrigger : public ATriggerBox
{
    GENERATED_BODY()

    ALocationTrigger();
public:
    void NotifyActorBeginOverlap(AActor* OtherActor) override;
    void NotifyActorEndOverlap(AActor* OtherActor) override;
    EPlaces GetLocation();
private:
    UPROPERTY(EditAnywhere)
    EPlaces Location;
};
