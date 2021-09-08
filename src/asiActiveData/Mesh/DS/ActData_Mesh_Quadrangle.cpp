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
#include <ActData_Mesh_Quadrangle.h>

//=======================================================================
//function : GetKey
//purpose  : compute the Key of the face based on the ids of its bounding nodes
//=======================================================================
Standard_Integer ActData_Mesh_Quadrangle::GetKey() const
{
  Standard_Integer aKey = myNodes[0];
  aKey += (aKey << 10);
  aKey ^= (aKey >> 6);
  aKey += (myNodes[1] + myNodes[3]);
  aKey += (aKey << 10);
  aKey ^= (aKey >> 6);
  aKey += myNodes[2];
  aKey += (aKey << 10);
  aKey ^= (aKey >> 6);
  aKey += (aKey << 3);
  aKey ^= (aKey >> 11);
//    return aKey + (aKey << 15);
  return aKey & 0x7fffffff;
}

//=======================================================================
//function : SetConnections
//purpose  : 
//=======================================================================

void ActData_Mesh_Quadrangle::SetConnections(const Standard_Integer idnode1, 
                                          const Standard_Integer idnode2,
                                          const Standard_Integer idnode3,
                                          const Standard_Integer idnode4)
{
  Standard_Integer idmin = (idnode1 < idnode2 ? idnode1 : idnode2);
  idmin = (idmin < idnode3 ? idmin : idnode3);
  idmin = (idmin < idnode4 ? idmin : idnode4);
  
  myNodes[0] = idmin;
  if (idmin == idnode1) {
    myNodes[1] = idnode2;
    myNodes[2] = idnode3;
    myNodes[3] = idnode4;
  } else if (idmin == idnode2) {
    myNodes[1] = idnode3;
    myNodes[2] = idnode4;
    myNodes[3] = idnode1;
  } else if (idmin == idnode3) {
    myNodes[1] = idnode4;
    myNodes[2] = idnode1;
    myNodes[3] = idnode2;
  } else {
    myNodes[1] = idnode1;
    myNodes[2] = idnode2;
    myNodes[3] = idnode3;
  }
}
