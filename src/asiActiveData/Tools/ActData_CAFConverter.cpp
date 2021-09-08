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

// Own include
#include <ActData_CAFConverter.h>

// Active Data includes
#include <ActData_BaseModel.h>
#include <ActData_BasePartition.h>

// OCCT includes
#include <Message_ProgressSentry.hxx>

//! Constructor accepting the conversion stream managed by this CAF Converter
//! instance.
//! \param theCStream [in] conversion stream.
ActData_CAFConverter::ActData_CAFConverter(const ActData_ConversionStream& theCStream)
  : Standard_Transient()
{
  for ( ActData_ConversionTupleSequence::Iterator it( *theCStream.List.operator->() );
        it.More(); it.Next() )
  {
    this->registerConversionRoutine( it.Value() );
  }
}

//! Performs conversion of the Data Model from the passed older version
//! to the given newer one. Returns false if such conversion is impossible.
//! \param theModel [in/out] Data Model to convert.
//! \param theOldVer [in] older version to perform conversion from.
//! \param theNewVer [in] newer version to perform conversion to.
//! \param theProgress [in] Progress Indicator.
//! \return true/false.
Standard_Boolean
  ActData_CAFConverter::Perform(Handle(ActAPI_IModel)& theModel,
                                const Standard_Integer theOldVer,
                                const Standard_Integer theNewVer,
                                const Handle(Message_ProgressIndicator)& theProgress)
{
  Handle(ActData_BaseModel) aModelBase = Handle(ActData_BaseModel)::DownCast(theModel);

  // Prepare a sequence of conversion routines to apply.
  ActData_ConversionSequence aCSeq;

  // Iterate over the conversion repository in order to check if the
  // ultimate version is reachable
  Standard_Integer aNextVer = -1, aCurrentOldVer = theOldVer;
  Standard_Boolean isNewVerReachable = Standard_True;
  while ( aNextVer != theNewVer )
  {
    aNextVer = m_conversionMap.NewByOld(aCurrentOldVer);
    if ( aNextVer == -1 )
    {
      isNewVerReachable = Standard_False;
      break;
    }
    aCSeq.Append( m_conversionMap.RoutineByDelta( ActData_VersionDelta(aCurrentOldVer, aNextVer) ) );
    aCurrentOldVer = aNextVer;
  }

  if ( !isNewVerReachable )
    return Standard_False;

  // Prepare progress entry
  Message_ProgressSentry PEntry(theProgress, "CAF_CONVERSION_TRAVERSING", 0, aCSeq.Length(), 1);
  PEntry.Show();

  // Apply the conversion sequence
  for ( ActData_ConversionSequence::Iterator it(aCSeq); it.More(); it.Next() )
  {
    const ActData_ConversionRoutine& aRoutine = it.Value();
    theModel->DisableTransactions();
    Standard_Boolean aConvRes = (*aRoutine)(theModel, theProgress);
    if ( !aConvRes )
    {
      return Standard_False;
    }
    PEntry.Next();
    PEntry.Show();
  }

  return Standard_True;
}

//! Performs registration of the given conversion routine for the passed
//! version delta.
//! \param theTuple [in] coversion tuple.
void ActData_CAFConverter::registerConversionRoutine(const ActData_ConversionTuple& theTuple)
{
  m_conversionMap.BindRoutine(theTuple);
}
