#include "PacmanPlayerController.h"
#include "PacmanPawn.h"
#include "Debug.h"

APacmanPlayerController::APacmanPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void APacmanPlayerController::OnMoveVertical(float Delta)
{
    VerticalAxisDelta = Delta;
}

void APacmanPlayerController::OnMoveHorizontal(float Delta)
{
    HorizontalAxisDelta = Delta;
}

void APacmanPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Compute delta movement.
    FVector2D MovementVector(HorizontalAxisDelta, VerticalAxisDelta); // Note that HorizontalAxisDelta / VerticalAxisDelta are either 0 or 1.
    auto Delta = this->MovementSpeed * MovementVector.GetSafeNormal() * DeltaTime;

    // Get pawn.
    auto Pawn = GetPawn();
    check(Pawn);

    // Cast to PacmanPawn.
    auto PacmanPawn = Cast<APacmanPawn>(Pawn);
    check(PacmanPawn);

    // Move the pawn using our computed delta.
    PacmanPawn->MoveVector(Delta);
}

void APacmanPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Bind input axes.
    InputComponent->BindAxis("Move Forward / Backward", this, &APacmanPlayerController::OnMoveVertical);
    InputComponent->BindAxis("Move Right / Left", this, &APacmanPlayerController::OnMoveHorizontal);

    // Disable this. We want to manage the active camera ourselves.
    bAutoManageActiveCameraTarget = false;
}
