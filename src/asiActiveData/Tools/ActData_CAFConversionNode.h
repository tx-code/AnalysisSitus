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

#ifndef ActData_CAFConversionNode_HeaderFile
#define ActData_CAFConversionNode_HeaderFile

// Active Data includes
#include <ActData_CAFConversionParameter.h>

DEFINE_STANDARD_HANDLE(ActData_CAFConversionNode, Standard_Transient)

//! \ingroup AD_DF
//!
//! Element of Conversion Model representing conversion data in scope
//! of a single Data Node.
class ActData_CAFConversionNode : public Standard_Transient
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_CAFConversionNode, Standard_Transient)

public:

  //! Iterator for Sampler.
  class Iterator
  {
  public:

    //! Default constructor.
    Iterator()
    {
    }

    //! Complete constructor.
    //! \param theModel [in] Conversion Node to iterate over.
    Iterator(const Handle(ActData_CAFConversionNode)& theNode)
    {
      this->Init(theNode);
    }

    //! Initializes iterator.
    //! \param theModel [in] Conversion Node to iterate over.
    void Init(const Handle(ActData_CAFConversionNode)& theNode)
    {
      m_iter.Init(theNode->m_params);
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

    //! Returns currently iterated value.
    //! \return current value.
    const Handle(ActData_CAFConversionParameter)& Value() const
    {
      return m_iter.Value();
    }

  private:

    //! Internal iterator.
    NCollection_Sequence<Handle(ActData_CAFConversionParameter)>::Iterator m_iter;

  };

public:

  ActData_EXPORT
    ActData_CAFConversionNode();

public:

  ActData_EXPORT void
    AddOrigin(const Handle(ActData_ParameterDTO)& theDTO);

  ActData_EXPORT ActAPI_DataObjectId
    NID() const;

  ActData_EXPORT Handle(ActData_CAFConversionParameter)
    ParamByOrigin(const Standard_Integer theOriPID) const;

  ActData_EXPORT Standard_Boolean
    Insert(const Handle(ActData_ParameterDTO)& theDTO,
           const ActAPI_ParameterGID& theGIDBefore);

  ActData_EXPORT Standard_Boolean
    Update(const ActAPI_ParameterGID& theGID,
           const Handle(ActData_ParameterDTO)& theDTO);

  ActData_EXPORT Standard_Boolean
    Remove(const ActAPI_ParameterGID& theGID);

  ActData_EXPORT void
    NormalizePIDs();

private:

  Standard_Integer
    posParameter(const ActAPI_ParameterGID& theGID) const;

private:

  //! Parameters.
  NCollection_Sequence<Handle(ActData_CAFConversionParameter)> m_params;

  //! Minimal original PID.
  Standard_Integer m_iMinPID;

};

#endif
