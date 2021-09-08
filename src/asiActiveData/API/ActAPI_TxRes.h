//-----------------------------------------------------------------------------
// Created on: November 2018
//-----------------------------------------------------------------------------
// Copyright (c) 2018-present, OPEN CASCADE SAS
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

#ifndef ActAPI_TxRes_HeaderFile
#define ActAPI_TxRes_HeaderFile

// Active Data (API) includes
#include <ActAPI_IDataCursor.h>

//! \ingroup AD_API
//!
//! Transaction result.
class ActAPI_TxRes : public Standard_Transient
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActAPI_TxRes, Standard_Transient)

public:

  //! Reference to a persistent Parameter with additional flag indicating
  //! whether this object is currently alive or not.
  struct t_parameterRef
  {
    ActAPI_DataObjectId        id;          //!< Persistent ID.
    Handle(ActAPI_IDataCursor) dc;          //!< Optional Data Cursor.
    Standard_Boolean           isAlive;     //!< Flag indicating whether the object is alive or not.
    Standard_Boolean           isUndefined; //!< Indicates whether a Parameter is of undefined type.

    //-----------------------------------------------------------------------//

    t_parameterRef() : isAlive(Standard_False), isUndefined(Standard_False) {}

    //-----------------------------------------------------------------------//

    struct Hasher
    {
      static Standard_Integer HashCode(const t_parameterRef&  theObject,
                                       const Standard_Integer theNbBuckets = 100)
      {
        return ::HashCode(theObject.id, theNbBuckets);
      }

      static Standard_Boolean IsEqual(const t_parameterRef& theObject1,
                                      const t_parameterRef& theObject2)
      {
        return ::IsEqual(theObject1.id, theObject2.id);
      }
    };
  };

public:

  //! Adds a reference to the persistent object.
  //! \param[in] _id          persistent ID.
  //! \param[in] _dc          Data Cursor.
  //! \param[in] _isAlive     status of the object in the current transaction.
  //! \param[in] _isUndefined indicates if this Data Cursor is of undefined type.
  void Add(const ActAPI_DataObjectId&        _id,
           const Handle(ActAPI_IDataCursor)& _dc,
           const Standard_Boolean            _isAlive,
           const Standard_Boolean            _isUndefined)
  {
    t_parameterRef ref;
    ref.id          = _id;
    ref.dc          = _dc;
    ref.isAlive     = _isAlive;
    ref.isUndefined = _isUndefined;
    //
    this->parameterRefs.Add(ref);
  }

public:

  //! References to persistent objects (Parameters by convention) returned
  //! as a result of transaction.
  NCollection_IndexedMap<t_parameterRef, t_parameterRef::Hasher> parameterRefs;

};

#endif
