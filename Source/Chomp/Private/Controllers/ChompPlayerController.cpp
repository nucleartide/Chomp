#include "Controllers/ChompPlayerController.h"
#include "Pawns/ChompPawn.h"
#include "Utils/Debug.h"
#include "GenericPlatform/GenericPlatformMath.h"

AChompPlayerController::AChompPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AChompPlayerController::OnMoveVertical(float Delta)
{
    VerticalAxisDelta = Delta;
    if (Delta != 0)
        LastAxisToBecomeNonZero = Axis::Vertical;
}

void AChompPlayerController::OnMoveHorizontal(float Delta)
{
    HorizontalAxisDelta = Delta;
    if (Delta != 0)
        LastAxisToBecomeNonZero = Axis::Horizontal;
}

void AChompPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Check if pawn is alive.
    auto Pawn = GetPawn<AChompPawn>();
    if (!Pawn)
    {
        // Then pawn is most likely dead. Early return.
        return;
    }

    // Move the pawn using our MovementDirection.
    FGridLocation MovementDir;
    if (LastAxisToBecomeNonZero == Axis::Vertical) // This is needed because we can't move diagonally.
        MovementDir.X = FGenericPlatformMath::RoundToInt(VerticalAxisDelta);
    else if (LastAxisToBecomeNonZero == Axis::Horizontal)
        MovementDir.Y = FGenericPlatformMath::RoundToInt(HorizontalAxisDelta);
    Pawn->MoveTowards(MovementDir, DeltaTime);
}

void AChompPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Bind input axes.
    InputComponent->BindAxis("Move Forward / Backward", this, &AChompPlayerController::OnMoveVertical);
    InputComponent->BindAxis("Move Right / Left", this, &AChompPlayerController::OnMoveHorizontal);

    // Disable this. We want to manage the active camera ourselves.
    bAutoManageActiveCameraTarget = false;
}
