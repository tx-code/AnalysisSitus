//-----------------------------------------------------------------------------
// Created on: 11 December(*) 2017
//-----------------------------------------------------------------------------
// Copyright (c) 2017, Sergey Slyadnev
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

#ifndef asiVisu_IVCurve2dPrs_h
#define asiVisu_IVCurve2dPrs_h

// asiData includes
#include <asiData_IVCurve2dNode.h>

// asiVisu includes
#include <asiVisu_DefaultPrs.h>

//! Presentation class for 2D curves in IV.
class asiVisu_IVCurve2dPrs : public asiVisu_DefaultPrs
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiVisu_IVCurve2dPrs, asiVisu_DefaultPrs)

  // Allows to register this Presentation class
  DEFINE_PRESENTATION_FACTORY(asiData_IVCurve2dNode, Instance)

public:

  //! Pipelines.
  enum PipelineId
  {
    Pipeline_Main = 1
  };

public:

  //! Factory method for Presentation.
  //! \param theNode [in] Node to create a Presentation for.
  //! \return new Presentation instance.
  asiVisu_EXPORT static Handle(asiVisu_Prs)
    Instance(const Handle(ActAPI_INode)& N);

public:

  //! Sets custom color.
  //! \param[in] color color to set.
  asiVisu_EXPORT void
    Colorize(const ActAPI_Color& color) const;

protected:

  //! Callback on adding presentation pipelines to renderer.
  virtual void
    renderPipelines(vtkRenderer* renderer) const;

  //! Callback on removing presentation pipelines from renderer.
  virtual void
    deRenderPipelines(vtkRenderer* renderer) const;

  //! Callback for pipeline update.
  virtual void
    afterUpdatePipelines() const;

private:

  //! Allocation is allowed only via Instance() method.
  //! Creates a Presentation object for the passed Node.
  //! \param N [in] Node to create a Presentation for.
  asiVisu_IVCurve2dPrs(const Handle(ActAPI_INode)& N);

};

#endif
