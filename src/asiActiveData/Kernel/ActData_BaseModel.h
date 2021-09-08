//-----------------------------------------------------------------------------
// Created on: February 2012
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

#ifndef ActData_BaseModel_HeaderFile
#define ActData_BaseModel_HeaderFile

// Active Data includes
#include <ActData_TransactionEngine.h>
#include <ActData_Common.h>
#include <ActData_CopyPasteEngine.h>
#include <ActData_FuncExecutionCtx.h>
#include <ActData_LogBook.h>

// Active Data (API) includes
#include <ActAPI_IModel.h>
#include <ActAPI_ITreeFunction.h>

// OCCT includes
#include <NCollection_DataMap.hxx>
#include <Standard_GUID.hxx>
#include <TDF_LabelList.hxx>
#include <TDocStd_Document.hxx>

// Active Data (API) forward declarations
class ActAPI_IPartition;
class ActAPI_INode;

// Active Data forward declarations
class ActData_BaseNode;
class ActData_BasePartition;
class ActData_BaseTreeFunction;
class ActData_BaseVarNode;
class ActData_CAFConverter;
class ActData_TreeFunctionParameter;

//-----------------------------------------------------------------------------

//! \ingroup AD_DF
//!
//! Base class for ACT Data Models.
class ActData_BaseModel : public ActAPI_IModel
{
friend class ActData_CAFConverter;
friend class ActData_CAFConversionAsset;
friend class ActData_CAFConversionCtx;
friend class ActData_CAFDumper;
friend class ActData_CAFLoader;
friend class ActData_CopyPasteEngine;

public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_BaseModel, ActAPI_IModel)

// Internal tags:
public:

  // Top-level structural CAF tagging.
  enum StructureTags
  {
    StructureTag_Version         = 1,
    StructureTag_Partitions      = 2,
    StructureTag_CopyPasteBuffer = 3,
    StructureTag_LogBook         = 4
  };

  //! Tags for storing versions.
  enum VersionTags
  {
    VersionTag_Framework   = 1,
    VersionTag_Application = 2
  };

public:

  ActData_EXPORT static Standard_Boolean MTime_On;

public:

  //! Creates clone of Data Model of the given type.
  //! \return cloned instance.
  template <typename ModelType>
  static Handle(ActAPI_IModel) CloneInstance()
  {
    Handle(ActAPI_IModel) aRes = new ModelType;
    if ( aRes->NewEmpty() )
      return aRes;
    return NULL;
  }

// Persistence basic:
public:

  ActData_EXPORT virtual Standard_Boolean
    NewEmpty();

  ActData_EXPORT virtual Standard_Boolean
    Open(const TCollection_AsciiString& theFilename,
         ActAPI_ProgressEntry           theNotifier = nullptr);

  ActData_EXPORT virtual void
    Release(const VersionStatus theVersionStatus = Version_Undefined);

  ActData_EXPORT virtual Standard_Boolean
    SaveAs(const TCollection_AsciiString& theFilename,
           ActAPI_ProgressEntry           theNotifier = nullptr);

  ActData_EXPORT virtual Standard_Boolean
    IsModified() const;

  ActData_EXPORT virtual Standard_Boolean
    IsSaved() const;

  ActData_EXPORT virtual ActAPI_IModel::VersionStatus
    GetVersionStatus() const;

  ActData_EXPORT Standard_Boolean
    IsInitialized() const;

  ActData_EXPORT Standard_Boolean
    IsUndefined() const;

// Transactions:
public:

  ActData_EXPORT virtual void
    DisableTransactions();

  ActData_EXPORT virtual void
    EnableTransactions();

  ActData_EXPORT virtual void
    OpenCommand();

  ActData_EXPORT virtual Standard_Boolean
    HasOpenCommand() const;

  ActData_EXPORT virtual void
    AbortCommand();

  ActData_EXPORT virtual void
    CommitCommand(ActAPI_TxData theData = 0);

  ActData_EXPORT virtual Handle(ActAPI_TxRes)
    Undo(const Standard_Integer theNbUndoes = 1);

  ActData_EXPORT virtual Standard_Integer
    NbUndos() const;

  ActData_EXPORT virtual Handle(ActAPI_TxRes)
    Redo(const Standard_Integer theNbRedoes = 1);

  ActData_EXPORT virtual Standard_Integer
    NbRedos() const;

  ActData_EXPORT virtual Handle(ActAPI_HNodeIdMap)
    GetModifiedNodes() const;

// Services for managing Document's structure:
public:

  ActData_EXPORT virtual Handle(ActAPI_IPartition)
    VariablePartition(const VariableType& theVarType) const;

  ActData_EXPORT virtual Handle(ActAPI_HPartitionList)
    Partitions() const;

  ActData_EXPORT virtual Handle(ActAPI_IPartition)
    Partition(const Standard_Integer theId) const;

  ActData_EXPORT virtual Handle(ActAPI_IPartition)
    Partition(const TCollection_AsciiString& theNodeType) const;

  ActData_EXPORT virtual Handle(ActAPI_INode)
    FindNode(const ActAPI_DataObjectId& theNodeId) const;

  ActData_EXPORT virtual Handle(ActAPI_INode)
    FindNodeByName(const TCollection_ExtendedString& theNodeName) const;

  ActData_EXPORT virtual Handle(ActAPI_HNodeList)
    FindNodesByName(const TCollection_ExtendedString& theNodeName) const;

  ActData_EXPORT virtual Handle(ActAPI_INode)
    FindNodeByNames(const std::vector<TCollection_ExtendedString>& theNodeNames) const;

  ActData_EXPORT virtual Handle(ActAPI_INode)
    GetRootNode() const;

  ActData_EXPORT virtual Standard_Boolean
    DeleteNode(const ActAPI_DataObjectId& theNodeId);

  ActData_EXPORT virtual Standard_Boolean
    DeleteNode(const Handle(ActAPI_INode)& theNode);

  ActData_EXPORT virtual Standard_Boolean
    CopyNode(const ActAPI_DataObjectId& theNodeId);

  ActData_EXPORT virtual Standard_Boolean
    CopyNode(const Handle(ActAPI_INode)& theNode);

  ActData_EXPORT virtual Handle(ActAPI_INode)
    PasteAsChild(const Handle(ActAPI_INode)& theParent);

  ActData_EXPORT virtual void
    RenameNode(const ActAPI_DataObjectId& theNodeId,
               const TCollection_AsciiString& theNewName);

  ActData_EXPORT virtual void
    RenameVariable(const ActAPI_DataObjectId& theNodeId,
                   const TCollection_AsciiString& theNewName,
                   const Standard_Boolean doFullSynchronization = Standard_False);

  ActData_EXPORT virtual ActAPI_DataObjectId
    AddVariable(const VariableType theVarType,
                const TCollection_AsciiString& theVarName);

// Accessors to raw CAF data & model sections
public:

  ActData_EXPORT virtual TDF_Label
    RootLabel() const;

  ActData_EXPORT virtual ActData_LogBook
    LogBook() const;

  //! Accessor for the underlying OCAF Document.
  //! \return OCAF Document.
  const Handle(TDocStd_Document)& Document() const
  {
    return m_doc;
  }

// Accessors for the internally used utilities:
public:

  ActData_EXPORT const Handle(ActData_TransactionEngine)&
    TransactionEngine() const;

  ActData_EXPORT const Handle(ActData_CopyPasteEngine)&
    CopyPasteEngine() const;

  ActData_EXPORT const Handle(ActData_FuncExecutionCtx)&
    FuncExecutionCtx() const;

// Tree Function mechanism:
public:

  ActData_EXPORT virtual Standard_Integer
    FuncExecuteAll(const Standard_Boolean doDetach = Standard_False,
                   ActAPI_TxData theData = 0);

  ActData_EXPORT virtual Handle(ActAPI_ITreeFunction)
    Function(const Standard_GUID& theFuncGUID) const;

  ActData_EXPORT virtual const Handle(HTreeFunctionMap)&
    Functions() const;

  ActData_EXPORT virtual void
    FuncSetProgressNotifier(const Handle(ActAPI_IProgressNotifier)& thePNotifier);

  ActData_EXPORT virtual void
    FuncSetPlotter(const Handle(ActAPI_IPlotter)& thePlotter);

  ActData_EXPORT virtual const Handle(ActAPI_IProgressNotifier)&
    FuncProgressNotifier() const;

  ActData_EXPORT virtual const Handle(ActAPI_IPlotter)&
    FuncPlotter() const;

  ActData_EXPORT virtual Standard_Boolean
    FuncIsProgressNotifierOn() const;

  ActData_EXPORT virtual Standard_Boolean
    FuncIsPlotterOn() const;

  ActData_EXPORT virtual void
    FuncProgressNotifierOn();

  ActData_EXPORT virtual void
    FuncPlotterOn();

  ActData_EXPORT virtual void
    FuncProgressNotifierOff();

  ActData_EXPORT virtual void
    FuncPlotterOff();

  ActData_EXPORT virtual void
    FuncReleaseLogBook();

  ActData_EXPORT virtual Standard_Integer
    FuncExecutionFlags() const;

  ActData_EXPORT virtual void
    FuncSetExecutionFlags(const Standard_Integer theFlags);

public:

  //! Default implementation of this method does not nothing but returns `true`.
  virtual Standard_Boolean FuncReconnectAll()
  {
    return Standard_True;
  }

// Construction:
protected:

  ActData_EXPORT
    ActData_BaseModel(const Standard_Boolean useExtTransactions = Standard_False);

// Construction internals:
protected:

  ActData_EXPORT void
    registerPartition(const Standard_Integer theTypeId,
                      const Handle(ActAPI_IPartition)& thePartition);

  ActData_EXPORT void
    registerTreeFunction(const Handle(ActAPI_ITreeFunction)& theTreeFunction);

  ActData_EXPORT void
    init(const Handle(TDocStd_Document)& theDoc);

  ActData_EXPORT static Handle(TDocStd_Document)
    newDocument();

// Tree Function internals:
protected:

  ActData_EXPORT void
    registerFunctionDriver(const Standard_GUID& theGUID,
                           const Handle(ActData_BaseTreeFunction)& theFunc);

// Structure management internals:
protected:

  ActData_EXPORT virtual void
    getInvariantCopyRefs(ActAPI_FuncGUIDStream& FuncGUIDs,
                         ActAPI_ParameterLocatorStream& Refs) const;

  ActData_EXPORT void
    doDeleteRecursive(const ActAPI_DataObjectId& theNodeId);

  ActData_EXPORT void
    chargeEvaluatorsWithVar(const Handle(ActData_BaseVarNode)& theVarNode);

// Versioning internals:
protected:

  ActData_EXPORT void
    bindVersionInfo();

  ActData_EXPORT Standard_Integer
    storedVersionFramework();

  ActData_EXPORT Standard_Integer
    storedVersionApp();

  ActData_EXPORT Standard_Integer
    actualVersionFramework();

  ActData_EXPORT virtual Handle(ActData_CAFConverter)
    converterFw();

// Copy & Paste internals:
private:

  TDF_Label
    accessCopyPasteSection(const Standard_Boolean toCreate = Standard_True);

  void
    releaseCopyPasteBuffer();

// Tree Function logging internals:
private:

  TDF_Label
    accessLogBookSection(const Standard_Boolean toCreate = Standard_True);

// Construction internals for descendant classes:
private:

  //! Fills the Data Model of a specific type with custom Partitions.
  virtual void
    initPartitions() = 0;

  //! Fills the Data Model of a specific type with custom Tree Functions.
  virtual void
    initFunctionDrivers() = 0;

// Structure management internals for descendant classes:
private:

  //! Allows particular implementation decide what Partition plays a role
  //! of Variable Nodes Partition for the given type. Variable Nodes
  //! Partition stores those Data Nodes which are involved as input arguments
  //! into expression evaluation mechanism.
  //! \param theVarType Variable type to return the dedicated Partition for.
  //! \return Variable Partition instance.
  virtual Handle(ActAPI_IPartition)
    getVariablePartition(const VariableType& theVarType) const = 0;

  //! Allows particular implementation decide what Node is a root one and
  //! return it.
  //! \return root Data Node.
  virtual Handle(ActAPI_INode)
    getRootNode() const = 0;

  //! Allows custom Data Model populate the passed collectons of references
  //! to pass out-scope filtering in Copy/Paste operation.
  //! \param FuncGUIDs [in/out] Function GUIDs to pass.
  //! \param Refs [in/out] Reference Parameters to pass.
  virtual void
    invariantCopyRefs(ActAPI_FuncGUIDStream& FuncGUIDs,
                      ActAPI_ParameterLocatorStream& Refs) const = 0;

// Versioning internals for descendant classes:
private:

  //! Returns version of the custom Application.
  //! \return version number.
  virtual Standard_Integer
    actualVersionApp() = 0;

  //! Callback for derived classes supplying CAF converter required to
  //! perform application-specific conversion of Data Model from older
  //! version of the application to the recent one.
  //! \return properly initialized CAF converter or NULL if you do not
  //!         need to have backward compatibility conversion in your
  //!         application.
  virtual Handle(ActData_CAFConverter)
    converterApp() = 0;

// Statuses:
private:

  //! Version status of the Data Model.
  VersionStatus m_versionStatus;

  //! Status of the Data Model.
  Standard_Integer m_status;

  //! Indicates whether the Simple (anonymous) Transaction Mode is
  //! used by the Data Model for the Transactional Scopes being deployed.
  Standard_Boolean m_bSimpleTxMode;

  //! Flags for fine-tuning Execution process of Tree Functions.
  Standard_Integer m_iFuncExecutionFlags;

// Data containers:
private:

  //! Registered Partitions.
  Handle(HPartitionMap) m_partitionMap;

  //! Registered Tree Functions
  Handle(HTreeFunctionMap) m_treeFunctionMap;

  //! CAF Document corresponding to the ACT Data Model.
  Handle(TDocStd_Document) m_doc;

  //! Root CAF label for the model.
  TDF_Label m_rootLabel;

// Engines & contexts:
private:

  //! Transaction Engine.
  Handle(ActData_TransactionEngine) m_trEngine;

  //! Copy/Paste tool.
  Handle(ActData_CopyPasteEngine) m_copyPasteEngine;

  //! Execution context for Tree Functions.
  Handle(ActData_FuncExecutionCtx) m_funcCtx;

};

#endif
