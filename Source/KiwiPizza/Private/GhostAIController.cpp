#include "GhostAIController.h"
#include "GhostPawn.h"
#include "Math/UnrealMathUtility.h"
#include "VectorTypes.h"
#include "Kismet/KismetMathLibrary.h"
#include "Debug.h"

void AGhostAIController::BeginPlay()
{
    Super::BeginPlay();

    if (IsTestOriginAndDestinationEnabled)
    {
        FVector2D Origin(0.0f, 0.0f);
        FVector2D Destination(100 * 10.5f, 0.0f);
        StartMovingFrom(Origin, Destination);
    }
}

void AGhostAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Get the pawn.
    // auto Pawn = GetPawn();
    // check(Pawn);

    // Cast the pawn.
    // auto GhostPawn = Cast<AGhostPawn>(Pawn);
    // check(GhostPawn);

    // Call MoveVector(), and move by 100 cm/s to the left.
    // FVector2D DeltaLocation(-100.0f, 0.0f);
    // GhostPawn->MoveVector(MovementSpeed * DeltaLocation * DeltaTime);

    // Move the Ghost, given the currently saved Source and Destination.
    Move(DeltaTime);
}

void AGhostAIController::StartMovingFrom(FVector2D Origin, FVector2D Destination)
{
    // Ensure Origin and Destination are axis-aligned.
    check((Origin.X == Destination.X) != (Origin.Y == Destination.Y));

    // Save the current Origin and Destination.
    CurrentOrigin = Origin;
    CurrentDestination = Destination;

    // Reset some internal bookkeeping.
    IsAtDestination = false;
    // ElapsedTime = 0.0f;

    // Compute the difference in units.
    // auto Difference = FMath::Abs(Origin.X - Destination.X) + FMath::Abs(Origin.Y - Destination.Y);

    // Multiply the number of units by the TimeToTraverseOneUnit to get the total time.
    // TotalTime = Difference * TimeToTraverseOneUnit;
}

void AGhostAIController::Move(float DeltaTime)
{
    if (IsAtDestination)
        return;

    // Compute the movement direction.
    FVector2D MovementDirection(CurrentDestination.X - CurrentOrigin.X, CurrentDestination.Y - CurrentOrigin.Y);

    // Magnify the movement direction by the movement speed and delta time.
    auto ScaledMovementDirection = MovementDirection.GetSafeNormal() * DeltaTime * MovementSpeed;

    // Grab a reference to the ghost pawn.
    auto GhostPawn = Cast<AGhostPawn>(GetPawn());
    check(GhostPawn);

    // Move the ghost pawn.
    GhostPawn->MoveVector(ScaledMovementDirection, DeltaTime);

    // Pawn has exceeded destination if...
    auto ActorLocation = GhostPawn->GetActorLocation();
    bool ExceededDestination = false;
    DEBUG_LOG(TEXT("%s"), *(ActorLocation.ToString()));
    if (MovementDirection.Y < 0)
        ExceededDestination = ActorLocation.Y <= CurrentDestination.Y;
    else if (MovementDirection.Y > 0)
        ExceededDestination = ActorLocation.Y >= CurrentDestination.Y;
    else if (MovementDirection.X < 0)
        ExceededDestination = ActorLocation.X <= CurrentDestination.X;
    else if (MovementDirection.X > 0)
        ExceededDestination = ActorLocation.X >= CurrentDestination.X;

    // If the pawn has exceeded the destination, update internal bookkeeping.
    if (ExceededDestination)
    {
        IsAtDestination = true;
    }

/*
    // Update the elapsed time.
    ElapsedTime += DeltaTime;

    // Clamp the ElapsedTime to 0 and TotalTime.
    ElapsedTime = FMath::Clamp(ElapsedTime, 0.0f, TotalTime);

    // Inverse lerp to get a lerp parameter.
    float LerpParam = UKismetMathLibrary::NormalizeToRange(ElapsedTime, 0.0f, TotalTime); // what happens if I don't include...

    // Lerp between CurrentOrigin and CurrentDestination, and get a new FVector2D.
    FVector Origin3D(CurrentOrigin.X, CurrentOrigin.Y, 0.0f);
    FVector Destination3D(CurrentDestination.X, CurrentDestination.Y, 0.0f);
    auto NewLocation = FMath::Lerp(Origin3D, Destination3D, LerpParam);
    // DEBUG_LOG(TEXT("%s"), *(NewLocation.ToString()));

    // Grab reference to GhostPawn.

    // Compute the delta required to move the ghost.
    auto CurrentLocation = GhostPawn->GetActorLocation();
    FVector2D Delta(NewLocation.X - CurrentLocation.X, NewLocation.Y - CurrentLocation.Y);
    DEBUG_LOG(TEXT("%s"), *(Delta.ToString()));
*/
}
