// *************************************************************************
//
//    Modification of file paralution_PCG.H is made by the PARALUTION team,
//    this file is a part of the PARALUTION library
//                                          
//    PARALUTION   www.paralution.com
//
//    Copyright (C) 2012-2013 Dimitar Lukarski
//
//    GPLv3 
//
// *************************************************************************
//
// The origin OpenFOAM license (file PCG.H):
//
/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#ifndef PARALUTION_PCG_H
#define PARALUTION_PCG_H

#include "lduMatrix.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class paralution_PCG Declaration
\*---------------------------------------------------------------------------*/

class paralution_PCG
:
    public lduMatrix::solver
{
    // Private Member Functions

        //- Disallow default bitwise copy construct
        paralution_PCG(const paralution_PCG&);

        //- Disallow default bitwise assignment
        void operator=(const paralution_PCG&);



public:

    //- Runtime type information
    TypeName("paralution_PCG");


    // Constructors

        //- Construct from matrix components and solver controls
        paralution_PCG
        (
            const word& fieldName,
            const lduMatrix& matrix,
            const FieldField<Field, scalar>& interfaceBouCoeffs,
            const FieldField<Field, scalar>& interfaceIntCoeffs,
            const lduInterfaceFieldPtrsList& interfaces,
            const dictionary& solverControls
        );


    //- Destructor
    virtual ~paralution_PCG()
    {}


    // Member Functions

        //- Solve the matrix with this solver
        virtual solverPerformance solve
        (
            scalarField& psi,
            const scalarField& source,
            const direction cmpt=0
        ) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
