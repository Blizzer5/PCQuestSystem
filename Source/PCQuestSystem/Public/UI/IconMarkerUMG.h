// Copyright � Pedro Costa, 2021. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "IconMarkerUMG.generated.h"

class UImage;
class USizeBox;
class UGridPanel;
class UTextBlock;

/**
 * 
 */
UCLASS()
class PCQUESTSYSTEM_API UIconMarkerUMG : public UUserWidget
{
	GENERATED_BODY()
public:
    void GetIconLocationRotationAndDistance(FVector2D& OutScreenPosition, float& OutRotationAngleDegrees, float& DistanceToActor, bool& bIsOnScreen);
    void UpdateIcon();
    void SetMarkerOwner(AActor* newMarkerOwner);
    void SetEdgePercentage(float newPercentage);
    void PlayWidgetFadeAnimation();
    void SetMarkerIconImage(UTexture2D* IconToUse);
    void SetMarkerOffset(FVector Offset);
private:
    UPROPERTY(meta = (BindWidget))
    UImage* IconMarker;
    UPROPERTY(meta = (BindWidget))
    UImage* MarkerDirection;
    UPROPERTY(meta = (BindWidget))
    UTextBlock* DistanceText;

    UPROPERTY(meta = (BindWidgetAnim), Transient)
    UWidgetAnimation* WidgetFadeAnimation;
    UPROPERTY(meta = (BindWidgetAnim), Transient)
    UWidgetAnimation* DistanceFadeAnimation;

    AActor* MarkerOwner;

    UPROPERTY(EditAnywhere, Category = "IconMarkerUMG")
    float PercentageEdge = 0.8f;

    UPROPERTY(EditAnywhere, Category = "IconMarkerUMG")
    float DistanceToFade = 5.f;

    bool bIsDistanceFaded = false;
    float currentDistanceToActor = 0;

    FVector MarkerOffset;
protected:
    void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

};
