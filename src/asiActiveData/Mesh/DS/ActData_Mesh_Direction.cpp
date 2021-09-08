//-----------------------------------------------------------------------------
// Created on: June 2016
//-----------------------------------------------------------------------------
// Copyright (c) 2017, OPEN CASCADE SAS
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//    * Neither the name of OPEN CASCADE SAS nor the
//      names of all contributors may be used to endorse or promote products
//      derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Web: http://dev.opencascade.org
//-----------------------------------------------------------------------------

// Own include
#include <ActData_Mesh_Direction.h>

// OCCT includes
#include <gp_Dir.hxx>
#include <Precision.hxx>

inline Standard_Integer _REPACK (const Standard_Real aValue) {
  return Standard_Integer(aValue * Standard_Real(IntegerLast()-1) + 0.5);
}

inline Standard_Real _UNPACK (const Standard_Integer iValue) {
  return Standard_Real(iValue) / Standard_Real(IntegerLast() - 1);
}

//=======================================================================
//function : Set
//purpose  : 
//=======================================================================

void ActData_Mesh_Direction::Set (const Standard_Real theX,
                           const Standard_Real theY,
                           const Standard_Real theZ)
{
  const gp_Dir aDir (theX, theY, theZ);   // normalize
  myCoord[0] = _REPACK(aDir.X());
  myCoord[1] = _REPACK(aDir.Y());
  myCoord[2] = _REPACK(aDir.Z());
}

//=======================================================================
//function : Set
//purpose  : 
//=======================================================================

void ActData_Mesh_Direction::Set (const gp_XYZ& theXYZ)
{
  const gp_Dir aDir (theXYZ);   // normalize
  myCoord[0] = _REPACK(aDir.X());
  myCoord[1] = _REPACK(aDir.Y());
  myCoord[2] = _REPACK(aDir.Z());
}

//=======================================================================
//function : Get
//purpose  : 
//=======================================================================

Standard_Boolean ActData_Mesh_Direction::Get (gp_Dir& outDir) const
{
  if ((myCoord[0] | myCoord[1] | myCoord[2]) != 0) {
    outDir.SetCoord (_UNPACK(myCoord[0]),
                     _UNPACK(myCoord[1]),
                     _UNPACK(myCoord[2]));
    return Standard_True;
  }
  return Standard_False;
}
