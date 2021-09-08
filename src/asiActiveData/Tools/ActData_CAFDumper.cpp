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
#include <ActData_CAFDumper.h>

// Active Data includes
#include <ActData_ParameterFactory.h>
#include <ActData_Utils.h>

// OCCT includes
#include <OSD_OpenMode.hxx>
#include <OSD_Path.hxx>
#include <OSD_Protection.hxx>
#include <TColStd_MapIteratorOfPackedMapOfInteger.hxx>
#include <TColStd_PackedMapOfInteger.hxx>
#include <TDataStd_AsciiString.hxx>
#include <TDataStd_BooleanArray.hxx>
#include <TDataStd_ExtStringArray.hxx>
#include <TDataStd_Integer.hxx>
#include <TDataStd_Name.hxx>
#include <TDataStd_Real.hxx>
#include <TDF_AttributeIterator.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_ListIteratorOfLabelList.hxx>
#include <TDF_Reference.hxx>
#include <TNaming_NamedShape.hxx>

//-----------------------------------------------------------------------------
// Used definitions
//-----------------------------------------------------------------------------

#define DELIMIT_LINE \
  "===================================================================================================="

#define DELIMIT_LINE_SHORT1 \
  "    **************************************************************************************"

#define DELIMIT_LINE_SHORT2 \
  "    -------------------------------------------------------------"

#define HEADER_INFO_VERSION_FW(Version) \
  TCollection_AsciiString("ACT Data Model dump. Bound FRAMEWORK version: ").Cat(Version).Cat("\n")

#define HEADER_INFO_VERSION_APP(Version) \
  TCollection_AsciiString("ACT Data Model dump. Bound APPLICATION version: ").Cat(Version).Cat("\n")

#define PARTITION_INFO(Type, RootEntry) \
  TCollection_AsciiString(DELIMIT_LINE).Cat("\n") \
                     .Cat("+-> [").Cat(RootEntry).Cat("] PARTITION").Cat(" [").Cat(Type).Cat("]\n")

#define NODE_INFO(Name, Type, RootEntry, IdentLeft) \
  TCollection_AsciiString(IdentLeft).Cat(DELIMIT_LINE_SHORT1).Cat("\n") \
                     .Cat(IdentLeft).Cat("+-> [").Cat(RootEntry).Cat("] NODE").Cat(" [").Cat(Type).Cat("]").Cat(" - ").Cat(Name).Cat("\n") \
                     .Cat(IdentLeft).Cat(DELIMIT_LINE_SHORT1).Cat("\n")

#define BAD_NODE_INFO(RootEntry, IdentLeft) \
  TCollection_AsciiString(IdentLeft).Cat(DELIMIT_LINE_SHORT1).Cat("\n") \
                     .Cat(IdentLeft).Cat("+-> [").Cat(RootEntry).Cat("] ### BAD NODE (Cannot settle down)").Cat("\n") \
                     .Cat(IdentLeft).Cat(DELIMIT_LINE_SHORT1).Cat("\n")

#define PARAMETER_INFO(Type, RootEntry, IdentLeft) \
  TCollection_AsciiString(IdentLeft).Cat(DELIMIT_LINE_SHORT2).Cat("\n") \
                     .Cat(IdentLeft).Cat("     [").Cat(RootEntry).Cat("] PARAMETER").Cat(" [").Cat(Type).Cat("]\n") \
                     .Cat(IdentLeft).Cat(DELIMIT_LINE_SHORT2).Cat("\n")

#define LABEL_INFO(Entry, IdentLeft) \
  TCollection_AsciiString(IdentLeft).Cat("L >> [").Cat(Entry).Cat("]\n")

#define ATTRIBUTE_INFO(Type, Value, IdentLeft) \
  TCollection_AsciiString(IdentLeft).Cat("A >> [").Cat(Type).Cat("] - ").Cat(Value).Cat("\n")

#define PREFIX_PARAM_TYPE     "{Type}"
#define PREFIX_PARAM_NAME     "{Name}"
#define PREFIX_PARAM_SID      "{SID}"
#define PREFIX_PARAM_EVALSTR  "{Eval}"
#define PREFIX_PARAM_MTIME    "{MTime}"
#define PREFIX_PARAM_VALIDITY "{Validity}"
#define PREFIX_PARAM_UFLAGS   "{UFlags}"
#define PREFIX_PARAM_PENDING  "{Pending}"

//-----------------------------------------------------------------------------
// Dumping
//-----------------------------------------------------------------------------

//! Dumps the contents of the passed Data Model to the file with the given
//! name.
//! \param theFilename    [in] filename.
//! \param theModel       [in] Data Model instance to dump.
//! \param theContentType [in] type of the content to dump.
//! \param theVerbosity   [in] dumping verbosity level.
//! \return true in case of success, false -- otherwise.
Standard_Boolean
  ActData_CAFDumper::Dump(const TCollection_AsciiString& theFilename,
                          const Handle(ActAPI_IModel)&   theModel,
                          const Content                  theContentType,
                          const Verbosity                theVerbosity)
{
  /* =====================
   *  Check prerequisites
   * ===================== */

  Handle(ActData_BaseModel) M = Handle(ActData_BaseModel)::DownCast(theModel);
  if ( M.IsNull() )
    return Standard_False;

  TCollection_AsciiString Buff;

  /* =======================
   *  Open file for dumping
   * ======================= */

  std::ofstream FILE( theFilename.ToCString() );
  if ( !FILE.is_open() )
    return Standard_False;

  /* ===========
   *  Dump data
   * =========== */

  if ( !Dump(FILE, theModel, theContentType, theVerbosity) )
    return Standard_False;

  /* ============
   *  Close file
   * ============ */

  FILE.close();

  return Standard_True;
}

//! Dumps the contents of the passed Data Model to the given output stream.
//! \param theOut         [in] output stream.
//! \param theModel       [in] Data Model instance to dump.
//! \param theContentType [in] type of the content to dump.
//! \param theVerbosity   [in] dumping verbosity level.
//! \return true in case of success, false -- otherwise.
Standard_Boolean
  ActData_CAFDumper::Dump(Standard_OStream&            theOut,
                          const Handle(ActAPI_IModel)& theModel,
                          const Content                theContentType,
                          const Verbosity              theVerbosity)
{
  /* =====================
   *  Check prerequisites
   * ===================== */

  Handle(ActData_BaseModel) M = Handle(ActData_BaseModel)::DownCast(theModel);
  if ( M.IsNull() )
    return Standard_False;

  TCollection_AsciiString Buff;

  /* =========================
   *  Dump header information
   * ========================= */

  // FRAMEWORK version
  Standard_Integer aVerFw = M->storedVersionFramework();
  Standard_Integer aVerFwMajor = ( aVerFw >> 16 ) & 0xff;
  Standard_Integer aVerFwMinor = ( aVerFw >> 8  ) & 0xff;
  Standard_Integer aVerFwPatch = ( aVerFw       ) & 0xff;
  TCollection_AsciiString aVerFwStr = TCollection_AsciiString(aVerFwMajor)
                                                 .Cat(".").Cat(aVerFwMinor)
                                                 .Cat(".").Cat(aVerFwPatch);
  Buff = HEADER_INFO_VERSION_FW(aVerFwStr);
  theOut << Buff.ToCString();

  // APPLICATION version
  Standard_Integer aVerApp = M->storedVersionApp();
  Standard_Integer aVerAppMajor = ( aVerApp >> 16 ) & 0xff;
  Standard_Integer aVerAppMinor = ( aVerApp >> 8  ) & 0xff;
  Standard_Integer aVerAppPatch = ( aVerApp       ) & 0xff;
  TCollection_AsciiString aVerAppStr = TCollection_AsciiString(aVerAppMajor)
                                                 .Cat(".").Cat(aVerAppMinor)
                                                 .Cat(".").Cat(aVerAppPatch);
  Buff = HEADER_INFO_VERSION_APP(aVerAppStr);
  theOut << Buff.ToCString();

  /* ==================================================
   *  Iterate over the Data Model dumping its contents
   * ================================================== */

  // -----------------------
  // --> VERSIONING section
  // -----------------------

  if ( theVerbosity == Verbosity_Details ||
       theVerbosity == Verbosity_DetailsSkipUnstable )
  {
    dumpLabelWithAttributes(theOut, M->m_rootLabel, 0,
                            Standard_False, theContentType, theVerbosity);
    dumpLabelWithAttributes(theOut, M->m_rootLabel.FindChild(ActData_BaseModel::StructureTag_Version, Standard_False), 0,
                            Standard_True, theContentType, theVerbosity);
  }

  // ------------------------
  // --> DOMAIN DATA section
  // ------------------------

  if ( theContentType == Content_Plain )
    dumpPlainStructure(theOut, M, theVerbosity);
  else if ( theContentType == Content_TreeNodes )
    dumpTreeNodeStructure(theOut, M->GetRootNode(), theVerbosity, 0);
  else if ( theContentType == Content_DependencyGraph )
    Standard_ProgramError::Raise("TODO: Not implemented: Content_DependencyGraph");

  // -----------------------------------
  // --> COPY & PASTE Buffering section
  // -----------------------------------

  Buff = TCollection_AsciiString(DELIMIT_LINE).Cat("\n");
  theOut << Buff.ToCString();

  if ( theContentType == Content_Plain )
    dumpLabelWithAttributes(theOut, M->accessCopyPasteSection(Standard_False), 0,
                            Standard_True, theContentType, theVerbosity);
  else if ( theContentType == Content_TreeNodes )
  {
    TDF_Label aCPSection = M->accessCopyPasteSection(Standard_False);
    if ( !aCPSection.IsNull() )
    {
      TDF_Label aBuffRoot = aCPSection.FindChild(1, Standard_False);
      if ( !aBuffRoot.IsNull() )
      {
        TDF_Label aNodeRoot = aBuffRoot.FindChild(1, Standard_False);
        Handle(ActAPI_INode) aFirstNode = ActData_NodeFactory::NodeSettle(aNodeRoot);
        dumpTreeNodeStructure(theOut, aFirstNode, theVerbosity, 0);
      }
    }
  }
  else if ( theContentType == Content_DependencyGraph )
    Standard_ProgramError::Raise("TODO: Not implemented: Content_DependencyGraph");

  // ----------------------------------
  // --> Tree Function LogBook section
  // ----------------------------------

  Buff = TCollection_AsciiString(DELIMIT_LINE).Cat("\n");
  theOut << Buff.ToCString();

  if ( theContentType == Content_Plain )
    dumpLabelWithAttributes(theOut, M->accessLogBookSection(Standard_False), 0,
                            Standard_True, theContentType, theVerbosity);

  return Standard_True;
}

//! Dumps the contents of the passed Data Node to the given output stream.
//! \param theOut         [in] output stream.
//! \param theModel       [in] Data Model instance.
//! \param theNode        [in] Data Node to dump.
//! \param theVerbosity   [in] dumping verbosity level.
//! \return true in case of success, false -- otherwise.
Standard_Boolean
  ActData_CAFDumper::Dump(Standard_OStream&            theOut,
                          const Handle(ActAPI_IModel)& theModel,
                          const Handle(ActAPI_INode)&  theNode,
                          const Verbosity              theVerbosity)
{
  // Contract check.
  Handle(ActData_BaseModel) M = Handle(ActData_BaseModel)::DownCast(theModel);
  //
  if ( M.IsNull() )
    return Standard_False;

  // Dump Node.
  dumpTreeNodeStructure(theOut, theNode, theVerbosity, 0);

  return Standard_True;
}

//! Dumps the plain structure of the Data Model.
//! \param theOut       [out] output stream.
//! \param theBaseModel [in]  Data Model instance.
//! \param theVerbosity [in]  dumping verbosity level.
void
  ActData_CAFDumper::dumpPlainStructure(Standard_OStream&                theOut,
                                        const Handle(ActData_BaseModel)& theBaseModel,
                                        const Verbosity                  theVerbosity)
{
  // Iterate over all registered Partitions
  PartitionMap::Iterator aPartIt( *theBaseModel->m_partitionMap.operator->() );
  for ( ; aPartIt.More(); aPartIt.Next() )
  {
    // Access Partition
    Handle(ActData_BasePartition) P = Handle(ActData_BasePartition)::DownCast( aPartIt.Value() );
    dumpPartition(theOut, P, Content_Plain, theVerbosity);

    // Iterate over the Nodes in their persistent order
    ActData_BasePartition::Iterator aNodeIt(P);
    for ( ; aNodeIt.More(); aNodeIt.Next() )
    {
      Handle(ActData_BaseNode) N = Handle(ActData_BaseNode)::DownCast( aNodeIt.Value() );
      if ( !N.IsNull() )
        dumpNode(theOut, N, Content_Plain, theVerbosity);
    }
  }
}

//! Recursively dumps the alternative user-defined structure of the Data Model
//! built by means of Tree Node connectivity established between Data Nodes.
//! \param theOut               [out] output stream.
//! \param theRootNode          [in]  root Node to dump.
//! \param theVerbosity         [in]  dumping verbosity level.
//! \param theNbSpacesOnTheLeft [in]  some margin spaces for readability.
void
  ActData_CAFDumper::dumpTreeNodeStructure(Standard_OStream&           theOut,
                                           const Handle(ActAPI_INode)& theRootNode,
                                           const Verbosity             theVerbosity,
                                           const Standard_Integer      theNbSpacesOnTheLeft)
{
  Handle(ActData_BaseNode) N = Handle(ActData_BaseNode)::DownCast(theRootNode);
  if ( N.IsNull() )
    return;

  // Dump the Data Node itself
  dumpNode(theOut, N, Content_Plain, theVerbosity, theNbSpacesOnTheLeft);
  
  // Dump the child Data Nodes recursively
  Handle(ActAPI_IChildIterator) aChildIt = N->GetChildIterator();
  for ( ; aChildIt->More(); aChildIt->Next() )
  {
    Handle(ActAPI_INode) aChildNode = aChildIt->Value();
    dumpTreeNodeStructure(theOut, aChildNode, theVerbosity,
                          theNbSpacesOnTheLeft + DEFAULT_IDENT_LEFT * 3);
  }
}

//! Dumps the passed Partition.
//! \param theOut         [out] output stream.
//! \param thePartition   [in]  Partition to dump.
//! \param theContentType [in]  type of the content to dump.
//! \param theVerbosity   [in]  dumping verbosity level.
void
  ActData_CAFDumper::dumpPartition(Standard_OStream&                    theOut,
                                   const Handle(ActData_BasePartition)& thePartition,
                                   const Content                        theContentType,
                                   const Verbosity                      theVerbosity)
{
  TCollection_AsciiString aPType = thePartition->DynamicType()->Name();
  TCollection_AsciiString aPRootEntry = ActData_Utils::GetEntry(thePartition->m_label);
  TCollection_AsciiString Buff = PARTITION_INFO(aPType, aPRootEntry);
  theOut << Buff.ToCString();

  dumpLabelWithAttributes(theOut, thePartition->m_label, DEFAULT_IDENT_LEFT,
                          Standard_False, theContentType, theVerbosity);
}

//! Dumps the header information for the passed Node.
//! \param theOut               [out] output stream.
//! \param theNode              [in]  Node to dump.
//! \param theVerbosity         [in]  dumping verbosity level.
//! \param theNbSpacesOnTheLeft [in]  some left margin for readability.
void
  ActData_CAFDumper::dumpNodeHeader(Standard_OStream&           theOut,
                                    const Handle(ActAPI_INode)& theNode,
                                    const Verbosity             theVerbosity,
                                    const Standard_Integer      theNbSpacesOnTheLeft)
{
  TCollection_AsciiString aName = theNode->GetName();
  TCollection_AsciiString aPType = theNode->DynamicType()->Name();
  TCollection_AsciiString aPRootEntry = ActData_Utils::GetEntry( theNode->RootLabel() );

  TCollection_AsciiString IDENT;
  if ( theVerbosity == Verbosity_StructureNodesOnly ||
       theVerbosity == Verbosity_StructureOnly )
    IDENT = emptySpaces(theNbSpacesOnTheLeft);

  TCollection_AsciiString Buff = NODE_INFO(aName, aPType, aPRootEntry, IDENT);
  theOut << Buff.ToCString();
}

//! Dumps the header information for bad Node.
//! \param theOut               [out] output stream.
//! \param theNodeLab           [in]  host Label.
//! \param theVerbosity         [in]  dumping verbosity level.
//! \param theNbSpacesOnTheLeft [in]  some left margin for readability.
void
  ActData_CAFDumper::dumpBadNodeHeader(Standard_OStream&      theOut,
                                       const TDF_Label&       theNodeLab,
                                       const Verbosity        theVerbosity,
                                       const Standard_Integer theNbSpacesOnTheLeft)
{
  TCollection_AsciiString aPRootEntry = ActData_Utils::GetEntry(theNodeLab);

  TCollection_AsciiString IDENT;
  if ( theVerbosity == Verbosity_StructureNodesOnly ||
       theVerbosity == Verbosity_StructureOnly )
    IDENT = emptySpaces(theNbSpacesOnTheLeft);

  TCollection_AsciiString Buff = BAD_NODE_INFO(aPRootEntry, IDENT);
  theOut << Buff.ToCString();
}

//! Dumps the passed Node.
//! \param theOut               [out] output stream.
//! \param theNode              [in]  Node to dump.
//! \param theContentType       [in]  type of the content to dump.
//! \param theVerbosity         [in]  dumping verbosity level.
//! \param theNbSpacesOnTheLeft [in]  some left margin for readability.
void
  ActData_CAFDumper::dumpNode(Standard_OStream&               theOut,
                              const Handle(ActData_BaseNode)& theNode,
                              const Content                   theContentType,
                              const Verbosity                 theVerbosity,
                              const Standard_Integer          theNbSpacesOnTheLeft)
{
  dumpLabelWithAttributes(theOut, theNode->m_label, theNbSpacesOnTheLeft,
                          Standard_True, theContentType, theVerbosity);
}

//! Dumps the passed CAF Label along with its Attributes.
//! \param theOut               [out] output stream.
//! \param theLabel             [in]  CAF Label to dump.
//! \param doDumpChildren       [in]  indicates whether to dump child Labels recursively.
//! \param theNbSpacesOnTheLeft [in]  decoration indentation of the left.
//! \param theContentType       [in]  type of the content to dump.
//! \param theVerbosity         [in]  dumping verbosity level.
//! \param thePrefix            [in]  optional prefix to use.
void
  ActData_CAFDumper::dumpLabelWithAttributes(Standard_OStream&              theOut,
                                             const TDF_Label&               theLabel,
                                             const Standard_Integer         theNbSpacesOnTheLeft,
                                             const Standard_Boolean         doDumpChildren,
                                             const Content                  theContentType,
                                             const Verbosity                theVerbosity,
                                             const TCollection_AsciiString& thePrefix)
{
  if ( theLabel.IsNull() )
    return;

  TCollection_AsciiString aMarginLeft = emptySpaces(theNbSpacesOnTheLeft);
  TCollection_AsciiString aLabEntry = ActData_Utils::GetEntry(theLabel);

  if ( theVerbosity == Verbosity_Details ||
       theVerbosity == Verbosity_DetailsSkipUnstable )
  {
    TCollection_AsciiString Buff = LABEL_INFO(aLabEntry, aMarginLeft);
    theOut << Buff.ToCString();
  }

  // Check whether this Label represents a Node. If so, dump its header
  if ( ActData_NodeFactory::IsNode(theLabel) )
  {
    if ( !ActData_NodeFactory::CanSettleNode(theLabel) )
      dumpBadNodeHeader(theOut, theLabel, theVerbosity, theNbSpacesOnTheLeft);
    else
    {
      Handle(ActAPI_INode) aNode = ActData_NodeFactory::NodeSettle(theLabel);
      dumpNodeHeader(theOut, aNode, theVerbosity, theNbSpacesOnTheLeft);
    }
  }

  // Check whether this Label represents a Parameter. If so, dump it
  Standard_Boolean isParameter = ActData_ParameterFactory::IsUserParameter(theLabel);
  if ( isParameter &&
       theVerbosity > Verbosity_StructureNodesOnly )
  {
    Standard_Boolean isUndefinedType;
    Handle(ActAPI_IUserParameter) aParam = ActData_ParameterFactory::NewParameterSettle(theLabel, isUndefinedType);
    //
    if ( !aParam.IsNull() && aParam->IsWellFormed() )
    {
      TCollection_AsciiString aParamType = aParam->DynamicType()->Name();
      TCollection_AsciiString Buff = PARAMETER_INFO(aParamType, aLabEntry, aMarginLeft);
      theOut << Buff.ToCString();
    }
  }

  if ( theVerbosity == Verbosity_Details ||
       theVerbosity == Verbosity_DetailsSkipUnstable )
  {
    // Iterate over the Attributes
    for ( TDF_AttributeIterator attIt(theLabel); attIt.More(); attIt.Next() )
    {
      Handle(TDF_Attribute) A = attIt.Value();
      dumpAttribute(theOut, A, theNbSpacesOnTheLeft, theContentType, theVerbosity, thePrefix);
    }
  }

  if ( doDumpChildren )
  {
    // Iterate over the sub-Labels
    for ( TDF_ChildIterator labIt(theLabel, Standard_False); labIt.More(); labIt.Next() )
    {
      const TDF_Label L = labIt.Value();
      TCollection_AsciiString aPrefix;
      if ( isParameter )
      {
        switch ( L.Tag() )
        {
          case ActData_UserParameter::DS_ParamType:
            aPrefix = PREFIX_PARAM_TYPE;
            break;
          case ActData_UserParameter::DS_Name:
            aPrefix = PREFIX_PARAM_NAME;
            break;
          case ActData_UserParameter::DS_SemanticId:
            aPrefix = PREFIX_PARAM_SID;
            break;
          case ActData_UserParameter::DS_EvalString:
            aPrefix = PREFIX_PARAM_EVALSTR;
            break;
          case ActData_UserParameter::DS_MTime:
            aPrefix = PREFIX_PARAM_MTIME;
            break;
          case ActData_UserParameter::DS_IsValid:
            aPrefix = PREFIX_PARAM_VALIDITY;
            break;
          case ActData_UserParameter::DS_UserFlags:
            aPrefix = PREFIX_PARAM_UFLAGS;
            break;
          case ActData_UserParameter::DS_IsPending:
            aPrefix = PREFIX_PARAM_PENDING;
            break;
          default:
            break;
        }
      }
      if ( theVerbosity == Verbosity_DetailsSkipUnstable &&
           aPrefix == PREFIX_PARAM_MTIME )
      {
        TCollection_AsciiString Buff;
        Buff += emptySpaces(theNbSpacesOnTheLeft + DEFAULT_IDENT_LEFT);
        Buff += "L >> ";
        Buff += "<MTime is not dumped>: tag = ";
        Buff += L.Tag();
        Buff += '\n';
        theOut << Buff.ToCString();
      }
      else
        dumpLabelWithAttributes(theOut, L, theNbSpacesOnTheLeft + DEFAULT_IDENT_LEFT,
                                doDumpChildren, theContentType, theVerbosity, aPrefix);
    }
  }
}

//! Dumps CAF Attribute.
//! \param theOut               [out] output stream.
//! \param theAttribute         [in]  CAF Attribute to dump.
//! \param theNbSpacesOnTheLeft [in]  decoration indentation of the left.
//! \param theContentType       [in]  type of the content to dump.
//! \param theVerbosity         [in]  dumping verbosity level.
//! \param thePrefix            [in]  optional prefix.
void
  ActData_CAFDumper::dumpAttribute(Standard_OStream&              theOut,
                                   const Handle(TDF_Attribute)&   theAttribute,
                                   const Standard_Integer         theNbSpacesOnTheLeft,
                                   const Content                  ActData_NotUsed(theContentType),
                                   const Verbosity                theVerbosity,
                                   const TCollection_AsciiString& thePrefix)
{
  // Attempt to dump the Attribute's value
  TCollection_AsciiString aValStr;
  if ( theAttribute->IsInstance( STANDARD_TYPE(TDataStd_Integer) ) )
  {
    Handle(TDataStd_Integer) aValAttr = Handle(TDataStd_Integer)::DownCast(theAttribute);
    aValStr = aValAttr->Get();
  }
  else if ( theAttribute->IsInstance( STANDARD_TYPE(TDataStd_Real) ) )
  {
    Handle(TDataStd_Real) aValAttr = Handle(TDataStd_Real)::DownCast(theAttribute);
    aValStr = aValAttr->Get();
  }
  else if ( theAttribute->IsInstance( STANDARD_TYPE(TDataStd_Name) ) )
  {
    Handle(TDataStd_Name) aValAttr = Handle(TDataStd_Name)::DownCast(theAttribute);
    const TCollection_ExtendedString& aUStr = aValAttr->Get();
    if ( aUStr.IsAscii() )
      aValStr = aUStr;
    else
      aValStr = "<UNICODE string>";
  }
  else if ( theAttribute->IsInstance( STANDARD_TYPE(TDataStd_AsciiString) ) )
  {
    Handle(TDataStd_AsciiString) aValAttr = Handle(TDataStd_AsciiString)::DownCast(theAttribute);
    aValStr = aValAttr->Get();
  }
  else if ( theAttribute->IsInstance( STANDARD_TYPE(TNaming_NamedShape) ) )
  {
    Handle(TNaming_NamedShape) aValAttr = Handle(TNaming_NamedShape)::DownCast(theAttribute);
    if ( theVerbosity == Verbosity_DetailsSkipUnstable && !aValAttr->Get().IsNull() )
      aValStr = "<Some shape address (unstable to dump)>";
    else
      aValStr = toString( aValAttr->Get() );
  }
  else if ( theAttribute->IsInstance( STANDARD_TYPE(TDataStd_BooleanArray) ) )
  {
    Handle(TDataStd_BooleanArray) aValAttr = Handle(TDataStd_BooleanArray)::DownCast(theAttribute);
    aValStr = arrayToLimitedStr<Standard_Boolean, Handle(TDataStd_BooleanArray)>(aValAttr);
  }
  else if ( theAttribute->IsInstance( STANDARD_TYPE(TDataStd_ExtStringArray) ) )
  {
    Handle(TDataStd_ExtStringArray) aValAttr = Handle(TDataStd_ExtStringArray)::DownCast(theAttribute);
    aValStr = arrayToLimitedStr<TCollection_ExtendedString, Handle(TDataStd_ExtStringArray)>(aValAttr);
  }
  else if ( theAttribute->IsInstance( STANDARD_TYPE(TDataStd_IntegerArray) ) )
  {
    Handle(TDataStd_IntegerArray) aValAttr = Handle(TDataStd_IntegerArray)::DownCast(theAttribute);
    aValStr = arrayToLimitedStr<Standard_Integer, Handle(TDataStd_IntegerArray)>(aValAttr);
  }
  else if ( theAttribute->IsInstance( STANDARD_TYPE(TDataStd_RealArray) ) )
  {
    Handle(TDataStd_RealArray) aValAttr = Handle(TDataStd_RealArray)::DownCast(theAttribute);
    aValStr = arrayToLimitedStr<Standard_Real, Handle(TDataStd_RealArray)>(aValAttr);
  }
  else if ( theAttribute->IsInstance( STANDARD_TYPE(TDataStd_IntPackedMap) ) )
  {
    Handle(TDataStd_IntPackedMap) aValAttr = Handle(TDataStd_IntPackedMap)::DownCast(theAttribute);
    aValStr = collectionToLimitedStr(aValAttr);
  }
  else if ( theAttribute->IsInstance( STANDARD_TYPE(TDF_Reference) ) )
  {
    Handle(TDF_Reference) aValAttr = Handle(TDF_Reference)::DownCast(theAttribute);
    aValStr = ActData_Utils::GetEntry( aValAttr->Get() );
  }
  else if ( theAttribute->IsInstance( STANDARD_TYPE(TDataStd_ReferenceList) ) )
  {
    Handle(TDataStd_ReferenceList) aValAttr = Handle(TDataStd_ReferenceList)::DownCast(theAttribute);
    aValStr = collectionToStr(aValAttr);
  }
  else if ( theAttribute->IsInstance( STANDARD_TYPE(ActData_MeshAttr) ) )
  {
    Handle(ActData_MeshAttr) aMeshAttr = Handle(ActData_MeshAttr)::DownCast(theAttribute);
    Handle(ActData_Mesh) aMesh = aMeshAttr->GetMesh();
    if ( aMesh.IsNull() )
      aValStr = "NULL MESH";
    else
    {
      aValStr = "Mesh: ";
      aValStr = aValStr.Cat("nodes {").Cat( aMesh->NbNodes() ).Cat("} faces {").Cat( aMesh->NbFaces() ).Cat("}");
    }
  }
  else
    aValStr = "### cannot dump ###";

  TCollection_AsciiString aLeftMargin = emptySpaces(theNbSpacesOnTheLeft);
  TCollection_AsciiString aType = theAttribute->DynamicType()->Name();
  if ( !thePrefix.IsEmpty() )
  {
    aType += " ";
    aType += thePrefix;
  }
  TCollection_AsciiString Buff = ATTRIBUTE_INFO(aType, aValStr, aLeftMargin);

  theOut << Buff.ToCString();
}

//! Returns a blank string containing the desired number of whitespaces.
//! \param theNbSpaces [in] desired number of spaces.
//! \return requested string.
TCollection_AsciiString
  ActData_CAFDumper::emptySpaces(const Standard_Integer theNbSpaces)
{
  TCollection_AsciiString aResult;
  for ( Standard_Integer i = 1; i <= theNbSpaces; i++ )
    aResult = aResult.Cat(" ");

  return aResult;
}

//! Prepares a truncated string representation of the given array.
//! \param theArr [in] collection to dump.
//! \return truncated string representation.
template<typename E, typename A>
TCollection_AsciiString
  ActData_CAFDumper::arrayToLimitedStr(const A& theArr)
{
  TCollection_AsciiString aResult("{");
  Standard_Integer j = 1;
  Standard_Boolean isTrimmed = Standard_False;
  for ( Standard_Integer i = theArr->Lower(); i <= theArr->Upper(); ++i, ++j )
  {
    if ( j == DEFAULT_ARR_LIMIT && i < theArr->Upper() - 1 )
    {
      isTrimmed = Standard_True;
      break; // We do not dump everything to keep the output human-readable
    }

    E aVal = theArr->Value(i);
    aResult = aResult.Cat( toString(aVal) );

    if ( i < theArr->Upper() )
      aResult = aResult.Cat(", ");
  }

  if ( isTrimmed )
  {
    aResult = aResult.Cat(", ... ");
    E aVal = theArr->Value( theArr->Upper() );
    aResult = aResult.Cat( toString(aVal) );
  }

  aResult = aResult.Cat("}");
  return aResult;
}

//! Dumps Packed Map of Integer to string.
//! \param theCol [in] collection to dump.
//! \return truncated string representation of the given collection.
TCollection_AsciiString
  ActData_CAFDumper::collectionToLimitedStr(const Handle(TDataStd_IntPackedMap)& theCol)
{
  TCollection_AsciiString aResult("{");
  Standard_Integer aNbElems = theCol->Extent();
  Standard_Boolean isTrimmed = Standard_False;
  TColStd_MapIteratorOfPackedMapOfInteger aMapIt( theCol->GetMap() );
  Standard_Integer j = 1;
  for ( ; aMapIt.More(); aMapIt.Next() )
  {
    if ( j == DEFAULT_ARR_LIMIT )
    {
      isTrimmed = Standard_True;
      break; // We do not dump everything to keep the output human-readable
    }

    Standard_Integer aVal = aMapIt.Key();
    aResult = aResult.Cat( toString(aVal) );

    if ( j < aNbElems )
      aResult = aResult.Cat(", ");

    j++;
  }

  if ( isTrimmed )
    aResult = aResult.Cat(", ... ");

  aResult = aResult.Cat("}");
  return aResult;
}

//! Dumps Reference List to string.
//! \param theCol [in] collection to dump.
//! \return truncated string representation of the given collection.
TCollection_AsciiString
  ActData_CAFDumper::collectionToStr(const Handle(TDataStd_ReferenceList)& theCol)
{
  TCollection_AsciiString aResult("{");
  const TDF_LabelList& aRefLabels = theCol->List();
  Standard_Integer j = 1, aNbElems = aRefLabels.Extent();
  for ( TDF_ListIteratorOfLabelList it(aRefLabels); it.More(); it.Next() )
  {
    const TDF_Label& aRefLabel = it.Value();
    aResult = aResult.Cat( ActData_Utils::GetEntry(aRefLabel) );

    if ( j < aNbElems )
      aResult = aResult.Cat(", ");

    j++;
  }

  aResult = aResult.Cat("}");
  return aResult;
}

//! Dumps BOOLEAN value to string.
//! \param theElem [in] value to dump.
//! \return string representation of the given value.
TCollection_AsciiString
  ActData_CAFDumper::toString(const Standard_Boolean theElem)
{
  return (theElem ? "1" : "0");
}

//! Dumps INTEGER value to string.
//! \param theElem [in] value to dump.
//! \return string representation of the given value.
TCollection_AsciiString
  ActData_CAFDumper::toString(const Standard_Integer theElem)
{
  return theElem;
}

//! Dumps REAL value to string.
//! \param theElem [in] value to dump.
//! \return string representation of the given value.
TCollection_AsciiString
  ActData_CAFDumper::toString(const Standard_Real theElem)
{
  return theElem;
}

//! Dumps EXTENDED STRING value to string.
//! \param theElem [in] value to dump.
//! \return string representation of the given value.
TCollection_AsciiString
  ActData_CAFDumper::toString(const TCollection_ExtendedString& theElem)
{
  if ( theElem.IsAscii() )
    return theElem;
  else
    return "<UNICODE string>";
}

//! Dumps OCCT topological shape to string.
//! \param theElem [in] value to dump.
//! \return string representation of the given value.
TCollection_AsciiString
  ActData_CAFDumper::toString(const TopoDS_Shape& theShape)
{
  std::string anAddrStr;
  std::ostringstream ost;
  ost << theShape.TShape().get();
  anAddrStr = ost.str();

  return TCollection_AsciiString("[").Cat( anAddrStr.c_str() ).Cat("]");
}
