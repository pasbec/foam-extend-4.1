/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright held by original author
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

\*---------------------------------------------------------------------------*/

#include "surfaceFormatsCore.H"

#include "Time.H"
#include "IFstream.H"
#include "OFstream.H"
#include "SortableList.H"
#include "surfMesh.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

Foam::word Foam::fileFormats::surfaceFormatsCore::nativeExt("ofs");

// * * * * * * * * * * * * * Static Member Functions * * * * * * * * * * * * //

Foam::string Foam::fileFormats::surfaceFormatsCore::getLineNoComment
(
    IFstream& is
)
{
    string line;
    do
    {
        is.getLine(line);
    }
    while ((line.empty() || line[0] == '#') && is.good());

    return line;
}


#if 0
Foam::fileName Foam::fileFormats::surfaceFormatsCore::localMeshFileName
(
    const word& surfName
)
{
    const word name(surfName.size() ? surfName : surfaceRegistry::defaultName);

    return fileName
    (
        surfaceRegistry::prefix/name/surfMesh::meshSubDir
      / name + "." + nativeExt
    );
}


Foam::fileName Foam::fileFormats::surfaceFormatsCore::findMeshInstance
(
    const Time& t,
    const word& surfName
)
{
    fileName localName = localMeshFileName(surfName);

    // Search back through the time directories list to find the time
    // closest to and lower than current time

    instantList ts = t.times();
    label instanceI;

    for (instanceI = ts.size()-1; instanceI >= 0; --instanceI)
    {
        if (ts[instanceI].value() <= t.timeOutputValue())
        {
            break;
        }
    }

    // Noting that the current directory has already been searched
    // for mesh data, start searching from the previously stored time directory

    if (instanceI >= 0)
    {
        for (label i = instanceI; i >= 0; --i)
        {
            if (isFile(t.path()/ts[i].name()/localName))
            {
                return ts[i].name();
            }
        }
    }

    return "constant";
}


Foam::fileName Foam::fileFormats::surfaceFormatsCore::findMeshFile
(
    const Time& t,
    const word& surfName
)
{
    fileName localName = localMeshFileName(surfName);

    // Search back through the time directories list to find the time
    // closest to and lower than current time

    instantList ts = t.times();
    label instanceI;

    for (instanceI = ts.size()-1; instanceI >= 0; --instanceI)
    {
        if (ts[instanceI].value() <= t.timeOutputValue())
        {
            break;
        }
    }

    // Noting that the current directory has already been searched
    // for mesh data, start searching from the previously stored time directory

    if (instanceI >= 0)
    {
        for (label i = instanceI; i >= 0; --i)
        {
            fileName testName(t.path()/ts[i].name()/localName);

            if (isFile(testName))
            {
                return testName;
            }
        }
    }

    // fallback to "constant"
    return t.path()/"constant"/localName;
}
#endif


bool Foam::fileFormats::surfaceFormatsCore::checkSupport
(
    const wordHashSet& available,
    const word& ext,
    const bool verbose,
    const word& functionName
)
{
    if (available.found(ext))
    {
        return true;
    }
    else if (verbose)
    {
        wordList toc = available.toc();
        SortableList<word> known(toc.xfer());

        Info<<"Unknown file extension for " << functionName
            << " : " << ext << nl
            <<"Valid types: (";
        // compact output:
        forAll(known, i)
        {
            Info<<" " << known[i];
        }
        Info<<" )" << endl;
    }

    return false;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::fileFormats::surfaceFormatsCore::surfaceFormatsCore()
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::fileFormats::surfaceFormatsCore::~surfaceFormatsCore()
{}


// ************************************************************************* //