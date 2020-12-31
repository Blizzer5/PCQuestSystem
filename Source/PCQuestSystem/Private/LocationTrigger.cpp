// Fill out your copyright notice in the Description page of Project Settings.


#include "LocationTrigger.h"
#include "Engine/TriggerBox.h"
#include "Engine/TriggerSphere.h"
#include "Engine/TriggerCapsule.h"
#include "Components/PrimitiveComponent.h"
#include "Components/ShapeComponent.h"


ALocationTrigger::ALocationTrigger()
{
    GetCollisionComponent()->SetCollisionObjectType(ECC_GameTraceChannel3);

    IconMarkerComponent = CreateDefaultSubobject<UIconMarkerComponent>(TEXT("IconMarkerComponent"));
}

void ALocationTrigger::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);

    if (UQuestManager* QuestManager = Cast<UQuestManager>(OtherActor->GetComponentByClass(UQuestManager::StaticClass())))
    {
        QuestManager->OnArrivedToPlace(Location);
    }
}

void ALocationTrigger::NotifyActorEndOverlap(AActor* OtherActor)
{
    Super::NotifyActorEndOverlap(OtherActor);

    if (UQuestManager* QuestManager = Cast<UQuestManager>(OtherActor->GetComponentByClass(UQuestManager::StaticClass())))
    {
        QuestManager->OnLeftPlace(Location);
    }
}

EPlaces ALocationTrigger::GetLocation()
{
    return Location;
}
