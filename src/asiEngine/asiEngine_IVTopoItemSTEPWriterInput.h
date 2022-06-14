//-----------------------------------------------------------------------------
// Created on: 19 April 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Andrey Voevodin
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

#ifndef asiEngine_IVTopoItemSTEPWriterInput_h
#define asiEngine_IVTopoItemSTEPWriterInput_h

// asiEngine includes
#include <asiEngine_Model.h>

// asiAlgo includes
#include <asiAlgo_WriteSTEPWithMetaInput.h>

//-----------------------------------------------------------------------------

//! Input data provider for the STEP writer with metadata.
class asiEngine_IVTopoItemSTEPWriterInput : public asiAlgo_WriteSTEPWithMetaInput
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiEngine_IVTopoItemSTEPWriterInput, asiAlgo_WriteSTEPWithMetaInput)

public:

  //! Ctor.
  //! \param[in] topoItemIV Topo item in IV (Imperative Viewer).
  //! \param[in] M Data Model.
  asiEngine_EXPORT
    asiEngine_IVTopoItemSTEPWriterInput(const Handle(asiData_IVTopoItemNode)& topoItemIV,
                                        const Handle(asiEngine_Model)&        M);

public:

  //! \return shape to write.
  asiEngine_EXPORT virtual TopoDS_Shape
    GetShape() const;

  //! \return number of subshapes to be resolved by the STEP writer as
  //!         individual entities to attach metadata to.
  asiEngine_EXPORT virtual int
    GetNumSubShapes() const;

  //! Accessor for a subshape by its zero-based index.
  //! \param[in] zeroBasedIdx zero-based index of a sub-shape to access.
  //! \return transient pointer to a sub-shape.
  asiEngine_EXPORT virtual TopoDS_Shape
    GetSubShape(const int zeroBasedIdx) const;

  //! Get additional location.
  //! \return location.
  asiEngine_EXPORT virtual TopLoc_Location
    GetLocation() const;

  //! Checks whether the passed shape has color or not.
  //! \param[in] shape shape in question.
  //! \return false if no color is attached, true -- otherwise.
  asiEngine_EXPORT virtual bool
    HasColor(const TopoDS_Shape& shape) const;

  //! Accessor for the color associated with the given shape.
  //! \param[in] shape shape in question.
  //! \return attached color.
  asiEngine_EXPORT virtual Quantity_Color
    GetColor(const TopoDS_Shape& shape) const;

  //! Accessor for the color.
  //! \return common color.
  virtual Quantity_Color
    GetCommonColor() const;

  //! Checks whether common shape has color or not.
  //! \return false if no common color is attached, true -- otherwise.
  virtual bool
    HasCommonColor() const;

protected:

  //! Topo item in IV (Imperative Viewer).
  Handle(asiData_IVTopoItemNode) m_topoItem;

  //! Data Model instance.
  Handle(asiEngine_Model)        m_model;

};

#endif
