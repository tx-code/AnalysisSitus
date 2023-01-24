//-----------------------------------------------------------------------------
// Created on: 25 January 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Elizaveta Krylova
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

#ifndef asiAlgo_BuildOBB_h
#define asiAlgo_BuildOBB_h

// asiAlgo includes
#include <asiAlgo_OBB.h>
#include <asiAlgo_AAG.h>
#include <asiAlgo_OrientCnc.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

// OCCT includes
#include <gp_Ax3.hxx>
#include <TopoDS_Solid.hxx>

//-----------------------------------------------------------------------------

//! Utility to build Oriented Bounding Box on part.
class asiAlgo_BuildOBB : public ActAPI_IAlgorithm
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_BuildOBB, ActAPI_IAlgorithm)

public:

  //! Constructor.
  //! \param[in] aag      the AAG of the input part.
  //! \param[in] progress the progress entry.
  //! \param[in] plotter  the imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_BuildOBB(const Handle(asiAlgo_AAG)& aag,
                     ActAPI_ProgressEntry       progress = nullptr,
                     ActAPI_PlotterEntry        plotter  = nullptr);

public:

  //! Builds OBB.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Perform();

public:

  //! \return resulting OBB.
  asiAlgo_EXPORT const asiAlgo_OBB&
    GetResult() const;

  //! \return resulting transformation matrix which defines the placement
  //!         of the oriented box.
  asiAlgo_EXPORT gp_Trsf
    GetResultTrsf() const;

  //! \return resulting OBB as a B-Rep box.
  asiAlgo_EXPORT TopoDS_Shape
    GetResultBox() const;

protected:

  Handle(asiAlgo_AAG) m_aag; //!< AAG of the input part.
  asiAlgo_OBB         m_obb; //!< Constructed OBB.

};

#endif
