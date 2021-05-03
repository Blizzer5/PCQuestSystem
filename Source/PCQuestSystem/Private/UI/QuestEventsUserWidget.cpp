// Copyright © Pedro Costa, 2021. All rights reserved

#include <UI/QuestEventsUserWidget.h>
#include "Components/TextBlock.h"
#include "Kismet/KismetSystemLibrary.h"

void UQuestEventsUserWidget::SetQuestComponent(UQuestComponent* OwnerQuestComponent)
{
    if (GetOwningPlayer()->IsLocalController())
    {
        OwnerQuestComponentComponent = OwnerQuestComponent;
        if (OwnerQuestComponentComponent)
        {
            //OwnerQuestComponentComponent->OnQuestCompletedDelegate.AddUniqueDynamic(this, &UQuestEventsUserWidget::OnQuestCompleted);
            //OwnerQuestComponentComponent->OnQuestActivated.AddUniqueDynamic(this, &UQuestEventsUserWidget::OnQuestActivated);
        }
    }
}

void UQuestEventsUserWidget::OnQuestActivated_Implementation(FQuest ActivatedQuest)
{
    UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnQuestActivated")));
}
