// Copyright © Pedro Costa, 2021. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBase.h"
#include "Components/QuestComponent.h"
#include "Engine/TriggerBox.h"
#include <Components/IconMarkerComponent.h>
#include "LocationTrigger.generated.h"


/**
 *
 */
UCLASS()
class PCQUESTSYSTEM_API ALocationTrigger : public ATriggerBox
{
    GENERATED_BODY()

    ALocationTrigger();

    UPROPERTY(EditAnywhere, Category = "Location Trigger")
    UIconMarkerComponent* IconMarkerComponent;
public:
    void NotifyActorBeginOverlap(AActor* OtherActor) override;
    void NotifyActorEndOverlap(AActor* OtherActor) override;
    EPlaces GetLocation();
private:
    UPROPERTY(EditAnywhere)
    EPlaces Location;
};
