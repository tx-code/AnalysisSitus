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

#ifndef ActData_Mesh_SpacePosition_HeaderFile
#define ActData_Mesh_SpacePosition_HeaderFile

// Mesh includes
#include <ActData_Mesh_Position.h>
#include <ActData_Mesh_TypeOfPosition.h>

// OCCT includes
#include <gp_Pnt.hxx>

DEFINE_STANDARD_HANDLE(ActData_Mesh_SpacePosition, ActData_Mesh_Position)

//! used to characterize a MeshNode with a 3D point
//! in space not related to any underlying geometry (CAD)
class ActData_Mesh_SpacePosition : public ActData_Mesh_Position
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_Mesh_SpacePosition, ActData_Mesh_Position)

public:

  //! Default constructor
  ActData_Mesh_SpacePosition();

  ActData_Mesh_SpacePosition(const Standard_Real x, const Standard_Real y, const Standard_Real z);

  ActData_Mesh_SpacePosition(const gp_Pnt& aCoords);

  //! returns the resulting 3d point to be set in the MeshNode instance
  virtual const gp_Pnt& Coords() const Standard_OVERRIDE;

  //! returns the type of position
  virtual ActData_Mesh_TypeOfPosition GetTypeOfPosition() const Standard_OVERRIDE;

  void SetCoords (const Standard_Real x, const Standard_Real y, const Standard_Real z);

  void SetCoords (const gp_Pnt& aCoords);

private:

  gp_Pnt myCoords;

};


//=======================================================================
//function : ActData_Mesh_SpacePosition
//purpose  : 
//=======================================================================

inline ActData_Mesh_SpacePosition::ActData_Mesh_SpacePosition()
  : ActData_Mesh_Position(),myCoords(0.,0.,0.)
{
}

//=======================================================================
//function : ActData_Mesh_SpacePosition
//purpose  : 
//=======================================================================

inline ActData_Mesh_SpacePosition::ActData_Mesh_SpacePosition(const Standard_Real x,
                                                const Standard_Real y,
                                                const Standard_Real z)
  : ActData_Mesh_Position(),myCoords(x,y,z)
{
}

//=======================================================================
//function : ActData_Mesh_SpacePosition
//purpose  : 
//=======================================================================

inline ActData_Mesh_SpacePosition::ActData_Mesh_SpacePosition(const gp_Pnt& aCoords)
  : ActData_Mesh_Position(),myCoords(aCoords)
{
}

//=======================================================================
//function : Coords
//purpose  : 
//=======================================================================

inline const gp_Pnt& ActData_Mesh_SpacePosition::Coords() const
{
  return myCoords;
}

//=======================================================================
//function : GetTypeOfPosition
//purpose  : 
//=======================================================================

inline ActData_Mesh_TypeOfPosition ActData_Mesh_SpacePosition::GetTypeOfPosition() const
{
  return ActData_Mesh_TOP_3DSPACE;
}

//=======================================================================
//function : SetCoords
//purpose  : 
//=======================================================================

inline void ActData_Mesh_SpacePosition::SetCoords(const Standard_Real x,
                                           const Standard_Real y,
                                           const Standard_Real z)
{
  myCoords.SetCoord(x,y,z);
}

//=======================================================================
//function : SetCoords
//purpose  : 
//=======================================================================

inline void ActData_Mesh_SpacePosition::SetCoords(const gp_Pnt& aCoords)
{
  myCoords = aCoords;
}

#endif // _ActData_SpacePosition_HeaderFile
