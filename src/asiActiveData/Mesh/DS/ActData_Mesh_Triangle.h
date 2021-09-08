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

#ifndef ActData_Mesh_Triangle_HeaderFile
#define ActData_Mesh_Triangle_HeaderFile

// Mesh includes
#include <ActData_Mesh_Face.h>

DEFINE_STANDARD_HANDLE(ActData_Mesh_Triangle, ActData_Mesh_Face)

class ActData_Mesh_Triangle : public ActData_Mesh_Face
{
public:

  //! constructor for a triangle
  ActData_Mesh_Triangle(const Standard_Integer ID, const Standard_Integer idnode1, const Standard_Integer idnode2, const Standard_Integer idnode3);

  ActData_EXPORT Standard_Integer GetKey() const Standard_OVERRIDE;

  //! returns the idnodes of the ith edge (rank) of the face
  //! rank must be comprised between 1 and myNbConnections included.
    void GetEdgeDefinedByNodes (const Standard_Integer rank, Standard_Integer& idnode1, Standard_Integer& idnode2) const Standard_OVERRIDE;

    Standard_Address GetConnections() const Standard_OVERRIDE;

    Standard_Integer GetConnection (const Standard_Integer rank) const Standard_OVERRIDE;

  //! creates a copy of me
    virtual Handle(ActData_Mesh_Element) Copy (const Standard_Integer theID) const Standard_OVERRIDE;




  DEFINE_STANDARD_RTTI_INLINE(ActData_Mesh_Triangle,ActData_Mesh_Face)

private:

  ActData_EXPORT void SetConnections (const Standard_Integer idnode1, const Standard_Integer idnode2, const Standard_Integer idnode3);

  Standard_Integer myNodes[3];

};


//=======================================================================
//function : ActData_Mesh_Triangle
//purpose  : 
//=======================================================================

inline ActData_Mesh_Triangle::ActData_Mesh_Triangle(const Standard_Integer ID,
                                              const Standard_Integer idnode1,
                                              const Standard_Integer idnode2,
                                              const Standard_Integer idnode3)
  : ActData_Mesh_Face (ID, 3)
{
  SetConnections(idnode1,idnode2,idnode3);
}

//=======================================================================
//function : GetConnections
//purpose  :
//=======================================================================

inline Standard_Address ActData_Mesh_Triangle::GetConnections() const
{
  return (Standard_Address)&myNodes;
}

//=======================================================================
//function : GetConnection
//purpose  :
//=======================================================================

inline Standard_Integer ActData_Mesh_Triangle::GetConnection(const Standard_Integer rank) const
{
  return myNodes[rank-1];
}

//=======================================================================
//function : GetEdgeDefinedByNodes
//purpose  : 
//=======================================================================

inline void ActData_Mesh_Triangle::GetEdgeDefinedByNodes(const Standard_Integer rank,
                                                      Standard_Integer& idnode1,
                                                      Standard_Integer& idnode2) const
{
  idnode1 = myNodes[rank-1];
  idnode2 = myNodes[rank % myNbNodes];
}

//=======================================================================
//function : Copy
//purpose  : creates a copy of me
//=======================================================================

inline Handle(ActData_Mesh_Element) ActData_Mesh_Triangle::Copy(const Standard_Integer theID) const
{
  return new ActData_Mesh_Triangle(theID, myNodes[0], myNodes[1], myNodes[2]);
}


#endif // _ActData_MeshTriangle_HeaderFile
