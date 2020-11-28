// Fill out your copyright notice in the Description page of Project Settings.


#include "IconMarkerComponent.h"
#include "IconMarkerUMG.h"
#include "UObject/UObjectGlobals.h"

// Sets default values for this component's properties
UIconMarkerComponent::UIconMarkerComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = false;

    static ConstructorHelpers::FObjectFinder<UBlueprint> IconMarkerObjectClass(TEXT("WidgetBlueprint'/PCQuestSystem/UI/UMG_IconMarker'"));

    if (IconMarkerObjectClass.Object)
    {
        MarkerUMGClass = (UClass*)IconMarkerObjectClass.Object->GeneratedClass;
    }
}


void UIconMarkerComponent::ActivateMarker()
{
    if (!MarkerUMG)
    {
        MarkerUMG = CreateWidget<UIconMarkerUMG>(GetWorld(), MarkerUMGClass.Get());
    }

    bIsActive = true;
    MarkerUMG->SetMarkerOwner(GetOwner());
    MarkerUMG->AddToViewport();
    MarkerUMG->PlayWidgetFadeAnimation();
}

void UIconMarkerComponent::DeactivateMarker()
{
    bIsActive = false;
    if (MarkerUMG)
    {
        MarkerUMG->RemoveFromViewport();
    }
}

