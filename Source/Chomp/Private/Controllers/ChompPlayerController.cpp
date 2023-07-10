#include "Controllers/ChompPlayerController.h"
#include "Pawns/ChompPawn.h"
#include "Utils/Debug.h"

AChompPlayerController::AChompPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AChompPlayerController::OnMoveVertical(float Delta)
{
    VerticalAxisDelta = Delta;
}

void AChompPlayerController::OnMoveHorizontal(float Delta)
{
    HorizontalAxisDelta = Delta;
}

void AChompPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Compute delta movement.
    FVector2D MovementVector(VerticalAxisDelta, HorizontalAxisDelta); // Note that HorizontalAxisDelta / VerticalAxisDelta are either 0 or 1.
    auto Delta = this->MovementSpeed * MovementVector.GetSafeNormal() * DeltaTime;

    // Get pawn.
    auto Pawn = GetPawn();
    check(Pawn);

    // Cast to ChompPawn.
    auto ChompPawn = Cast<AChompPawn>(Pawn);
    check(ChompPawn);

    // Move the pawn using our computed delta.
    ChompPawn->MoveVector(Delta, DeltaTime);
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
