/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | foam-extend: Open Source CFD
   \\    /   O peration     | Version:     4.0
    \\  /    A nd           | Web:         http://www.foam-extend.org
     \\/     M anipulation  | For copyright notice see file Copyright
-------------------------------------------------------------------------------
License
    This file is part of foam-extend.

    foam-extend is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the License, or (at your
    option) any later version.

    foam-extend is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with foam-extend.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "dynamicPolyRefinementFvMesh.H"
#include "addToRunTimeSelectionTable.H"
#include "refinementSelection.H"
#include "polyhedralRefinement.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{

defineTypeNameAndDebug(dynamicPolyRefinementFvMesh, 0);

addToRunTimeSelectionTable
(
    topoChangerFvMesh,
    dynamicPolyRefinementFvMesh,
    IOobject
);

}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void Foam::dynamicPolyRefinementFvMesh::readDict()
{
    // Read and check refinement interval
    refineInterval_ = readLabel(refinementDict_.lookup("refineInterval"));
    if (refineInterval_ < 1)
    {
        FatalErrorIn("dynamicPolyRefinementFvMesh::readDict()")
            << "Illegal refineInterval found: " << refineInterval_ << nl
            << "The refineInterval controls the refinement/unrefinement"
            << " trigerring within a certain time step and should be > 0"
            << exit(FatalError);
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::dynamicPolyRefinementFvMesh::dynamicPolyRefinementFvMesh
(
    const IOobject& io
)
:
    topoChangerFvMesh(io),
    refinementDict_
    (
        IOdictionary
        (
            IOobject
            (
                "dynamicMeshDict",
                time().constant(),
                *this,
                IOobject::MUST_READ,
                IOobject::NO_WRITE,
                false
            )
        ).subDict(typeName + "Coeffs")
    ),
    refineInterval_(readLabel(refinementDict_.lookup("refineInterval"))),
    curTimeIndex_(-1),

    refinementSelectionPtr_(refinementSelection::New(*this, refinementDict_))
{
    // Add the topology modifier engine
    Info<< "Adding polyhedralRefinement topology modifier" << endl;

    topoChanger_.setSize(1);
    topoChanger_.set
    (
        0,
        new polyhedralRefinement
        (
            "polyhedralRefinement",
            refinementDict_,
            0,
            topoChanger_
        )
    );

    // Write mesh and modifiers
    topoChanger_.writeOpt() = IOobject::AUTO_WRITE;
    topoChanger_.write();
    write();
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::dynamicPolyRefinementFvMesh::~dynamicPolyRefinementFvMesh()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool Foam::dynamicPolyRefinementFvMesh::update()
{
    // Re-read the data from dictionary for on-the-fly changes
    readDict();

    // Performing refinement/unrefinement when:
    // 1. We are at the first time step
    // 2. Time step is a multiplier of specified refineInterval
    // 3. Only once per time step
    if
    (
        time().timeIndex() > 0
     && time().timeIndex() % refineInterval_ == 0
     && curTimeIndex_ < time().timeIndex()
    )
    {
        // Update current time index to skip multiple topo changes per single
        // time step
        curTimeIndex_ = time().timeIndex();

        // Get reference to polyhedralRefinement polyMeshModifier
        polyhedralRefinement& polyRefModifier =
            refCast<polyhedralRefinement>(topoChanger_[0]);

        // Get refinement candidates from refinement selection algorithm. Note:
        // return type is Xfer<labelList> so there's no copying
        const labelList refCandidates
        (
            refinementSelectionPtr_->refinementCellCandidates()
        );

        // Set cells to refine. Note: polyhedralRefinement ensures that face and
        // point consistent refinement is performed
        polyRefModifier.setCellsToRefine(refCandidates);

        // Get unrefinement point candidates from refinement selection
        // algorithm. Note: return type is Xfer<labelList> so there's no copying
        const labelList unrefCandidates
        (
            refinementSelectionPtr_->unrefinementPointCandidates()
        );

        // Set split points to unrefine around.
        // Notes:
        // 1. polyhedralRefinement ensures that only a consistent set of split
        //    points is used for unrefinement
        // 2. Must be called after polyhedralRefinement::setCellsToRefine
        polyRefModifier.setSplitPointsToUnrefine(unrefCandidates);

        // Activate the polyhedral refinement engine if there are some cells to
        // refine or there are some split points to unrefine around
        bool enableTopoChange =
            !refCandidates.empty() || !unrefCandidates.empty();

        // Note: must enable topo change for all processors since face and point
        // consistent refinement must be ensured across coupled patches
        reduce(enableTopoChange, orOp<bool>());

        if (enableTopoChange)
        {
            polyRefModifier.enable();
        }
        else
        {
            polyRefModifier.disable();
        }

        // Perform refinement and unrefinement in one go
        autoPtr<mapPolyMesh> topoChangeMap = topoChanger_.changeMesh();

        // Output cell balance
        Info<< "Successfully performed polyhedral refinement. "
            << "Changed from "
            << returnReduce(topoChangeMap->nOldCells(), sumOp<label>())
            << " to "
            << returnReduce(topoChangeMap->cellMap().size(), sumOp<label>())
            << " cells." << endl;

        return topoChangeMap->morphing();
    }

    // No refinement/unrefinement at this time step. Return false
    return false;
}


// ************************************************************************* //
