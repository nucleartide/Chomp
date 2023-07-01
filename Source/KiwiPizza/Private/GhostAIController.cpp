#include "GhostAIController.h"
#include "GhostPawn.h"

void AGhostAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Get the pawn.
    auto Pawn = GetPawn();
    check(Pawn);

    // Cast the pawn.
    auto GhostPawn = Cast<AGhostPawn>(Pawn);
    check(GhostPawn);

    // Call MoveVector(), and move by 100 cm/s to the left.
    FVector2D DeltaLocation(-100.0f, 0.0f);
    GhostPawn->MoveVector(MovementSpeed * DeltaLocation * DeltaTime);
}
