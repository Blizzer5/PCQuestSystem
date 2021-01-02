// Fill out your copyright notice in the Description page of Project Settings.

#include <PCQSBlueprintFunctionLibrary.h>
#include "GameFramework/Actor.h"
#include "Kismet/KismetMathLibrary.h"
#include <Components/IconMarkerComponent.h>
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

TArray<UIconMarkerComponent*> UPCQSBlueprintFunctionLibrary::AllMarkerComponents = {};

void UPCQSBlueprintFunctionLibrary::GetActorInformationToPlayerController(APlayerController* PlayerController, AActor* ActorToCheck, FVector2D& OutScreenPosition, float& OutRotationAngleDegrees, float& DistanceToActor, bool& bIsOnScreen, float PercentageEdge /*= 1.0f*/)
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

    UWorld* World = GEngine->GetWorldFromContextObject(PlayerController, EGetWorldErrorMode::LogAndReturnNull);

    if (!World)
    {
        return;
    }

    if (!PlayerController)
    {
        return;
    }

    FVector InLocation = ActorToCheck->GetActorLocation();
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

TArray<UIconMarkerComponent*> UPCQSBlueprintFunctionLibrary::GetAllIconComponents()
{
    return AllMarkerComponents;
}

void UPCQSBlueprintFunctionLibrary::GetActorXPositionOnCompass(APlayerController* PlayerController, AActor* ActorToCheck, float margin, float& XPosition)
{
    XPosition = 0;
    FVector CameraLoc;
    FRotator CameraRot;

    PlayerController->GetPlayerViewPoint(CameraLoc, CameraRot);
    FRotator rotation = UKismetMathLibrary::FindLookAtRotation(ActorToCheck->GetActorLocation(), CameraLoc);
    FVector vector = UKismetMathLibrary::Conv_RotatorToVector(rotation);
    FVector2D normalized2D = FVector2D(vector.X, vector.Y);
    UKismetMathLibrary::Normalize2D(normalized2D);
    FVector2D RightVector2D = FVector2D(PlayerController->GetPawn()->GetActorRightVector().X, PlayerController->GetPawn()->GetActorRightVector().Y);
    FVector2D ForwardVector2D = FVector2D(PlayerController->GetPawn()->GetActorForwardVector().X, PlayerController->GetPawn()->GetActorForwardVector().Y);
    XPosition = UKismetMathLibrary::DotProduct2D(RightVector2D, normalized2D) / UKismetMathLibrary::DotProduct2D(ForwardVector2D, normalized2D) * margin;
}

AQuestManager* UPCQSBlueprintFunctionLibrary::GetWorldQuestManager(UObject* WorldContext)
{
    if (WorldContext && WorldContext->GetWorld())
    {
        TArray<AActor*> questManager;
        UGameplayStatics::GetAllActorsOfClass(WorldContext, AQuestManager::StaticClass(), questManager);
        if (questManager.Num() == 0)
        {
            UKismetSystemLibrary::PrintString(WorldContext, FString::Printf(TEXT("Tried to get Quest Manager but the world has none. Please put a QuestManager in the World.")));
            return nullptr;
        }
        return Cast<AQuestManager>(questManager[0]);
    }

    return nullptr;
}

void UPCQSBlueprintFunctionLibrary::AddIconMarkerComponent(UIconMarkerComponent* Component)
{
    AllMarkerComponents.Add(Component);
}

void UPCQSBlueprintFunctionLibrary::RemoveIconMarkerComponent(UIconMarkerComponent* Component)
{
    AllMarkerComponents.Remove(Component);
}