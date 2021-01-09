// Fill out your copyright notice in the Description page of Project Settings.

#include <Actors/LocationTrigger.h>
#include "Engine/TriggerBox.h"
#include "Engine/TriggerSphere.h"
#include "Engine/TriggerCapsule.h"
#include "Components/PrimitiveComponent.h"
#include "Components/ShapeComponent.h"
#include "QuestComponent.h"
#include <Components/IconMarkerComponent.h>


ALocationTrigger::ALocationTrigger()
{
    GetCollisionComponent()->SetCollisionObjectType(ECC_GameTraceChannel3);

    IconMarkerComponent = CreateDefaultSubobject<UIconMarkerComponent>(TEXT("IconMarkerComponent"));
}

void ALocationTrigger::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);

    if (UQuestComponent* QuestComponent = Cast<UQuestComponent>(OtherActor->GetComponentByClass(UQuestComponent::StaticClass())))
    {
        QuestComponent->OnArrivedToPlace(Location);
    }
}

void ALocationTrigger::NotifyActorEndOverlap(AActor* OtherActor)
{
    Super::NotifyActorEndOverlap(OtherActor);

    if (UQuestComponent* QuestComponent = Cast<UQuestComponent>(OtherActor->GetComponentByClass(UQuestComponent::StaticClass())))
    {
        QuestComponent->OnLeftPlace(Location);
    }
}

EPlaces ALocationTrigger::GetLocation()
{
    return Location;
}
