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

#ifndef ActData_CopyPasteEngine_HeaderFile
#define ActData_CopyPasteEngine_HeaderFile

// Active Data includes
#include <ActData_BaseNode.h>
#include <ActData_Common.h>
#include <ActData_RefClassifier.h>
#include <ActData_SamplerTreeNode.h>
#include <ActData_Utils.h>

// Active Data (API) includes
#include <ActAPI_IModel.h>
#include <ActAPI_INode.h>
#include <ActAPI_ITreeFunction.h>

// OCCT includes
#include <NCollection_DoubleMap.hxx>
#include <NCollection_Map.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_Label.hxx>
#include <TDF_LabelMapHasher.hxx>

DEFINE_STANDARD_HANDLE(ActData_CopyPasteEngine, Standard_Transient)

//! \ingroup AD_DF
//!
//! Copy/Paste functionality provider.
class ActData_CopyPasteEngine : public Standard_Transient
{
public:

  DEFINE_STANDARD_RTTI_INLINE(ActData_CopyPasteEngine, Standard_Transient)

public:

  //! Engine status. Has a meaning of last error code.
  enum StatusCode
  {
    Status_NoError              = 0x00001, //!< Copy/Paste Engine raised no error.
    Status_WarnNullFuncArgument = 0x00002, //!< Null Tree Function argument Parameter detected.
    Status_WarnNullFuncResult   = 0x00004  //!< Null Tree Function result Parameter detected.
  };

  //! Accessor for Engine status.
  //! \return status.
  inline Standard_Integer Status() const
  {
    return m_status;
  }

public:

  //! Correspondence map between source Labels and the target ones.
  typedef NCollection_DoubleMap<TDF_Label, TDF_Label,
                                TDF_LabelMapHasher, TDF_LabelMapHasher> RelocationTable;

public:

  //! Auxiliary class which can be used for customization of reference
  //! filtering rules.
  class ReferenceFilter
  {
  public:

    //! Filter for Tree Function references.
    class TreeFunctionFilter
    {
    public:

      //! Cleans up the filter.
      void Clear()
      {
        m_outScopedToPass.Clear();
      }

      //! Configures reference filter to pass out-scoped Tree Functions
      //! of the given type (specified by GUID).
      //! \param GUID [in] Tree Function GUID.
      void PassOutScoped(const Standard_GUID& GUID)
      {
        m_outScopedToPass.Add(GUID);
      }

      //! Returns true if the given Tree Function type (GUID) is registered
      //! as passing out-scoped filtering.
      //! \param GUID [in] Tree Function GUID.
      //! \return true/false.
      Standard_Boolean IsPassingOutScoped(const Standard_GUID& GUID) const
      {
        return m_outScopedToPass.Contains(GUID);
      }

    private:

      //! Map of Tree Function GUIDs to pass through out-scoped filtering.
      NCollection_Map<Standard_GUID, ActiveData::GuidHasher> m_outScopedToPass;

    };

    //! Filter for Reference Parameters.
    class RefParamFilter
    {
    public:

      //! Cleans up the filter.
      void Clear()
      {
        m_outScopedToPass.Clear();
      }

      //! Configures reference filter to pass out-scoped Reference Parameters
      //! with the given relative ID for the given type of Data Node.
      //! \param NodeType [in] Node type.
      //! \param ParamID [in] ID of the Reference Parameter.
      void PassOutScoped(const TCollection_AsciiString& NodeType,
                         const Standard_Integer         ParamID)
      {
        if ( !m_outScopedToPass.IsBound(NodeType) )
          m_outScopedToPass.Bind( NodeType, ParamIDCol() );

        ParamIDCol& IDMap = m_outScopedToPass.ChangeFind(NodeType);
        IDMap.Add(ParamID);
      }

      //! Returns true if the given type of Reference Parameter is registered
      //! as passing out-scoped filtering.
      //! \param NodeType [in] Node type.
      //! \param ParamID [in] ID of the Reference Parameter.
      //! \return true/false.
      Standard_Boolean IsPassingOutScoped(const TCollection_AsciiString& NodeType,
                                          const Standard_Integer         ParamID) const
      {
        if ( !m_outScopedToPass.IsBound(NodeType) )
          return Standard_False;

        return m_outScopedToPass.Find(NodeType).Contains(ParamID);
      }

    private:

      //! Type definition for a collection of Parameter relative IDs.
      typedef NCollection_Map<Standard_Integer> ParamIDCol;

      //! Type definition for map of Reference Parameters to pass through
      //! out-scoped filtering.
      typedef NCollection_DataMap<TCollection_AsciiString, ParamIDCol> OutScopedMap;

    private:

      //! Map of Reference Parameters to pass through out-scoped filtering.
      OutScopedMap m_outScopedToPass;

    };

  public:

    //! Cleans up the filter.
      void Clear()
      {
        m_treeFunctionFilter.Clear();
        m_refParamFilter.Clear();
      }

    //! Charges the Reference Filter with reference identifiers to pass.
    //! \param FuncGUIDs [in] Tree Function types to pass.
    //! \param Refs [in] Reference Parameter Locators to pass.
    void Load(const ActAPI_FuncGUIDStream&         FuncGUIDs,
              const ActAPI_ParameterLocatorStream& Refs)
    {
      this->Clear();

      for ( ActAPI_FuncGUIDList::Iterator it( *FuncGUIDs.List.operator->() ); it.More(); it.Next() )
        m_treeFunctionFilter.PassOutScoped( it.Value() );

      for ( ActAPI_ParameterLocatorList::Iterator it( *Refs.List.operator->() ); it.More(); it.Next() )
      {
        const ActAPI_ParameterLocator& PLoc = it.Value();
        m_refParamFilter.PassOutScoped(PLoc.NodeType, PLoc.PID);
      }
    }

    //! Accessor for Tree Function filter.
    //! \return Tree Function filter.
    TreeFunctionFilter& AccessTreeFunctionFilter()
    {
      return m_treeFunctionFilter;
    }

    //! Accessor for Reference Parameter filter.
    //! \return Reference Parameter Filter.
    RefParamFilter& AccessRefParamFilter()
    {
      return m_refParamFilter;
    }

  private:

    //! Filter for Tree Functions.
    TreeFunctionFilter m_treeFunctionFilter;

    //! Filter for Reference Parameters.
    RefParamFilter m_refParamFilter;

  };

public:

  ActData_EXPORT
    ActData_CopyPasteEngine();

  ActData_EXPORT
    ActData_CopyPasteEngine(const Handle(ActAPI_IModel)& Model);

public:

  //! Copying direction.
  enum CopyDirection
  {
    Direction_ToBuffer = 1, //!< From initial source to buffer.
    Direction_FromBuffer    //!< Distribute by Partitions.
  };

public:

  ActData_EXPORT void
    Init(const Handle(ActAPI_IModel)& Model);

  ActData_EXPORT Standard_Boolean
    TransferToBuffer(const Handle(ActAPI_INode)& Source);

  ActData_EXPORT Handle(ActAPI_INode)
    RestoreFromBuffer();

  ActData_EXPORT const Handle(ActAPI_IModel)&
    GetModel() const;

  ActData_EXPORT void
    SetModel(const Handle(ActAPI_IModel)& theModel);

  ActData_EXPORT TDF_Label
    GetBufferHead() const;

  ActData_EXPORT Handle(ActAPI_INode)
    GetRootBuffered() const;

  ActData_EXPORT const RelocationTable&
    GetRelocationTable(const Standard_Boolean isFirstStage) const;

  ActData_EXPORT const ActData_SamplerTreeNode&
    GetSamplerTree() const;

  ActData_EXPORT ReferenceFilter&
    AccessReferenceFilter();

// Global options:
public:

  ActData_EXPORT static void
    SetSuffixOptionNone();

  ActData_EXPORT static void
    SetSuffixOptionForRoot();

  ActData_EXPORT static void
    SetSuffixOptionAll();

// Main steps:
private:

  Standard_Boolean transfer(const Handle(ActAPI_INode)& theSource,
                            const CopyDirection         theDirection);

  void flattenTree(const Handle(ActAPI_INode)& theRoot,
                   ActData_SamplerTreeNode&    theSTree,
                   const CopyDirection         theDirection,
                   const Standard_Boolean      doUseSuffix);

  void rebuildTreeLinks(const CopyDirection theDirection);

  void normalizeBackReferences(const Handle(ActAPI_INode)& theNode,
                               const CopyDirection         theDirection);

  void normalizeDirectReferences(const Handle(ActAPI_INode)& theRoot,
                                 const CopyDirection         theDirection);

private:

  void normalizeObservers(const Handle(ActData_BaseNode)&      theNode,
                          const ActData_BaseNode::ObserverType theObserverType,
                          const CopyDirection                  theDirection);

  void normalizeTreeFunction(const Handle(ActData_BaseNode)& theNode,
                             const Standard_Integer          theTFuncID,
                             const Standard_Boolean          isInternal,
                             const CopyDirection             theDirection);

  void normalizeReference(const Handle(ActData_BaseNode)& theNode,
                          const Standard_Integer          theRefID,
                          const CopyDirection             theDirection);

  void normalizeReferenceList(const Handle(ActData_BaseNode)& theNode,
                              const Standard_Integer          theRefID,
                              const CopyDirection             theDirection);

  Handle(ActAPI_INode)
    getCopyBySampleID(const ActAPI_DataObjectId& theSampleID,
                      const CopyDirection        theDirection) const;

  TDF_Label copyNode(const Handle(ActAPI_INode)& theNode,
                     const CopyDirection         theDirection,
                     const Standard_Boolean      doUseSuffix);

private:

  //! Additional options concerning copy/paste string suffixes to apply.
  enum SuffixOptions
  {
    Option_SuffixNone,
    Option_SuffixForRoot,
    Option_SuffixAll
  };

private:

  //! Data Model instance.
  Handle(ActAPI_IModel) m_model;

  //! Sampler Tree.
  ActData_SamplerTreeNode m_sTree;

  //! Relocation Table.
  RelocationTable m_relocTable[2];

  //! Reference filter.
  ReferenceFilter m_refFilter;

  //! Current status of Engine.
  StatusCode m_status;

// Global options:
private:

  static SuffixOptions _suffixOption; //!< Option for copy/paste suffixes.

};

#endif
