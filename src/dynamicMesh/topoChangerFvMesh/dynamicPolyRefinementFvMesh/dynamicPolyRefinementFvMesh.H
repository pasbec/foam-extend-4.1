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

Class
    Foam::dynamicPolyRefinementFvMesh

Description
    Adaptive mesh refinement for arbitrary polyhedral cells.

SourceFiles
    dynamicPolyRefinementFvMesh.C

Author
    Vuko Vukcevic, Wikki Ltd.  All rights reserved

Notes
    Generalisation and refactorisation of dynamicRefineMesh for polyhedral cells

\*---------------------------------------------------------------------------*/

#ifndef dynamicPolyRefinementFvMesh_H
#define dynamicPolyRefinementFvMesh_H

#include "topoChangerFvMesh.H"
#include "refinementSelection.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                Class dynamicPolyRefinementFvMesh Declaration
\*---------------------------------------------------------------------------*/

class dynamicPolyRefinementFvMesh
:
    public topoChangerFvMesh
{
    // Private data

        //- Dictionary containing dynamic mesh coefficient controls
        dictionary refinementDict_;

        //- Refinement interval
        label refineInterval_;

        //- Current time index (helper variable to skip multiple topo changes in
        //  a single time step)
        label curTimeIndex_;

        //- Refinement selection algorithm that has two jobs:
        //  1. Selects cells to refine from all cells
        //  2. Selects split points that are ok to refine based on given set of
        //     all split points
        autoPtr<refinementSelection> refinementSelectionPtr_;


    // Private Member Functions

        //- Helper function for reading the dictionary and updating the data
        void readDict();


        // Copy control

            //- Disallow default bitwise copy construct
            dynamicPolyRefinementFvMesh(const dynamicPolyRefinementFvMesh&);

            //- Disallow default bitwise assignment
            void operator=(const dynamicPolyRefinementFvMesh&);

public:

    //- Runtime type information
    TypeName("dynamicPolyRefinementFvMesh");


    // Constructors

        //- Construct from IOobject
        explicit dynamicPolyRefinementFvMesh(const IOobject& io);


    //- Destructor
    virtual ~dynamicPolyRefinementFvMesh();


    // Member Functions

        //- Update the mesh for topology change
        virtual bool update();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
