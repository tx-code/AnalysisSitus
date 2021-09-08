//-----------------------------------------------------------------------------
// Created on: March 2013
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

#ifndef ActData_CAFConversionModel_HeaderFile
#define ActData_CAFConversionModel_HeaderFile

// Active Data includes
#include <ActData_BaseModel.h>
#include <ActData_CAFConversionNode.h>
#include <ActData_Common.h>
#include <ActData_ParameterDTO.h>

DEFINE_STANDARD_HANDLE(ActData_CAFConversionModel, Standard_Transient)

//! \ingroup AD_DF
//!
//! Conversion Model (Sampler) representing transient sampler structure used
//! by Conversion Context.
class ActData_CAFConversionModel : public Standard_Transient
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_CAFConversionModel, Standard_Transient)

public:

  //! Relocation map.
  typedef NCollection_DataMap<ActAPI_ParameterGID,
                              ActAPI_ParameterGID,
                              ActAPI_ParameterGID::Hasher> TRelocMap;

  //! Type shortcut for map of Conversion Nodes against their original IDs.
  typedef NCollection_DataMap<ActAPI_DataObjectId,
                              Handle(ActData_CAFConversionNode)> TNodeMap;

  //! Iterator for Sampler.
  class Iterator
  {
  public:

    //! Default constructor.
    Iterator()
    {
    }

    //! Complete constructor.
    //! \param theModel [in] Conversion Sampler to iterate over.
    Iterator(const Handle(ActData_CAFConversionModel)& theModel)
    {
      this->Init(theModel);
    }

    //! Initializes iterator.
    //! \param theModel [in] Conversion Sampler to iterate over.
    void Init(const Handle(ActData_CAFConversionModel)& theModel)
    {
      m_iter.Initialize(theModel->m_nodeMap);
    }

    //! Returns true if there is something to iterate still.
    //! \return true/false.
    Standard_Boolean More() const
    {
      return m_iter.More();
    }

    //! Mover iteration pointer to the next item.
    void Next()
    {
      m_iter.Next();
    }

    //! Returns currently iterated key.
    //! \return current key.
    const ActAPI_DataObjectId& Key() const
    {
      return m_iter.Key();
    }

    //! Returns currently iterated value.
    //! \return current value.
    const Handle(ActData_CAFConversionNode)& Value() const
    {
      return m_iter.Value();
    }

  private:

    TNodeMap::Iterator m_iter; //!< Internal iterator.

  };

public:

  ActData_EXPORT
    ActData_CAFConversionModel();

public:

  ActData_EXPORT Standard_Boolean
    ContainsNode(const ActAPI_DataObjectId& theNID) const;

  ActData_EXPORT Standard_Boolean
    AddOriginNode(const ActAPI_DataObjectId& theNID,
                  const Handle(ActAPI_IModel)& theModel);

  ActData_EXPORT Handle(ActData_CAFConversionNode)
    GetNode(const ActAPI_DataObjectId& theNID) const;

public:

  ActData_EXPORT void
    Clear();

  ActData_EXPORT void
    BuildRelocationMap();

  ActData_EXPORT ActAPI_ParameterGID
    Converted(const ActAPI_ParameterGID& theOldGID) const;

private:

  TNodeMap  m_nodeMap;  //!< Map of Conversion Nodes.
  TRelocMap m_relocMap; //!< Conversion relocation map.

};

#endif
