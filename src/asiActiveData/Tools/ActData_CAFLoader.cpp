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

// Own include
#include <ActData_CAFLoader.h>

// Active Data includes
#include <ActData_BasePartition.h>
#include <ActData_CAFConverter.h>
#include <ActData_CAFDumper.h>

// OCCT includes
#include <Message_ProgressSentry.hxx>

//-----------------------------------------------------------------------------
// Static services
//-----------------------------------------------------------------------------

//! Loads Data Model from file and performs compatibility conversion if
//! necessary. Once conversion is done, checks whether the resulting Model
//! is composed of WELL-FORMED Data Objects only.
//! \param theFilename [in] name of the file containing CAF Document to open.
//! \param theModel [in/out] resulting Data Model. Must be not-null instance
//!        initially.
//! \return true in case of success, false -- otherwise.
Standard_Boolean
  ActData_CAFLoader::LoadAndConvert(const TCollection_AsciiString& theFilename,
                                    Handle(ActAPI_IModel)& theModel)
{
  /* ===============================
   *  Load Model contents from file
   * =============================== */

  Standard_Integer aVerFwStored, aVerAppStored,
                   aVerFwActual, aVerAppActual;

  if ( !LoadWithVersions(theFilename, theModel,
                         aVerFwStored, aVerAppStored,
                         aVerFwActual, aVerAppActual) )
    return Standard_False;

  /* ============================================
   *  Perform backward compatibility conversions
   * ============================================ */

  if ( !Convert(theModel,
                aVerFwStored, aVerAppStored,
                aVerFwActual, aVerAppActual) )
    return Standard_False;

  /* =========================================================
   *  Check whether bad-formed Nodes exist. If so, the loaded
   *  Document is said to be invalid
   * ========================================================= */

  if ( CheckWellFormed(theModel) )
  {
    /* =====================================================
     *  Let the final state of the Data Model reconnect its
     *  Tree Functions (this should be done at the very end
     *  of the conversion process)
     * ===================================================== */

    if ( !ReconnectTreeFunctions(theModel) )
      return Standard_False;

    return Standard_True;
  }

  // Bad-formed Nodes were found after successful conversion. We have to
  // mark such Model as faulty, otherwise it becomes dangerous
  Handle(ActData_BaseModel) BM = Handle(ActData_BaseModel)::DownCast(theModel);
  if ( BM->m_versionStatus == ActData_BaseModel::Version_LessOk )
    BM->m_versionStatus = ActData_BaseModel::Version_LessFail;

  return Standard_False;
}

//! Loads Data Model from the given file. Conversion is not performed by this
//! method.
//! \param theFilename [in] file to load CAF Document from.
//! \param theModel [in/out] resulting Data Model. Must be not-null instance
//!        initially.
//! \param theFwVerStored [out] version of Framework stored in the
//!        loaded file.
//! \param theAppVerStored [out] version of Application stored in the
//!        loaded file.
//! \param theFwVerActual [out] actual version of Framework. This value is
//!        initialized here for convenience. It is not stored in the file.
//! \param theAppVerActual [out] actual version of Application. This value is
//!        initialized here for convenience. It is not stored in the file.
//! \return true in case of success, false -- otherwise.
Standard_Boolean
  ActData_CAFLoader::LoadWithVersions(const TCollection_AsciiString& theFilename,
                                      Handle(ActAPI_IModel)& theModel,
                                      Standard_Integer& theFwVerStored,
                                      Standard_Integer& theAppVerStored,
                                      Standard_Integer& theFwVerActual,
                                      Standard_Integer& theAppVerActual)
{
  /* ===============================
   *  Load Model contents from file
   * =============================== */

  if ( theModel.IsNull() )
    return Standard_False;

  if ( !theModel->Open(theFilename) )
    return Standard_False;

  Handle(ActData_BaseModel)
    aModelBase = Handle(ActData_BaseModel)::DownCast(theModel);

  /* ================================
   *  Retrieve and check the version
   * ================================ */

  // Stored versions from the opened Document
  Standard_Integer aStoredVersionFw = aModelBase->storedVersionFramework();
  Standard_Integer aStoredVersionApp = aModelBase->storedVersionApp();

  // Actual versions of the environment
  Standard_Integer aCurrentVerFw = aModelBase->actualVersionFramework();
  Standard_Integer aCurrentVerApp = aModelBase->actualVersionApp();

  if ( aStoredVersionFw == -1 ||
       aStoredVersionApp == -1 ) // Version(s) not bound
  {
    aModelBase->Release(ActData_BaseModel::Version_NotBoundFail);
    return Standard_False;
  }

  if ( aStoredVersionFw > aCurrentVerFw ||
       aStoredVersionApp > aCurrentVerApp ) // Version is higher than current
  {
    aModelBase->Release(ActData_BaseModel::Version_HigherFail);
    return Standard_False;
  }

  // Initialize string representations of versions
  theFwVerStored  = aStoredVersionFw;
  theAppVerStored = aStoredVersionApp;
  theFwVerActual  = aCurrentVerFw;
  theAppVerActual = aCurrentVerApp;

  // TODO: Buffering section and LogBook must be ignored in SaveAs, so there
  //       will be no need to clean up them here once it is fixed!!!

  // Disable transactions
  aModelBase->DisableTransactions();

  // Release not useful sections
  aModelBase->releaseCopyPasteBuffer();
  aModelBase->FuncReleaseLogBook();

  // Enable transactions
  aModelBase->EnableTransactions();

  return Standard_True;
}

//! Performs Data Model conversion.
//! \param theModel [in/out] Data Model to convert.
//! \param theFwVerStored [in] stored version of Framework.
//! \param theAppVerStored [in] stored version of Application.
//! \param theFwVerActual [in] actual version of Framework.
//! \param theAppVerActual [in] actual version of Application.
//! \param theFnBefore [in] log file to dump Data Model before conversion.
//! \param theFnAfter [in] log file to dump Data Model after conversion.
//! \param theProgress [in] Progress Indicator.
//! \return true in case of success, false -- otherwise.
Standard_Boolean
  ActData_CAFLoader::Convert(Handle(ActAPI_IModel)& theModel,
                             const Standard_Integer theFwVerStored,
                             const Standard_Integer theAppVerStored,
                             const Standard_Integer theFwVerActual,
                             const Standard_Integer theAppVerActual,
                             const TCollection_AsciiString& theFnBefore,
                             const TCollection_AsciiString& theFnAfter,
                             const Handle(Message_ProgressIndicator)& theProgress)
{
  Handle(ActData_BaseModel)
    aModelBase = Handle(ActData_BaseModel)::DownCast(theModel);

  if ( aModelBase.IsNull() )
    return Standard_False;

  aModelBase->DisableTransactions();

  // Prepare progress
  Message_ProgressSentry PEntry(theProgress, "CAF_CONVERSION_FW_APP", 0, 2, 1);
  PEntry.Show();

  // Dump BEFORE
  if ( !theFnBefore.IsEmpty() )
    ActData_CAFDumper::Dump(theFnBefore, aModelBase,
                            ActData_CAFDumper::Content_Plain,
                            ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // FRAMEWORK conversion
  if ( theFwVerStored < theFwVerActual )
  {
    if ( !aModelBase->converterFw()->Perform(theModel, theFwVerStored, theFwVerActual, theProgress) )
    {
      aModelBase->Release(ActData_BaseModel::Version_LessFail);
      return Standard_False;
    }
    else
    {
      aModelBase->m_status |= ActData_BaseModel::MS_Modified;
      aModelBase->m_versionStatus = ActData_BaseModel::Version_LessOk;
    }
  }

  // Step progress
  PEntry.Next();
  PEntry.Show();

  // APPLICATION conversion
  if ( theAppVerStored < theAppVerActual )
  {
    Handle(ActData_CAFConverter) aConverterApp = aModelBase->converterApp();
    if ( aConverterApp.IsNull() ||
        !aConverterApp->Perform(theModel, theAppVerStored, theAppVerActual, theProgress) )
    {
      aModelBase->Release(ActData_BaseModel::Version_LessFail);
      return Standard_False;
    }
    else
    {
      aModelBase->m_status |= ActData_BaseModel::MS_Modified;
      aModelBase->m_versionStatus = ActData_BaseModel::Version_LessOk;
    }
  }

  if ( aModelBase->m_versionStatus == ActData_BaseModel::Version_Undefined )
    aModelBase->m_versionStatus = ActData_BaseModel::Version_Ok;

  // Dump AFTER
  if ( !theFnAfter.IsEmpty() )
    ActData_CAFDumper::Dump(theFnAfter, aModelBase,
                            ActData_CAFDumper::Content_Plain,
                            ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Make sure that transactions are enabled after conversion
  aModelBase->EnableTransactions();

  // Push to results
  theModel = aModelBase;

  return Standard_True;
}

//! Checks whether the passed Data Model contains WELL-FORMED Nodes only.
//! \param theModel [in] Data Model to check.
//! \return true if Data Model is Ok, false -- otherwise.
Standard_Boolean
  ActData_CAFLoader::CheckWellFormed(const Handle(ActAPI_IModel)& theModel)
{
  if ( theModel.IsNull() )
    return Standard_False;

  Handle(ActAPI_HPartitionList) aPList = theModel->Partitions();
  for ( ActAPI_PartitionList::Iterator it( *aPList.operator->() ); it.More(); it.Next() )
  {
    // Access base Partition
    const Handle(ActAPI_IPartition)& aPart = it.Value();
    Handle(ActData_BasePartition) aPartBase = Handle(ActData_BasePartition)::DownCast(aPart);

    if ( aPartBase.IsNull() )
      Standard_ProgramError::Raise("Non-standard Partitions prohibited");

    // Iterate over the Nodes in their persistent order
    ActData_BasePartition::Iterator aNodeIt(aPartBase);
    for ( ; aNodeIt.More(); aNodeIt.Next() )
    {
      Handle(ActAPI_INode) aNode = aNodeIt.Value();
      if ( !aNode->IsWellFormed() )
        return Standard_False;
    }
  }

  return Standard_True;
}

//! This function is invoked by the converter to let the Data Model reconnect
//! its Tree Functions upon completion of the conversion procedure. The
//! Tree Functions must be reconnected at the very end of the conversion routine
//! as normally such reconnecting requires data access through the actual cursors
//! which should be populated with the normalized data. If, in contrast, the
//! reconnection is being done as long as the conversion process unfolds, there is
//! a high probability to supply cursors with inconsistent data.
//!
//! \param theModel [in] final Data Model (after all conversions) to reconnect
//!                      Tree Functions for.
//! \return false if reconnection is impossible for whatever reason.
Standard_Boolean
  ActData_CAFLoader::ReconnectTreeFunctions(const Handle(ActAPI_IModel)& theModel)
{
  return theModel->FuncReconnectAll();
}

//-----------------------------------------------------------------------------
// Kernel
//-----------------------------------------------------------------------------

//! Constructor.
//! \param theModel [in] initial Model instance to load Document to.
ActData_CAFLoader::ActData_CAFLoader(const Handle(ActAPI_IModel)& theModel)
: Standard_Transient(),
  m_model        (theModel),
  m_fwVerStored  (-1),
  m_appVerStored (-1),
  m_fwVerActual  (-1),
  m_appVerActual (-1)
{
}

//! Loads Data Model from file without conversion. Once loading is done,
//! you can access retrieved Model and versions via dedicated methods.
//! \param theFilename [in] name of the file containing CAF Document to open.
//! \return true in case of success, false -- otherwise.
Standard_Boolean
  ActData_CAFLoader::Load(const TCollection_AsciiString& theFilename)
{
  m_filename = theFilename;

  /* ===============================
   *  Load Model contents from file
   * =============================== */

  Standard_Integer aVerFwStored, aVerAppStored,
                   aVerFwActual, aVerAppActual;

  if ( !LoadWithVersions(m_filename, m_model,
                         aVerFwStored, aVerAppStored,
                         aVerFwActual, aVerAppActual) )
    return Standard_False;

  if ( aVerFwStored != -1 )
    m_fwVerStored = aVerFwStored;

  if ( aVerAppStored != -1 )
    m_appVerStored = aVerAppStored;

  if ( aVerFwActual != -1 )
    m_fwVerActual = aVerFwActual;

  if ( aVerAppActual != -1 )
    m_appVerActual = aVerAppActual;

  return Standard_True;
}

//! Checks whether the loaded Data Model needs conversion or not.
//! \return true/false.
Standard_Boolean ActData_CAFLoader::NeedsConversion() const
{
  if ( m_fwVerStored < m_fwVerActual ||
       m_appVerStored < m_appVerActual )
    return Standard_True;

  return Standard_False;
}

//! Performs conversion.
//! \param theFnBefore [in] log file to dump Data Model before conversion.
//! \param theFnAfter [in] log file to dump Data Model after conversion.
//! \param theProgress [in] Progress Indicator.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_CAFLoader::Convert(const TCollection_AsciiString& theFnBefore,
                                            const TCollection_AsciiString& theFnAfter,
                                            const Handle(Message_ProgressIndicator)& theProgress)
{
  return Convert(m_model,
                 m_fwVerStored, m_appVerStored,
                 m_fwVerActual, m_appVerActual,
                 theFnBefore, theFnAfter,
                 theProgress);
}

//! Checks whether the resulting Data Model contains WELL-FORMED Nodes only.
//! \return true/false.
Standard_Boolean ActData_CAFLoader::CheckWellFormed() const
{
  return CheckWellFormed(m_model);
}

//! Reconnects Tree Functions in the Data Model.
//! \return true/false.
Standard_Boolean ActData_CAFLoader::ReconnectTreeFunctions()
{
  return ReconnectTreeFunctions(m_model);
}

//! Converts the passed version number (stored in OCAF) to human-readable
//! string representation in dot-form, i.e. xxx.yyy.zzz.
//! \param theVerNum [in] version number to convert.
//! \return string representation of version.
TCollection_AsciiString
  ActData_CAFLoader::versionToString(const Standard_Integer theVerNum) const
{
  Standard_Integer aVerMajor = ( theVerNum >> 16 ) & 0xff;
  Standard_Integer aVerMinor = ( theVerNum >> 8  ) & 0xff;
  Standard_Integer aVerPatch = ( theVerNum       ) & 0xff;
  TCollection_AsciiString aVerStr = TCollection_AsciiString(aVerMajor)
                                              .Cat(".").Cat(aVerMinor)
                                              .Cat(".").Cat(aVerPatch);
  return aVerStr;
}
