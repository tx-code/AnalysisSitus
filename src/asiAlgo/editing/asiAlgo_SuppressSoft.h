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

#ifndef asiAlgo_SuppressSoft_h
#define asiAlgo_SuppressSoft_h

// asiAlgo includes
#include <asiAlgo_SuppressFaces.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

//! Utility to suppress "soft" (isolated) features.
class asiAlgo_SuppressSoft : public ActAPI_IAlgorithm
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_SuppressSoft, ActAPI_IAlgorithm)

public:

  //! Status codes.
  enum StatusCode
  {
    StatusCode_NoError              = 0,    //!< Successful operation.
    StatusCode_WarnMissingFaces     = 0x01, //!< Some faces missing in the CAD model were requested for removal.
    StatusCode_WarnNoFaces2Suppress = 0x02, //!< There are no faces which can be suppressed.
    StatusCode_WarnHardFacesFound   = 0x04, //!< There are "hard" features which cannot be suppressed.
    StatusCode_ErrFailed            = 0x08  //!< Operation failed.
  };

public:

  //! Constructor.
  //! \param[in] masterCAD full CAD model.
  //! \param[in] aag       attributed adjacency graph.
  //! \param[in] progress  progress notifier.
  //! \param[in] plotter   imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_SuppressSoft(const TopoDS_Shape&        masterCAD,
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

  //! \return unsuppressed features.
  const asiAlgo_Feature& GetUnsuppressed() const
  {
    return m_unsuppressed;
  }

  //! \return AAG.
  const Handle(asiAlgo_AAG)& GetAAG() const
  {
    return m_aag;
  }

  //! \return number of features that will not be suppressed.
  const int GetNumberOfOldUnsuppressed() const
  {
    return m_nbOfOldUnsuppressed;
  }

  //! \return result shape.
  const TopoDS_Shape& GetResult() const
  {
    return m_output;
  }

  //! \return modification history.
  Handle(BRepTools_History) GetHistory() const
  {
    return m_tool->GetHistory();
  }

protected:

  //! Starting from the given seed face, this routine recursively visits
  //! neighbor faces and puts the feature ones to the output collection.
  //! At the end as a result we will have all feature faces corresponding to
  //! an isolated feature.
  //! \param[in]      seed_face_id       ID of the seed face to start search from.
  //! \param[in]      globalFeatureFaces full collection of feature faces requested
  //!                                    for suppression.
  //! \param[out]     localFeatureFaces  collection of the gathered feature faces
  //!                                    which can be reached from the seed one.
  //! \param[in, out] visited            already visited nodes (to skip them).
  void collectLocalFeature(const int              seed_face_id,
                           const asiAlgo_Feature& globalFeatureFaces,
                           asiAlgo_Feature&       localFeatureFaces,
                           asiAlgo_Feature&       visited) const;

private:

  //! Fills history of modification.
  //! \param[in/out] history modification history.
  virtual void buildHistory(Handle(BRepTools_History)& history)
  {
    history = m_tool->GetHistory();
  }

protected:

  TopoDS_Shape                  m_input;        //!< Input shape.
  TopoDS_Shape                  m_output;       //!< Output shape.
  Handle(asiAlgo_AAG)           m_aag;          //!< AAG.
  Handle(asiAlgo_SuppressFaces) m_tool;         //!< Face deletion tool.
  asiAlgo_Feature               m_unsuppressed; //!< Remaining unsuppressed faces, if any.

  // This variable is needed in order to know how many features were not suppressed.
  // After suppression, m_unsuppressed will contain the updated indices of unsuppressed faces,
  // which may be more than before the suppression (unsuppressed faces are collected at the beginning).
  int m_nbOfOldUnsuppressed; //!< The number of features that won't be suppressed.

};

#endif
