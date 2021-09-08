// File:      ActData_RealVarPartition.h
// Created:   April 2012
// Author:     Sergey SLYADNEV

#ifndef ActData_RealVarPartition_HeaderFile
#define ActData_RealVarPartition_HeaderFile

// Active Data includes
#include <ActData_BasePartition.h>
#include <ActData_Common.h>

DEFINE_STANDARD_HANDLE(ActData_RealVarPartition, ActData_BasePartition)

//! \ingroup AD_DF
//!
//! Partition for Real Variable Nodes.
class ActData_RealVarPartition : public ActData_BasePartition
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_RealVarPartition, ActData_BasePartition)

public:

  ActData_EXPORT static Handle(ActData_RealVarPartition) Instance();

public:

  ActData_EXPORT virtual Handle(Standard_Type)
    GetNodeType() const;

protected:

  //! Allow allocation only via Instance method.
  ActData_RealVarPartition();

};

#endif
