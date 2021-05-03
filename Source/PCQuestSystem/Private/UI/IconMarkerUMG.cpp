// Copyright © Pedro Costa, 2021. All rights reserved

#include <UI/IconMarkerUMG.h>
#include "Components/Widget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include <PCQSBlueprintFunctionLibrary.h>


void UIconMarkerUMG::GetIconLocationRotationAndDistance(FVector2D& OutScreenPosition, float& OutRotationAngleDegrees, float& DistanceToActor, bool& bIsOnScreen)
{
    UPCQSBlueprintFunctionLibrary::GetActorInformationToPlayerController(UGameplayStatics::GetPlayerController(this, 0), MarkerOwner, true, MarkerOffset, OutScreenPosition, OutRotationAngleDegrees, DistanceToActor, bIsOnScreen, PercentageEdge);
}

void UIconMarkerUMG::UpdateIcon()
{
    FVector2D WidgetPosition;
    float WidgetRotationAngle;
    bool bIsOnScreen;
    float distanceToActor;

    GetIconLocationRotationAndDistance(WidgetPosition, WidgetRotationAngle, distanceToActor, bIsOnScreen);
    SetPositionInViewport(WidgetPosition);
    MarkerDirection->SetRenderTransformAngle(WidgetRotationAngle);
    float metersDistance = distanceToActor * 0.01f;
    DistanceText->SetText(FText::AsNumber((int)(metersDistance)));
    MarkerDirection->SetVisibility(bIsOnScreen ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
    DistanceText->SetVisibility(bIsOnScreen && distanceToActor > 5 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

    float CurrentMetersDistance = currentDistanceToActor * 0.01f;
    bool bShouldFadeDistance = metersDistance < DistanceToFade || !bIsOnScreen;
    if (bIsDistanceFaded != bShouldFadeDistance)
    {
        PlayAnimation(DistanceFadeAnimation, 0.f, 1, bShouldFadeDistance ? EUMGSequencePlayMode::Forward : EUMGSequencePlayMode::Reverse);
    }

    bIsDistanceFaded = bShouldFadeDistance;
    currentDistanceToActor = distanceToActor;
}

void UIconMarkerUMG::SetMarkerOwner(AActor* newMarkerOwner)
{
    MarkerOwner = newMarkerOwner;
}

void UIconMarkerUMG::SetEdgePercentage(float newPercentage)
{
    PercentageEdge = newPercentage;
}

void UIconMarkerUMG::PlayWidgetFadeAnimation()
{
    PlayAnimation(WidgetFadeAnimation, 0,1, EUMGSequencePlayMode::Reverse);
}

void UIconMarkerUMG::SetMarkerIconImage(UTexture2D* IconToUse)
{
    IconMarker->SetBrushFromTexture(IconToUse);
}

void UIconMarkerUMG::SetMarkerOffset(FVector Offset)
{
    MarkerOffset = Offset;
}

void UIconMarkerUMG::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    UpdateIcon();
}
