// Fill out your copyright notice in the Description page of Project Settings.


#include "IconMarkerUMG.h"
#include "Components/Widget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UIconMarkerUMG::GetIconLocationRotationAndDistance(const FVector InLocation, FVector2D& OutScreenPosition, float& OutRotationAngleDegrees, float& DistanceToActor, bool& bIsOnScreen)
{
    bIsOnScreen = false;
    OutRotationAngleDegrees = 0.f;
    FVector2D ScreenPosition = FVector2D();

    if (!GEngine)
    {
        return;
    }

    const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
    const FVector2D ViewportCenter = FVector2D(ViewportSize.X / 2, ViewportSize.Y / 2);

    UWorld* World = GEngine->GetWorldFromContextObject(this,EGetWorldErrorMode::LogAndReturnNull);

    if (!World)
    {
        return;
    }

    APlayerController* PlayerController = (this ? UGameplayStatics::GetPlayerController(this, 0) : NULL);

    if (!PlayerController)
    {
        return;
    }

    // This doesn't work if we want to have local player with more than one viewport
    DistanceToActor = FMath::Abs(FVector::Distance(InLocation, PlayerController->GetPawn()->GetActorLocation()));

    FVector CameraLoc;
    FRotator CameraRot;

    PlayerController->GetPlayerViewPoint(CameraLoc, CameraRot);

    const FVector CameraToLoc = InLocation - CameraLoc;
    FVector Forward = CameraRot.Vector();
    FVector Offset = CameraToLoc.GetSafeNormal();

    float DotProduct = FVector::DotProduct(Forward, Offset);
    bool bLocationIsBehindCamera = (DotProduct < 0);

    if (bLocationIsBehindCamera)
    {
        FVector Inverted = CameraToLoc * -1.f;
        FVector NewInLocation = CameraLoc + Inverted;

        PlayerController->ProjectWorldLocationToScreen(NewInLocation, ScreenPosition);

        ScreenPosition.X = ViewportSize.X - ScreenPosition.X;
        ScreenPosition.Y = ViewportSize.Y - ScreenPosition.Y;
    }
    else
    {
        PlayerController->ProjectWorldLocationToScreen(InLocation, ScreenPosition);
    }

    // Check to see if it's on screen. If it is, ProjectWorldLocationToScreen is all we need, return it.
    if (ScreenPosition.X >= 0.f && ScreenPosition.X <= ViewportSize.X
        && ScreenPosition.Y >= 0.f && ScreenPosition.Y <= ViewportSize.Y && !bLocationIsBehindCamera)
    {
        OutScreenPosition = ScreenPosition;
        bIsOnScreen = true;
        return;
    }

    ScreenPosition -= ViewportCenter;

    float AngleRadians = FMath::Atan2(ScreenPosition.Y, ScreenPosition.X);
    AngleRadians -= FMath::DegreesToRadians(90.f);

    OutRotationAngleDegrees = FMath::RadiansToDegrees(AngleRadians) + 180.f;

    float Cos = cosf(AngleRadians);
    float Sin = -sinf(AngleRadians);

    ScreenPosition = FVector2D(ViewportCenter.X + (Sin * 150.f), ViewportCenter.Y + Cos * 150.f);

    float m = Cos / Sin;

    FVector2D ScreenBounds = ViewportCenter * PercentageEdge;

    if (Cos > 0)
    {
        ScreenPosition = FVector2D(ScreenBounds.Y / m, ScreenBounds.Y);
    }
    else
    {
        ScreenPosition = FVector2D(-ScreenBounds.Y / m, -ScreenBounds.Y);
    }

    if (ScreenPosition.X > ScreenBounds.X)
    {
        ScreenPosition = FVector2D(ScreenBounds.X, ScreenBounds.X * m);
    }
    else if (ScreenPosition.X < -ScreenBounds.X)
    {
        ScreenPosition = FVector2D(-ScreenBounds.X, -ScreenBounds.X * m);
    }

    ScreenPosition += ViewportCenter;

    OutScreenPosition = ScreenPosition;
}

void UIconMarkerUMG::UpdateIcon()
{
    FVector2D WidgetPosition;
    float WidgetRotationAngle;
    bool bIsOnScreen;
    float distanceToActor;

    FVector Location = MarkerOwner->GetActorLocation();
    Location.Z += 120;


    GetIconLocationRotationAndDistance(Location, WidgetPosition, WidgetRotationAngle, distanceToActor, bIsOnScreen);
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

void UIconMarkerUMG::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    UpdateIcon();
}
