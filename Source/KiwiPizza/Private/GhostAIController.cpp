#include "GhostAIController.h"
#include "GhostPawn.h"
#include "Math/UnrealMathUtility.h"
#include "VectorTypes.h"
#include "Kismet/KismetMathLibrary.h"

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

    // Move the Ghost, given the currently saved Source and Destination.
    Move(DeltaTime);
}

void AGhostAIController::StartMovingFrom(FVector2D Origin, FVector2D Destination)
{
    // Ensure Origin and Destination are axis-aligned.
    check(Origin.X == Destination.X || Origin.Y == Destination.Y);

    // Save the current Origin and Destination.
    CurrentOrigin = Origin;
    CurrentDestination = Destination;

    // Reset some internal bookkeeping.
    IsAtDestination = false;
    ElapsedTime = 0.0f;

    // Compute the difference in units.
    auto Difference = FMath::Abs(Origin.X - Destination.X) + FMath::Abs(Origin.Y - Destination.Y);

    // Multiply the number of units by the TimeToTraverseOneUnit to get the total time.
    TotalTime = Difference * TimeToTraverseOneUnit;
}

void AGhostAIController::Move(float DeltaTime)
{
    if (IsAtDestination)
        return;

    // Update the elapsed time.
    ElapsedTime += DeltaTime;

    // Clamp the ElapsedTime to 0 and TotalTime.
    ElapsedTime = FMath::Clamp(ElapsedTime, 0.0f, TotalTime);

    // Inverse lerp to get a lerp parameter.
    float LerpParam = UKismetMathLibrary::NormalizeToRange(ElapsedTime, 0.0f, TotalTime); // what happens if I don't include...

    // Lerp between CurrentOrigin and CurrentDestination, and get a new FVector2D.
    FVector Origin3D(CurrentOrigin.X, CurrentOrigin.Y, 0.0f);
    FVector Destination3D(CurrentDestination.X, CurrentDestination.Y, 0.0f);
    // float NewLocation = FMath::Lerp(Origin3D, Destination3D, LerpParam);

    // Set the actor's location to the new FVector2D.
    // Use the MoveVector() provided by the Pawn class: MoveVector(newPosition - currentPosition)
    // ...

    // If the actor has reached the destination, update internal bookkeeping.
    // ...

    // In the Pawn class, the ghost's orientation should also be updated.
    // ...
}
