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

#ifndef asiData_IVCurve2dNode_h
#define asiData_IVCurve2dNode_h

// asiData includes
#include <asiData.h>

// Active Data includes
#include <ActData_BaseNode.h>

// OCCT includes
#include <Geom_Surface.hxx>
#include <Geom2d_Curve.hxx>

//-----------------------------------------------------------------------------
// Data Node representing a single 2D curve in IV (Imperative Viewer)
//-----------------------------------------------------------------------------

//! Data Node representing a single 2D curve in IV (Imperative Viewer). To
//! take advantage of Active Data which is able to store topological shapes
//! only (not geometry itself), we have to wrap the stored geometric primitive
//! with a topological container. For s 2D curve, such a container is an edge
//! with CONS (Curve ON Surface) representation.
class asiData_IVCurve2dNode : public ActData_BaseNode
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiData_IVCurve2dNode, ActData_BaseNode)

  // Automatic registration of Node type in global factory
  DEFINE_NODE_FACTORY(asiData_IVCurve2dNode, Instance)

public:

  //! IDs for the underlying Parameters.
  enum ParamId
  {
  //------------------//
  // Common           //
  //------------------//
    PID_Name,         //!< Name of the Node.
  //------------------//
    PID_Curve,        //!< Stored geometry.
    PID_Surface,      //!< Host surface for the 2D curve.
  //------------------//
    PID_GroupPrs,     //!< Presentation group.
    PID_DrawOriTip,   //!< Whether to draw orientation tip for the curve.
    PID_HasColor,     //!< Indicates whether the Color Parameter is active.
    PID_Color,        //!< Color.
  //------------------//
    PID_Last = PID_Name + ActData_BaseNode::RESERVED_PARAM_RANGE
  };

public:

  //! Returns new DETACHED instance of the Node ensuring its correct
  //! allocation in a heap.
  //! \return new instance of the Node.
  asiData_EXPORT static Handle(ActAPI_INode)
    Instance();

// Generic naming support:
public:

  //! Accessor for the Node's name.
  //! \return name of the Node.
  asiData_EXPORT virtual TCollection_ExtendedString
    GetName();

  //! Sets name for the Node.
  //! \param theName [in] name to set.
  asiData_EXPORT virtual void
    SetName(const TCollection_ExtendedString& name);

// Handy accessors to the stored data:
public:

  //! Returns the stored geometry.
  //! \param surface [out] host surface.
  //! \param f       [out] first parameter.
  //! \param l       [out] last parameter.
  //! \return stored geometry.
  asiData_EXPORT Handle(Geom2d_Curve)
    GetCONS(Handle(Geom_Surface)& surface,
            double&               f,
            double&               l) const;

  //! Sets curve to store.
  //! \param curve   [in] geometry to store.
  //! \param surface [in] host surface.
  //! \param f       [in] first parameter of the curve.
  //! \param l       [in] last parameter of the curve.
  asiData_EXPORT void
    SetCONS(const Handle(Geom2d_Curve)& curve,
            const Handle(Geom_Surface)& surface,
            const double                f,
            const double                l);

  //! Sets the property indicating whether to draw the orientation tip
  //! for the curve.
  //! \param[in] on true/false.
  asiData_EXPORT void
    SetDrawOrientationTip(const bool on);

  //! \return stored value of orientation tip flag.
  asiData_EXPORT bool
    GetDrawOrientationTip() const;

  //! Sets the property indicating whether this Node has the Color Parameter activated.
  asiData_EXPORT void
    SetHasColor(const bool);

  //! \return the Boolean flag indicating whether this Node has the Color Parameter activated.
  asiData_EXPORT bool
    HasColor() const;

  //! Sets the color.
  asiData_EXPORT void
    SetColor(const int);

  //! \return color.
  asiData_EXPORT int
    GetColor() const;

// Initialization:
public:

  //! Performs initial actions required to make Node WELL-FORMED.
  asiData_EXPORT void
    Init();

protected:

  //! Default constructor. Registers all involved Parameters.
  //! Allocation is allowed only via Instance() method.
  asiData_EXPORT
    asiData_IVCurve2dNode();

};

#endif
