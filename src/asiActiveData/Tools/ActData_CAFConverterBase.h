//-----------------------------------------------------------------------------
// Created on: June 2012
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

#ifndef ActData_CAFConverterBase_HeaderFile
#define ActData_CAFConverterBase_HeaderFile

// Active Data includes
#include <ActData_Common.h>

// OCCT includes
#include <Message_ProgressIndicator.hxx>

// Active Data (API) forward declarations
class ActAPI_IModel;

//! Type definition for conversion routine.
typedef Standard_Boolean (*ActData_ConversionRoutine)(Handle(ActAPI_IModel)& theModel,
                                                      const Handle(Message_ProgressIndicator)& theProgress);

//! Type definition for conversion sequence.
typedef NCollection_Sequence<ActData_ConversionRoutine> ActData_ConversionSequence;

//! \ingroup AD_DF
//!
//! Versioning delta represented by the older and newer version tags.
struct ActData_VersionDelta
{
  Standard_Integer OldVersion;
  Standard_Integer NewVersion;

  ActData_VersionDelta() : OldVersion(-1), NewVersion(-1) {}
  ActData_VersionDelta(const Standard_Integer theOldVer,
                       const Standard_Integer theNewVer) : OldVersion(theOldVer),
                                                           NewVersion(theNewVer) {}
};

//! \ingroup AD_DF
//!
//! Hash-function for versioning deltas.
//! \param theVDelta [in] versioning delta.
//! \param theUpper [in] hash integer.
inline Standard_Integer HashCode(const ActData_VersionDelta& theVDelta,
                                 const Standard_Integer theUpper)
{
  Standard_Integer aKey = theVDelta.OldVersion + theVDelta.NewVersion;
  aKey += (aKey << 10);
  aKey ^= (aKey >> 6);
  aKey += (aKey << 3);
  aKey ^= (aKey >> 11);
  return (aKey & 0x7fffffff) % theUpper;
}

//! \ingroup AD_DF
//!
//! Equality checker for versioning deltas.
//! \param theVDelta1 [in] first delta.
//! \param theVDelta2 [in] second delta.
inline Standard_Boolean IsEqual(const ActData_VersionDelta& theVDelta1,
                                const ActData_VersionDelta& theVDelta2)
{
  return theVDelta1.OldVersion == theVDelta2.OldVersion &&
         theVDelta1.NewVersion == theVDelta2.NewVersion;
}

//! \ingroup AD_DF
//!
//! Set of values playing as a basis of conversion process. Includes
//! old/new version pair along with the correspondent conversion routine.
//! plays as a simple DTO for registration of conversion delta.
struct ActData_ConversionTuple
{
  //! Delta of versions.
  ActData_VersionDelta Delta;

  //! Conversion routine.
  ActData_ConversionRoutine Routine;

  //! Default constructor.
  ActData_ConversionTuple() : Delta( ActData_VersionDelta() ),
                              Routine(NULL) {}

  //! Complete constructor.
  //! \param theOldVer [in] old version.
  //! \param theNewVer [in] new version.
  //! \param theRoutine [in] conversion routine.
  ActData_ConversionTuple(const Standard_Integer theOldVer,
                          const Standard_Integer theNewVer,
                          const ActData_ConversionRoutine& theRoutine)
  {
    Delta = ActData_VersionDelta(theOldVer, theNewVer);
    Routine = theRoutine;
  }
};

//! \ingroup AD_DF
//!
// Ordered collection of conversion tuples.
typedef NCollection_Sequence<ActData_ConversionTuple> ActData_ConversionTupleSequence;

//! \ingroup AD_DF
//!
// Ordered collection of conversion tuples manipulated by OCCT Handles.
typedef NCollection_Shared<ActData_ConversionTupleSequence> ActData_HConversionTupleSequence;

//! \ingroup AD_DF
//!
//! Mapping between version deltas and their correspondent conversion
//! routines.
class ActData_ConversionMap
{
public:

  //! Returns new version associated in a version delta with the given
  //! old version.
  //! \param theOldVer [in] old version to access the new version.
  //! \return new version number or -1 if no such old version exists.
  Standard_Integer NewByOld(const Standard_Integer theOldVer)
  {
    for ( _ConversionMap::Iterator it(m_cMap); it.More(); it.Next() )
    {
      const ActData_VersionDelta& aVerDelta = it.Key();
      if ( aVerDelta.OldVersion == theOldVer )
        return aVerDelta.NewVersion;
    }

    return -1;
  }

  //! Binds conversion routine to the given pair of versions.
  //! \param theOldVer [in] old version.
  //! \param theNewVer [in] new version.
  //! \param theRoutine [in] conversion routine to bind.
  void BindRoutine(const Standard_Integer theOldVer,
                   const Standard_Integer theNewVer,
                   const ActData_ConversionRoutine& theRoutine)
  {
    if ( this->NewByOld(theOldVer) != -1 )
      Standard_ProgramError::Raise("Conversion delta already exists");

    m_cMap.Bind(ActData_VersionDelta(theOldVer, theNewVer), theRoutine);
  }

  //! Binds conversion routine to the given pair of versions.
  //! \param theTuple [in] conversion tuple.
  void BindRoutine(const ActData_ConversionTuple& theTuple)
  {
    this->BindRoutine(theTuple.Delta.OldVersion,
                      theTuple.Delta.NewVersion,
                      theTuple.Routine);
  }

  //! Returns conversion routine by the given version delta.
  //! \param theDelta [in] version delta to access the conversion routine for.
  //! \return conversion routine for the given delta.
  ActData_ConversionRoutine RoutineByDelta(const ActData_VersionDelta& theDelta)
  {
    if ( !m_cMap.IsBound(theDelta) )
      return NULL;

    return m_cMap.Find(theDelta);
  }

private:

  //! Type shortcut for mapping between version deltas and their correspondent
  //! conversion routines.
  typedef NCollection_DataMap<ActData_VersionDelta, ActData_ConversionRoutine> _ConversionMap;

private:

  //! Associations between version deltas and their correspondent conversion
  //! routines.
  _ConversionMap m_cMap;

};

//! \ingroup AD_DF
//!
//! Class providing a convenient way for assembling conversion tuples.
class ActData_ConversionStream
{
public:

  Handle(ActData_HConversionTupleSequence) List; //!< Actual list of tuples.

public:

  //! Conversion operator.
  operator Handle(ActData_HConversionTupleSequence)()
  {
    return List;
  }

  //! Pushes the passed conversion tuple to the internal list.
  //! \param theTuple [in] tuple to append to the internal list.
  //! \return this instance for further streaming.
  ActData_ConversionStream& operator<<(const ActData_ConversionTuple& theTuple)
  {
    if ( List.IsNull() )
      List = new ActData_HConversionTupleSequence();

    List->Append(theTuple);

    return *this;
  }

};

#endif
