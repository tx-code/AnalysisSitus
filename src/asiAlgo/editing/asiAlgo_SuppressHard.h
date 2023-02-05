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

#ifndef algoFeat_SuppressHard_h
#define algoFeat_SuppressHard_h

// asiAlgo includes
#include <asiAlgo_AAG.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

// OCCT includes
#include <Message_ProgressRange.hxx>

//! Utility to suppress hard features.
class asiAlgo_SuppressHard : public ActAPI_IAlgorithm
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_SuppressHard, ActAPI_IAlgorithm)

public:

  //! Constructor.
  //! \param[in] masterCAD full CAD model.
  //! \param[in] aag       Attributed Adjacency Graph.
  //! \param[in] progress  progress notifier.
  //! \param[in] plotter   imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_SuppressHard(const TopoDS_Shape&        masterCAD,
                         const Handle(asiAlgo_AAG)& aag,
                         ActAPI_ProgressEntry       progress,
                         ActAPI_PlotterEntry        plotter);

public:

  //! Removes the given feature faces from the master model.
  //! \param[in] faceIndices indices of faces to delete.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Perform(const asiAlgo_Feature& faceIndices);

public:

  //! \return result shape.
  const TopoDS_Shape& GetResult() const
  {
    return m_output;
  }

  //! \return modification history.
  const Handle(BRepTools_History)& GetHistory() const
  {
    return m_history;
  }

protected:

  //! History is prepared in Perform() method..
  virtual void buildHistory(Handle(BRepTools_History)&) {};

protected:

  TopoDS_Shape              m_input;   //!< Input shape.
  TopoDS_Shape              m_output;  //!< Output shape.
  Handle(asiAlgo_AAG)       m_aag;     //!< AAG.
  Handle(BRepTools_History) m_history; //!< Modification history.

};

#endif
