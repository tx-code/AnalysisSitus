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

#ifndef ActAPI_IModel_HeaderFile
#define ActAPI_IModel_HeaderFile

// Active Data (API) includes
#include <ActAPI_INode.h>
#include <ActAPI_IPartition.h>
#include <ActAPI_IPlotter.h>
#include <ActAPI_IProgressNotifier.h>
#include <ActAPI_TxData.h>
#include <ActAPI_TxRes.h>

// OCCT includes
#include <NCollection_DataMap.hxx>
#include <Standard_DefineHandle.hxx>
#include <Standard_GUID.hxx>
#include <Standard_Type.hxx>
#include <TCollection_AsciiString.hxx>

// Standard includes
#include <vector>

//! \ingroup AD_API
//!
// Active Data (API) forward declarations
class ActAPI_ITreeFunction;

//! \ingroup AD_API
//!
//! Type definition for map of registered Partitions.
typedef NCollection_DataMap<Standard_Integer,
                            Handle(ActAPI_IPartition)> PartitionMap;

//! \ingroup AD_API
//!
//! Type definition for map of registered Partitions operated by Handle.
typedef NCollection_Shared<PartitionMap> HPartitionMap;

//! \ingroup AD_API
//!
//! Type definition for map of registered Tree Functions.
typedef NCollection_DataMap<Standard_GUID,
                            Handle(ActAPI_ITreeFunction),
                            ActiveData::GuidHasher> TreeFunctionMap;

//! \ingroup AD_API
//!
//! Type definition for map of registered Tree Functions operated by Handle.
typedef NCollection_Shared<TreeFunctionMap> HTreeFunctionMap;

//-----------------------------------------------------------------------------
// Model
//-----------------------------------------------------------------------------

//! \ingroup AD_API
//!
//! Interface for ACT Data Models.
class ActAPI_IModel : public Standard_Transient
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActAPI_IModel, Standard_Transient)

public:

  //! Default destructor.
  ActData_EXPORT virtual
    ~ActAPI_IModel();

public:

  //! Built-in types of Variable Nodes.
  enum VariableType
  {
    Variable_Real = 1,
    Variable_Int,
    Variable_Bool
  };

  //! Version of the currently loaded Data Model.
  enum VersionStatus
  {
    Version_Undefined = 0,
    Version_NotBoundFail,
    Version_LessFail,
    Version_LessOk,
    Version_HigherFail,
    Version_Ok
  };

  //! Execution flags.
  enum FuncExecutionFlags
  {
    ExecFlags_NoFlags = 0,
    ExecFlags_ForceNoDetach = 1
  };

  //! Status of the Data Model regarding to the state of the underlying CAF
  //! Document. If such document is not bound, the status is UNDEFINED,
  //! otherwise -- INITIALIZED. If the Data Model has been affected, the
  //! status is switched to MODIFIED. If the Data Model has been saved to
  //! external file, the status gets SAVED value as well.
  enum Status
  {
    MS_Undefined   = 0x01,
    MS_Initialized = 0x02,
    MS_Modified    = 0x04,
    MS_Saved       = 0x08
  };

  //! Status for Tree Function execution mechanism. This status is used
  //! by FuncExecuteAll method in order to give the client code the
  //! return status.
  enum ExecutionStatus
  {
    Execution_Undefined     = 0x001, //!< Undefined.
    Execution_NoFunctions   = 0x002, //!< No functions were requested for execution.
    Execution_Done          = 0x004, //!< Execution has been done successfully.
    Execution_LoopsDetected = 0x008, //!< Dependency loops detected.
    Execution_Failed        = 0x010  //!< Execution failed due some unknown reason.
  };

// Auxiliary functions:
public:

  static Standard_Boolean IsExecutionDone(const Standard_Integer Status)
  {
    return (Status & Execution_Done) > 0;
  }

  static Standard_Boolean IsExecutionDoneWithoutErrors(const Standard_Integer Status)
  {
    const int status = ( (Status & Execution_Done) &&
                        !(Status & Execution_NoFunctions) &&
                        !(Status & Execution_LoopsDetected) );
    return status > 0;
  }

  static Standard_Boolean IsExecutionFailed(const Standard_Integer Status)
  {
    return (Status & Execution_Failed) > 0;
  }

  static Standard_Boolean IsExecutionInvalidGraph(const Standard_Integer Status)
  {
    return (Status & Execution_LoopsDetected) > 0;
  }

// Persistence:
public:

  //! Creates new empty Model and settles it down to the root Label
  //! of underlying OCCT CAF document.
  virtual Standard_Boolean
    NewEmpty() = 0;

  //! Opens Data Model contents from file.
  //! \param theFilename [in] filename.
  //! \param theNotifier [in] notifier.
  //! \return true in case of success, false -- otherwise.
  virtual Standard_Boolean
    Open(const TCollection_AsciiString& theFilename,
         ActAPI_ProgressEntry           theNotifier = nullptr) = 0;

  //! Releases the Data Model.
  //! \param theVersionStatus [in] ultimate version status to set.
  virtual void
    Release(const VersionStatus theVersionStatus = Version_Undefined) = 0;

  //! Saves Data Model into external file.
  //! \param theFilename [in] filename.
  //! \param theNotifier [in] notifier.
  //! \return true in case of success, false -- otherwise.
  virtual Standard_Boolean
    SaveAs(const TCollection_AsciiString& theFilename,
           ActAPI_ProgressEntry           theNotifier = nullptr) = 0;

  //! Returns true if the Model has been modified.
  //! \return true/false.
  virtual Standard_Boolean
    IsModified() const = 0;

  //! Returns true if the Model has been saved to a file at
  //! least once or has been loaded from a file.
  //! \return true/false.
  virtual Standard_Boolean
    IsSaved() const = 0;

  //! Returns true if the Model is in INITIALIZED status.
  //! \return true/false.
  virtual Standard_Boolean
    IsInitialized() const = 0;

  //! Returns true if the Model is in UNDEFINED status.
  //! \return true/false.
  virtual Standard_Boolean
    IsUndefined() const = 0;

// Transactions:
public:

  //! Disables transaction mechanism.
  virtual void
    DisableTransactions() = 0;

  //! Enables transaction mechanism (enabled initially by default).
  virtual void
    EnableTransactions() = 0;

  //! Starts new transaction.
  virtual void
    OpenCommand() = 0;

  //! Checks whether a transaction is already opened.
  //! \return true/false.
  virtual Standard_Boolean
    HasOpenCommand() const = 0;

  //! Aborts transaction.
  virtual void
    AbortCommand() = 0;

  //! Commits transaction.
  //! \param theData [in] optional application-specific data to bind to
  //!        transaction.
  virtual void
    CommitCommand(ActAPI_TxData theData = 0) = 0;

  //! Performs Undo action on the underlying CAF document.
  //! \param theNbUndoes [in] number of Undo operations to perform one-by-one.
  //! \return affected Parameters (including META ones).
  virtual Handle(ActAPI_TxRes)
    Undo(const Standard_Integer theNbUndoes = 1) = 0;

  //! Returns the number of currently available Undo deltas.
  //! \return number of available Undo deltas.
  virtual Standard_Integer
    NbUndos() const = 0;

  //! Performs Redo action on the underlying CAF document.
  //! \param theNbRedoes [in] number of Redo operations to perform one-by-one.
  //! \return affected Parameters (including META ones).
  virtual Handle(ActAPI_TxRes)
    Redo(const Standard_Integer theNbRedoes = 1) = 0;

  //! Returns the number of currently available Redo deltas.
  //! \return number of available Redo deltas.
  virtual Standard_Integer
    NbRedos() const = 0;

  //! \return IDs of the Data Nodes modified in the last transaction.
  virtual Handle(ActAPI_HNodeIdMap)
    GetModifiedNodes() const = 0;

// Structure services:
public:

  //! Creates a clone of Data Model instance. Normally, this cloned instance
  //! should represent new and empty Model.
  //! \return cloned instance.
  virtual Handle(ActAPI_IModel)
    Clone() const = 0;

  //! Accessor for dedicated Partition storing Data Nodes representing
  //! variables. Variable Nodes play as source for expression evaluation
  //! mechanism which is actually a specialization of Tree Function
  //! mechanism for auto-calculating Parameter values.
  //! \param theVarType [in] type of the Variable to return Partition for.
  //! \return Variable Partition instance.
  virtual Handle(ActAPI_IPartition)
    VariablePartition(const VariableType& theVarType) const = 0;

  //! Returns all registered Partitions in a plain list preserving their
  //! storage order.
  //! \return ordered collection of the registered Partitions.
  virtual Handle(ActAPI_HPartitionList)
    Partitions() const = 0;

  //! Returns Partition by its ID.
  //! \param theId [in] ID of the Partition.
  //! \return Partition instance.
  virtual Handle(ActAPI_IPartition)
    Partition(const Standard_Integer theId) const = 0;

  //! Returns Partition by the Node type.
  //! \param theNodeType [in] Node type.
  //! \return Partition instance.
  virtual Handle(ActAPI_IPartition)
    Partition(const TCollection_AsciiString& theNodeType) const = 0;

  //! Finds the Data Node with the passed ID.
  //! \param theNodeId [in] ID of the Node to find.
  //! \return Node instance.
  virtual Handle(ActAPI_INode)
    FindNode(const ActAPI_DataObjectId& theNodeId) const = 0;

  //! Finds Data Node by its name.
  //!
  //! \param theNodeName [in] name of the Node to find.
  //! \return found Node or NULL if nothing was found.
  virtual Handle(ActAPI_INode)
    FindNodeByName(const TCollection_ExtendedString& theNodeName) const = 0;

  //! Finds all Data Nodes which have the specified name.
  //!
  //! \param theNodeName [in] name of the Nodes to find.
  //! \return found Nodes or empty list if nothing was found.
  virtual Handle(ActAPI_HNodeList)
    FindNodesByName(const TCollection_ExtendedString& theNodeName) const = 0;

  //! Finds Data Node by its name and the names of its direct parents.
  //!
  //! \param theNodeNames [in] names of the parent Nodes and the target
  //!                          Node (the last item in the sequence).
  //! \return found Node or NULL if nothing was found.
  virtual Handle(ActAPI_INode)
    FindNodeByNames(const std::vector<TCollection_ExtendedString>& theNodeNames) const = 0;

  //! Returns root Node of the Model.
  //! \return root Node.
  virtual Handle(ActAPI_INode)
    GetRootNode() const = 0;

  //! Deletes the Data Node with the passed ID from the Data Model. The
  //! deletion rules are the followings:
  //! - Data Node is deleted with underlying child Nodes recursively;
  //! - If Data Node A (those being deleted) is a source for Data Node B, the
  //!   corresponding Tree Function Parameter is removed from B;
  //! \param theNodeId [in] ID of the Data Node to delete.
  //! \return true in case of success, false -- otherwise.
  virtual Standard_Boolean
    DeleteNode(const ActAPI_DataObjectId& theNodeId) = 0;

  //! Another form of deletion method.
  //! \param theNode [in] Data Node to delete.
  //! \return true in case of success, false -- otherwise.
  virtual Standard_Boolean
    DeleteNode(const Handle(ActAPI_INode)& theNode) = 0;

  //! Copies the given Data Node to the internal buffer.
  //! \param theNodeId [in] ID of the Data Node to copy.
  //! \return true in case of success, false -- otherwise.
  virtual Standard_Boolean
    CopyNode(const ActAPI_DataObjectId& theNodeId) = 0;

  //! Another form of copying method.
  //! \param theNode [in] Data Node to copy.
  //! \return true in case of success, false -- otherwise.
  virtual Standard_Boolean
    CopyNode(const Handle(ActAPI_INode)& theNode) = 0;

  //! Dual method for copying. Pastes the buffered Data Node as a child
  //! of the given one.
  //! \param theParent [in] parent Node to paste the buffered copy into.
  //! \return resulting copy.
  virtual Handle(ActAPI_INode)
    PasteAsChild(const Handle(ActAPI_INode)& theParent) = 0;

  //! Renames the Data Node with the passed ID. This is just a convenience
  //! method for ActAPI_INode::SetName.
  //! \param theNodeId [in] ID of the Data Node to delete.
  //! \param theNewName [in] new name to set for the Data Node.
  virtual void
    RenameNode(const ActAPI_DataObjectId& theNodeId,
               const TCollection_AsciiString& theNewName) = 0;

  // --------------------------------------------------------------------------
  // TODO: this method is going to become too heavy to be really efficient.
  // --------------------------------------------------------------------------
  //! Renames the Variable Node with the given ID. Unlike RenameNode method,
  //! this one takes special care of dependent Nodes. According to the value
  //! of doFullSynchronization flag, this routine either affects only
  //! directly connected Nodes (false -- default) by modifying their
  //! evaluation strings, or performs full traversing over the entire Data
  //! Model adjusting evaluation strings of each Parameter (true).
  //! \param theNodeId [in] ID of the Variable Data Node.
  //! \param theNewName [in] new name to set.
  //! \param doFullSynchronization [in] indicates whether the full traversal
  //!        of Data Model should be performed in order to provide actual
  //!        replacement of Variable Name in all existing evaluation strings.
  //!        The alternative is a partial update which affects only CONNECTED
  //!        Parameters. The latter approach is much less heavy and recommended
  //!        to use if you design your Data Model so that it cannot persist
  //!        invalid (disconnected) evaluation strings.
  virtual void
    RenameVariable(const ActAPI_DataObjectId& theNodeId,
                   const TCollection_AsciiString& theNewName,
                   const Standard_Boolean doFullSynchronization = Standard_False) = 0;

  virtual ActAPI_DataObjectId
    AddVariable(const VariableType theVarType,
                const TCollection_AsciiString& theVarName) = 0;

  virtual TDF_Label
    RootLabel() const = 0;

// Tree Function mechanism:
public:

  //! Executes the entire set of Tree Functions registered in the Model.
  //! \param doDetach [in] indicates whether execution routine must be
  //!        performed in the detached (working) thread.
  //! \param theData [in] optional TxData structure which allows to pass
  //!        transactional information to the working thread. Use this in
  //!        order to customize your threading pattern. E.g. you can specify
  //!        a callback to invoke once data is committed.
  //! \return true in case of success, false -- otherwise.
  virtual Standard_Integer
    FuncExecuteAll(const Standard_Boolean doDetach = Standard_False,
                   ActAPI_TxData theData = 0) = 0;

  //! Returns Tree Function instance by GUID.
  //! \param theFuncGUID [in] Tree Function GUID.
  //! \return Tree Function instance.
  virtual Handle(ActAPI_ITreeFunction)
    Function(const Standard_GUID& theFuncGUID) const = 0;

  //! Returns all registered Tree Functions.
  //! \return Tree Function map.
  virtual const Handle(HTreeFunctionMap)&
    Functions() const = 0;

  //! Sets Progress Notifier for execution context.
  //! \param thePNotifier [in] Progress Notifier to set.
  virtual void
    FuncSetProgressNotifier(const Handle(ActAPI_IProgressNotifier)& thePNotifier) = 0;

  //! Sets Imperative Plotter for execution context.
  //! \param thePlotter [in] Imperative Plotter to set.
  virtual void
    FuncSetPlotter(const Handle(ActAPI_IPlotter)& thePlotter) = 0;

  //! Returns global Progress Notifier.
  //! \return Progress Notifier instance.
  virtual const Handle(ActAPI_IProgressNotifier)&
    FuncProgressNotifier() const = 0;

  //! Returns global Imperative Plotter.
  //! \return Imperative Plotter instance.
  virtual const Handle(ActAPI_IPlotter)&
    FuncPlotter() const = 0;

  //! Returns true if Progress Notifier is ENABLED, false -- otherwise.
  //! \return true/false.
  virtual Standard_Boolean
    FuncIsProgressNotifierOn() const = 0;

  //! Returns true if Plotter is ENABLED, false -- otherwise.
  //! \return true/false.
  virtual Standard_Boolean
    FuncIsPlotterOn() const = 0;

  //! Sets Progress Notifier ENABLED.
  virtual void
    FuncProgressNotifierOn() = 0;

  //! Sets Plotter ENABLED.
  virtual void
    FuncPlotterOn() = 0;

  //! Sets Progress Notifier DISABLED.
  virtual void
    FuncProgressNotifierOff() = 0;

  //! Sets Plotter DISABLED.
  virtual void
    FuncPlotterOff() = 0;

  //! Releases Modification LogBook utilized by Tree Function mechanism.
  virtual void
    FuncReleaseLogBook() = 0;

  //! Get execution flags.
  //! \return bitwise summed execution flags.
  //! \sa ExecutionStatus.
  virtual Standard_Integer
    FuncExecutionFlags() const = 0;

  //! Set flags to configure Execution process for Tree Functions.
  //! \param theFlags [in] bitwise summed execution flags.
  //! \sa ExecutionStatus.
  virtual void
    FuncSetExecutionFlags(const Standard_Integer theFlags) = 0;

  //! This method allows reconnecting all Tree Functions in the Data Model.
  //! The method is designed to be invoked by the compatibility conversion
  //! routine.
  //! \return true in case of success, false -- otherwise.
  virtual Standard_Boolean
    FuncReconnectAll() = 0;

// Versioning:
public:

  //! Returns version status of the Model.
  //! \return version status.
  virtual VersionStatus
    GetVersionStatus() const = 0;

};

#endif
