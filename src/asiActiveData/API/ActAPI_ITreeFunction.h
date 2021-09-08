//-----------------------------------------------------------------------------
// Created on: March 2012
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

#ifndef ActAPI_ITreeFunction_HeaderFile
#define ActAPI_ITreeFunction_HeaderFile

// Active Data (API) includes
#include <ActAPI_IParameter.h>

// OCCT includes
#include <Standard_GUID.hxx>
#include <Standard_Type.hxx>

//! \ingroup AD_API
//!
//! Tree Function interface.
class ActAPI_ITreeFunction : public Standard_Transient
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActAPI_ITreeFunction, Standard_Transient)

public:

  ActData_EXPORT virtual ~ActAPI_ITreeFunction();

public:

  virtual Standard_Integer
    Execute(const Handle(ActAPI_HParameterList)& theArgsIN,
            const Handle(ActAPI_HParameterList)& theArgsOUT) const = 0;

  virtual Standard_CString
    GetGUID() const = 0; 

  virtual Standard_CString
    GetName() const = 0;

  virtual Standard_Boolean
    MustExecuteIntact(const Handle(ActAPI_HParameterList)& theArgsIN,
                      const Handle(Standard_Transient)&    theUserData = nullptr) const = 0;

  virtual Standard_Boolean
    IsHeavy() const = 0;

  virtual Standard_Integer
    Priority() const = 0;

  virtual const Handle(Standard_Transient)&
    GetUserData() const = 0;

};

//! Shortcuts for list of Tree Function GUIDs.
typedef NCollection_Sequence<Standard_GUID>     ActAPI_FuncGUIDList;
typedef NCollection_Shared<ActAPI_FuncGUIDList> ActAPI_HFuncGUIDList;

//-----------------------------------------------------------------------------
// Tree Function streaming
//-----------------------------------------------------------------------------

//! \ingroup AD_API
//!
//! Class providing a convenient way for assembling Tree Function GUIDs.
class ActAPI_FuncGUIDStream
{
public:

  Handle(ActAPI_HFuncGUIDList) List; //!< Actual list.

public:

  //! Default constructor.
  ActAPI_FuncGUIDStream()
  {
    List = new ActAPI_HFuncGUIDList();
  }

  //! Copy constructor. Performs SHALLOW copying.
  //! \param S [in] stream to copy.
  ActAPI_FuncGUIDStream(const ActAPI_FuncGUIDStream& S)
  {
    List = S.List;
  }

  //! Conversion operator.
  operator Handle(ActAPI_HFuncGUIDList)()
  {
    return List;
  }

  //! Pushes the passed GUID to the internal list.
  //! \param GUID [in] GUID to append to the internal list.
  //! \return this instance for further streaming.
  ActAPI_FuncGUIDStream& operator<<(const Standard_GUID& GUID)
  {
    List->Append(GUID);

    return *this;
  }
};

#endif
