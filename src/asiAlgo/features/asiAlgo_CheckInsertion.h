//-----------------------------------------------------------------------------
// Created on: 16 June 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Sergey Slyadnev
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

#ifndef asiAlgo_CheckInsertion_h
#define asiAlgo_CheckInsertion_h

// asiALgo includes
#include <asiAlgo_InsertionType.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

// OCCT includes
#include <TopoDS_Face.hxx>

//-----------------------------------------------------------------------------

//! \ingroup ASI_AFR
//!
//! Checks if a set of feature faces is isolated in another face set.
class asiAlgo_CheckInsertion : public ActAPI_IAlgorithm
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_CheckInsertion, ActAPI_IAlgorithm)

public:

  //! Constructor.
  //! \param[in] progress Progress Entry.
  //! \param[in] plotter  Imperative Plotter.
  asiAlgo_EXPORT
    asiAlgo_CheckInsertion(ActAPI_ProgressEntry progress,
                           ActAPI_PlotterEntry  plotter);

public:

  //! Performs insertion analysis.
  //! \param[in] featureFace feature face.
  //! \param[in] masterFace  master face.
  //! \return insertion state.
  asiAlgo_EXPORT asiAlgo_InsertionType
    Check(const TopoDS_Face& featureFace,
          const TopoDS_Face& masterFace) const;

};

#endif
