//-----------------------------------------------------------------------------
// Created on: May 2012
//-----------------------------------------------------------------------------
// Copyright (c) 2017, OPEN CASCADE SAS
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
//    * Neither the name of OPEN CASCADE SAS nor the
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
//
// Web: http://dev.opencascade.org
//-----------------------------------------------------------------------------

#ifndef ActData_NodeFactory_HeaderFile
#define ActData_NodeFactory_HeaderFile

// Let the static registry of RTTI fill itself
#include <Standard_Type.hxx>

// Active Data includes
#include <ActData_BaseNode.h>
#include <ActData_Common.h>

// Active Data (API) includes
#include <ActAPI_INode.h>

// OCCT includes
#include <NCollection_DataMap.hxx>
#include <TDF_Label.hxx>

//! Pointer to Node allocation routine.
typedef Handle(ActAPI_INode) (*ActData_NodeAllocator)();

#define DEFINE_NODE_FACTORY(C, AllocFunction) \
  class Registrator \
  { \
  public: \
    Registrator() \
    { \
      ActData_NodeFactory::RegisterNodeType(C::get_type_name(), \
                                            AllocFunction); \
    } \
  }; \
  static Registrator m_typeRegistrator;

#define REGISTER_NODE_TYPE(C) \
  C::Registrator C::m_typeRegistrator;

#define REGISTER_PARAMETER_COMMON(Class, Id, IsExpressible) \
  this->registerParameter(Id, ActData_##Class##Parameter::Instance(), IsExpressible)

#define REGISTER_PARAMETER(Class, Id) \
  REGISTER_PARAMETER_COMMON(Class, Id, Standard_False)

#define REGISTER_PARAMETER_EXPR(Class, Id) \
  REGISTER_PARAMETER_COMMON(Class, Id, Standard_True)

//! \ingroup AD_DF
//!
//! Factory for Data Nodes.
class ActData_NodeFactory
{
public:

  //! Mapping between Node types and allocation routines.
  typedef NCollection_DataMap<TCollection_AsciiString, ActData_NodeAllocator> NodeAllocMap;

// Node factory:
public:

  ActData_EXPORT static const NodeAllocMap&
    RegisterNodeType(const TCollection_AsciiString& theType,
                     const ActData_NodeAllocator&   theAllocFunc);

  ActData_EXPORT static const NodeAllocMap&
    GetAllocMap();

  ActData_EXPORT static Standard_Boolean
    IsNode(const TDF_Label&         theLab,
           TCollection_AsciiString& theNodeType);

  ActData_EXPORT static Standard_Boolean
    IsNode(const TDF_Label& theLab);

  ActData_EXPORT static Handle(ActAPI_INode)
    NodeInstanceByType(const TCollection_AsciiString& theNodeType);

  ActData_EXPORT static Standard_Boolean
    CanSettleNode(const TDF_Label& theLab);

  ActData_EXPORT static Handle(ActAPI_INode)
    NodeSettle(const TDF_Label& theLab);

  ActData_EXPORT static void
    NodeSettle(const Handle(ActData_BaseNode)& theNode,
               const TDF_Label&                theLab);

  ActData_EXPORT static Handle(ActAPI_INode)
    NodeByParamSettle(const TDF_Label& theParamLabel);

  ActData_EXPORT static Handle(ActAPI_INode)
    NodeByParamSettle(const Handle(ActAPI_IUserParameter)& theParam);

private:

  static NodeAllocMap m_allocMap;

};

#endif
