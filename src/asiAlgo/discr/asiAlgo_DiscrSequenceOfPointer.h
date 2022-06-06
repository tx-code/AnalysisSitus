//-----------------------------------------------------------------------------
// Created on: 15 April 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Quaoar Studio LLC
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
//    * Neither the name of the copyright holder(s) nor the
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
//-----------------------------------------------------------------------------

#ifndef asiAlgo_DiscrSequenceOfPointer_HeaderFile
#define asiAlgo_DiscrSequenceOfPointer_HeaderFile

// asiAlgo includes
#include <asiAlgo.h>

// OpenCascade includes
#include <NCollection_Sequence.hxx>
#include <NCollection_List.hxx>
#include <NCollection_IndexedMap.hxx>

//-----------------------------------------------------------------------------

namespace asiAlgo {
namespace discr {

class Edge;

typedef std::pair<Edge*, bool> PairOfPEdgeBoolean;

//! Computes a hash code for the given pair of the pointer to an edge and a boolean value of the face boundary,
//! in the range [1, upper].
//! \param val   the given pair of the edge pointer and a Boolean value.
//! \param upper the upper bound of the range for the hash code to compute.
//! \return the computed hash code, in the range [1, upper].
inline int HashCode(const PairOfPEdgeBoolean& val,
                    const int                 upper)
{
  return ::HashCode(val.first, upper);
}

typedef NCollection_Sequence<void*>                SequenceOfPointer;
typedef NCollection_List<void*>                    ListOfPointer;
typedef NCollection_IndexedMap<PairOfPEdgeBoolean> IMapOfPointerBoolean;

}
}

#endif