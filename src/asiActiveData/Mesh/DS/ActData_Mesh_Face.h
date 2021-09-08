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

#ifndef _ActData_MeshFace_HeaderFile
#define _ActData_MeshFace_HeaderFile

// Mesh includes
#include <ActData_Mesh_Element.h>
#include <ActData_Mesh_ElementType.h>

class ActData_Mesh;

DEFINE_STANDARD_HANDLE(ActData_Mesh_Face, ActData_Mesh_Element)

class ActData_Mesh_Face : public ActData_Mesh_Element
{

public:


  //! Query the Mesh object holding this element
  ActData_Mesh* GetMesh() const Standard_OVERRIDE;
  
    virtual Standard_Integer NbEdges() const Standard_OVERRIDE;
  
    virtual Standard_Integer NbFaces() const Standard_OVERRIDE;
  
  //! returns the idnodes of this face. rank is ignored.
  ActData_EXPORT virtual void GetFaceDefinedByNodes (const Standard_Integer rank, const Standard_Address idnode, Standard_Integer& nb) const Standard_OVERRIDE;
  
  ActData_EXPORT virtual void Print (Standard_OStream& OS) const Standard_OVERRIDE;
  
  //! return the mesh element type
    ActData_Mesh_ElementType GetType() const Standard_OVERRIDE;


friend class ActData_Mesh;


  DEFINE_STANDARD_RTTI_INLINE(ActData_Mesh_Face,ActData_Mesh_Element)

protected:

  
    ActData_Mesh_Face(const Standard_Integer ID, const Standard_Integer NbConnections);



private:


  ActData_Mesh* myMesh;


};


//=======================================================================
//function : ActData_Mesh_Face
//purpose  : 
//=======================================================================

inline ActData_Mesh_Face::ActData_Mesh_Face (const Standard_Integer ID,
                                       const Standard_Integer nb)
  : ActData_Mesh_Element (ID, nb),
    myMesh            (0L)
{}

//=======================================================================
//function : GetMesh
//purpose  : 
//=======================================================================

inline ActData_Mesh * ActData_Mesh_Face::GetMesh () const
{
  return myMesh;
}

//=======================================================================
//function : NbEdges
//purpose  : 
//=======================================================================

inline Standard_Integer ActData_Mesh_Face::NbEdges() const
{
  return myNbNodes;
}

//=======================================================================
//function : NbFaces
//purpose  : 
//=======================================================================

inline Standard_Integer ActData_Mesh_Face::NbFaces() const
{
  return 1;
}

//=======================================================================
//function : GetType
//purpose  :
//=======================================================================

inline ActData_Mesh_ElementType ActData_Mesh_Face::GetType() const
{
  return ActData_Mesh_ET_Face;
}

#endif
