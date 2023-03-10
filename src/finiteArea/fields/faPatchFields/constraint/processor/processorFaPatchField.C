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

#include "processorFaPatchField.H"
#include "processorFaPatch.H"
#include "IPstream.H"
#include "OPstream.H"
#include "demandDrivenData.H"
#include "transformField.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * Constructors * * * * * * * * * * * * * * //

template<class Type>
processorFaPatchField<Type>::processorFaPatchField
(
    const faPatch& p,
    const DimensionedField<Type, areaMesh>& iF
)
:
    coupledFaPatchField<Type>(p, iF),
    procPatch_(refCast<const processorFaPatch>(p))
{}


template<class Type>
processorFaPatchField<Type>::processorFaPatchField
(
    const faPatch& p,
    const DimensionedField<Type, areaMesh>& iF,
    const Field<Type>& f
)
:
    coupledFaPatchField<Type>(p, iF, f),
    procPatch_(refCast<const processorFaPatch>(p))
{}


// Construct by mapping given processorFaPatchField<Type>
template<class Type>
processorFaPatchField<Type>::processorFaPatchField
(
    const processorFaPatchField<Type>& ptf,
    const faPatch& p,
    const DimensionedField<Type, areaMesh>& iF,
    const faPatchFieldMapper& mapper
)
:
    coupledFaPatchField<Type>(ptf, p, iF, mapper),
    procPatch_(refCast<const processorFaPatch>(p))
{
    if (!isType<processorFaPatch>(this->patch()))
    {
        FatalErrorIn
        (
            "processorFaPatchField<Type>::processorFaPatchField\n"
            "(\n"
            "    const processorFaPatchField<Type>& ptf,\n"
            "    const faPatch& p,\n"
            "    const DimensionedField<Type, areaMesh>& iF,\n"
            "    const faPatchFieldMapper& mapper\n"
            ")\n"
        )   << "\n    patch type '" << p.type()
            << "' not constraint type '" << typeName << "'"
            << "\n    for patch " << p.name()
            << " of field " << this->dimensionedInternalField().name()
            << " in file " << this->dimensionedInternalField().objectPath()
            << exit(FatalIOError);
    }
}


template<class Type>
processorFaPatchField<Type>::processorFaPatchField
(
    const faPatch& p,
    const DimensionedField<Type, areaMesh>& iF,
    const dictionary& dict
)
:
    coupledFaPatchField<Type>(p, iF, dict),
    procPatch_(refCast<const processorFaPatch>(p))
{
    if (!isType<processorFaPatch>(p))
    {
        FatalIOErrorIn
        (
            "processorFaPatchField<Type>::processorFaPatchField\n"
            "(\n"
            "    const faPatch& p,\n"
            "    const Field<Type>& field,\n"
            "    const dictionary& dict\n"
            ")\n",
            dict
        )   << "\n    patch type '" << p.type()
            << "' not constraint type '" << typeName << "'"
            << "\n    for patch " << p.name()
            << " of field " << this->dimensionedInternalField().name()
            << " in file " << this->dimensionedInternalField().objectPath()
            << exit(FatalIOError);
    }
}


template<class Type>
processorFaPatchField<Type>::processorFaPatchField
(
    const processorFaPatchField<Type>& ptf
)
:
    processorLduInterfaceField(),
    coupledFaPatchField<Type>(ptf),
    procPatch_(refCast<const processorFaPatch>(ptf.patch()))
{}


template<class Type>
processorFaPatchField<Type>::processorFaPatchField
(
    const processorFaPatchField<Type>& ptf,
    const DimensionedField<Type, areaMesh>& iF
)
:
    coupledFaPatchField<Type>(ptf, iF),
    procPatch_(refCast<const processorFaPatch>(ptf.patch()))
{}


// * * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * //

template<class Type>
processorFaPatchField<Type>::~processorFaPatchField()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
tmp<Field<Type> > processorFaPatchField<Type>::patchNeighbourField() const
{
    return *this;
}


template<class Type>
void processorFaPatchField<Type>::initEvaluate
(
    const Pstream::commsTypes commsType
)
{
    if (Pstream::parRun())
    {
        procPatch_.send(commsType, this->patchInternalField()());
    }
}


template<class Type>
void processorFaPatchField<Type>::evaluate
(
    const Pstream::commsTypes commsType
)
{
    if (Pstream::parRun())
    {
        procPatch_.receive<Type>(commsType, *this);

        if (doTransform())
        {
            transform(*this, procPatch_.forwardT(), *this);
        }
    }
}


template<class Type>
tmp<Field<Type> > processorFaPatchField<Type>::snGrad() const
{
    return this->patch().deltaCoeffs()*(*this - this->patchInternalField());
}


template<class Type>
void processorFaPatchField<Type>::initInterfaceMatrixUpdate
(
    const scalarField& psiInternal,
    scalarField&,
    const lduMatrix&,
    const scalarField&,
    const direction,
    const Pstream::commsTypes commsType,
    const bool switchToLhs
) const
{
    procPatch_.send
    (
        commsType,
        this->patch().patchInternalField(psiInternal)()
    );
}


template<class Type>
void processorFaPatchField<Type>::updateInterfaceMatrix
(
    const scalarField&,
    scalarField& result,
    const lduMatrix&,
    const scalarField& coeffs,
    const direction cmpt,
    const Pstream::commsTypes commsType,
    const bool switchToLhs
) const
{
    scalarField pnf
    (
        procPatch_.receive<scalar>(commsType, this->size())()
    );

    // Transform according to the transformation tensor
    transformCoupleField(pnf, cmpt);

    // Multiply the field by coefficients and add into the result

    const unallocLabelList& edgeFaces = this->patch().edgeFaces();

    if (switchToLhs)
    {
        forAll(edgeFaces, elemI)
        {
            result[edgeFaces[elemI]] += coeffs[elemI]*pnf[elemI];
        }
    }
    else
    {
        forAll(edgeFaces, elemI)
        {
            result[edgeFaces[elemI]] -= coeffs[elemI]*pnf[elemI];
        }
    }
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
