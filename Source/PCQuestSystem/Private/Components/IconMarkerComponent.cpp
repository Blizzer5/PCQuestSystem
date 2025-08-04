// Copyright � Pedro Costa, 2021. All rights reserved

#include <Components/IconMarkerComponent.h>
#include "UObject/UObjectGlobals.h"
#include <PCQSBlueprintFunctionLibrary.h>
#include <UI/IconMarkerUMG.h>

// Sets default values for this component's properties
UIconMarkerComponent::UIconMarkerComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

UIconMarkerComponent::~UIconMarkerComponent()
{
    if (MarkerUMG.IsValid())
    {
        MarkerUMG->RemoveFromParent();
    }
    MarkerUMG = nullptr;

    UPCQSBlueprintFunctionLibrary::RemoveIconMarkerComponent(this);
}

void UIconMarkerComponent::BeginPlay()
{
    Super::BeginPlay();

    UPCQSBlueprintFunctionLibrary::AddIconMarkerComponent(this);
}

void UIconMarkerComponent::ActivateMarker()
{
    if (bShowOnScreen)
    {
        if (!MarkerUMG.IsValid())
        {
            MarkerUMG = CreateWidget<UIconMarkerUMG>(GetWorld(), MarkerUMGClass.Get());
            MarkerUMG->SetMarkerIconImage(MarkerIcon);
            MarkerUMG->SetMarkerOwner(GetOwner());
            MarkerUMG->SetMarkerOffset(ActorOffset);
        }

        bActive = true;
        MarkerUMG->AddToViewport();
        MarkerUMG->PlayWidgetFadeAnimation();
    }
}

void UIconMarkerComponent::DeactivateMarker()
{
    bActive = false;
    if (MarkerUMG.IsValid())
    {
        MarkerUMG->RemoveFromParent();
    }
}

bool UIconMarkerComponent::ShouldShowOnScreen()
{
    return bShowOnScreen && bActive;
}

bool UIconMarkerComponent::ShouldShowOnCompass()
{
    return bShowOnCompass && bActive;
}

UTexture2D* UIconMarkerComponent::GetMarkerIcon()
{
    return MarkerIcon;
}

bool UIconMarkerComponent::IsMarkerActive()
{
    return bActive;
}

void UIconMarkerComponent::ShowMarker() const
{
    if (MarkerUMG.IsValid())
    {
        MarkerUMG->AddToViewport();
    }
}

void UIconMarkerComponent::HideMarker() const
{
    if (MarkerUMG.IsValid())
    {
        MarkerUMG->RemoveFromParent();
    }
}

void UIconMarkerComponent::SetMarkerUMGToUse(TSubclassOf<UIconMarkerUMG> markerToUse)
{
    MarkerUMGClass = markerToUse;
}

