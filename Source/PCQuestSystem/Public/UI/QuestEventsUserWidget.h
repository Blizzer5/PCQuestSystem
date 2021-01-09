// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuestComponent.h"
#include "QuestEventsUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class PCQUESTSYSTEM_API UQuestEventsUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable, Category = "QuestEventsUserWidget")
    void SetQuestComponent(UQuestComponent* OwnerQuestComponent);
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "QuestEventsUserWidget")
    void OnQuestActivated(FQuest ActivatedQuest);
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "QuestEventsUserWidget")
    void OnQuestCompleted(FQuest ActivatedQuest);

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
        UTextBlock* MissionNameText;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
        UTextBlock* QuestEventText;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
        UTextBlock* RewardsTitleText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetAnim))
        UWidgetAnimation* MissionEvent;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetAnim))
        UWidgetAnimation* ShowRewards;

private:
    UQuestComponent* OwnerQuestComponentComponent;
};
