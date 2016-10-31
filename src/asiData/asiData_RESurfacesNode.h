//-----------------------------------------------------------------------------
// Created on: 05 April 2016
// Created by: Sergey SLYADNEV
//-----------------------------------------------------------------------------
// Web: http://dev.opencascade.org/
//-----------------------------------------------------------------------------

#ifndef asiData_RESurfacesNode_h
#define asiData_RESurfacesNode_h

// Geometry includes
#include <asiData_RESurfaceNode.h>

// OCCT includes
#include <TColGeom_SequenceOfSurface.hxx>

//-----------------------------------------------------------------------------
// Surfaces for Reverse Engineering
//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(asiData_RESurfacesNode, ActData_BaseNode)

//! Node representing surfaces for reverse engineering.
class asiData_RESurfacesNode : public ActData_BaseNode
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiData_RESurfacesNode, ActData_BaseNode)

  // Automatic registration of Node type in global factory
  DEFINE_NODE_FACTORY(asiData_RESurfacesNode, Instance)

public:

  //! IDs for the underlying Parameters.
  enum ParamId
  {
  //------------------//
  // Common           //
  //------------------//
    PID_Name,         //!< Name of the Node.
  //------------------//
    PID_Last = PID_Name + ActData_BaseNode::RESERVED_PARAM_RANGE
  };

public:

  asiData_EXPORT static Handle(ActAPI_INode)
    Instance();

// Generic naming support:
public:

  asiData_EXPORT virtual TCollection_ExtendedString
    GetName();

  asiData_EXPORT virtual void
    SetName(const TCollection_ExtendedString& theName);

// Handy accessors to the stored data:
public:

  asiData_EXPORT Handle(asiData_RESurfaceNode)
    Surface(const int oneBased_idx);

  asiData_EXPORT Handle(asiData_RESurfaceNode)
    TX_AddSurface(const Handle(Geom_Surface)& surface,
                  const double                uLimit,
                  const double                vLimit);

  asiData_EXPORT void
    TX_CleanSurfaces();

// Initialization:
public:

  asiData_EXPORT void
    Init();

protected:

  //! Allocation is allowed only via Instance method.
  asiData_EXPORT
    asiData_RESurfacesNode();

};

#endif
