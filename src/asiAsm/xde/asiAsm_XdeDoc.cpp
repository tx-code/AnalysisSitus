//-----------------------------------------------------------------------------
// Created on: 22 September 2020 (*)
// Created by: Sergey SLYADNEV
//-----------------------------------------------------------------------------
// Copyright (c) 2020-present, Sergey Slyadnev
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
//    * Neither the name of the copyright holder(s) nor the
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
//-----------------------------------------------------------------------------

// Own include
#include <asiAsm_XdeDoc.h>

// asiAsm includes
#include <asiAsm_XdeApp.h>
#include <asiAsm_XdeDocIterator.h>
#include <asiAsm_XdeGraph.h>
#include <asiAsm_XdePartRepr.h>

// asiAlgo includes
#include <asiAlgo_FileFormat.h>

// OpenCascade includes
#include <BRep_Builder.hxx>
#include <CDM_MetaData.hxx>
#include <Quantity_ColorRGBA.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <TDataStd_ChildNodeIterator.hxx>
#include <TDataStd_Name.hxx>
#include <TDataStd_TreeNode.hxx>
#include <TDF_AttributeIterator.hxx>
#include <TDF_ChildIterator.hxx>
#include <TNaming_NamedShape.hxx>
#include <TNaming_Tool.hxx>
#include <XCAFDoc.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XSControl_TransferReader.hxx>
#include <XSControl_WorkSession.hxx>

//-----------------------------------------------------------------------------

#undef COUT_DEBUG
#if defined COUT_DEBUG
  #pragma message("===== warning: COUT_DEBUG is enabled")
#endif

#undef FILE_DEBUG
#if defined FILE_DEBUG
  #pragma message("===== warning: FILE_DEBUG is enabled")
#endif

//-----------------------------------------------------------------------------

asiAsm_XdeDoc::asiAsm_XdeDoc(ActAPI_ProgressEntry progress,
                             ActAPI_PlotterEntry  plotter)
: Standard_Transient (),
  m_progress         (progress),
  m_plotter          (plotter)
{
  this->NewDocument();
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::NewDocument()
{
  this->init( this->newDocument() ); // Initialize internal structure.
}

//-----------------------------------------------------------------------------

bool asiAsm_XdeDoc::Load(const TCollection_AsciiString& filename)
{
  // Recognize format.
  const asiAlgo_FileFormat
    format = asiAlgo_FileFormatTool::FormatFromFileExtension(filename);

  // Load CAD data.
  switch ( format )
  {
    case FileFormat_STEP:
      return this->LoadSTEP(filename);
    case FileFormat_XBF:
      return this->LoadNative(filename);
    default:
      break;
  }

  m_progress.SendLogMessage(LogErr(Normal) << "Unsupported file format.");
  return false;
}

//-----------------------------------------------------------------------------

bool asiAsm_XdeDoc::LoadNative(const TCollection_AsciiString& filename)
{
  if ( !m_doc.IsNull() )
    this->Release();

  Handle(asiAsm_XdeApp) A = this->getApplication();
  //
  Handle(TDocStd_Document) Doc;

  /* =======================
   *  Open the CAF Document.
   * ======================= */

  PCDM_ReaderStatus status = PCDM_RS_OpenError;
  //
  try
  {
    status = A->Open(filename, Doc);

    if ( status == PCDM_RS_AlreadyRetrieved )
    {
      const int nb    = A->IsInSession(filename);
      const int nbDoc = A->NbDocuments();

      if ( (nb >= 1) && (nb <= nbDoc) )
      {
        A->GetDocument(nb, Doc);
        status = PCDM_RS_OK;
      }
      else
      {
        m_progress.SendLogMessage(LogErr(Normal) << "Already retrieved but not found.");
        return false;
      }
    }
  }
  catch ( Standard_Failure exc )
  {
    m_progress.SendLogMessage( LogErr(Normal) << "OCCT exception: %1 %2."
                                              << exc.DynamicType()->Name()
                                              << exc.GetMessageString() );
    return false;
  }

  // Check status.
  if ( status != PCDM_RS_OK )
  {
    TCollection_AsciiString statusStr;

         if ( status == PCDM_RS_NoDriver )                    statusStr = "PCDM_RS_NoDriver";
    else if ( status == PCDM_RS_UnknownFileDriver )           statusStr = "PCDM_RS_UnknownFileDriver";
    else if ( status == PCDM_RS_OpenError )                   statusStr = "PCDM_RS_OpenError";
    else if ( status == PCDM_RS_NoVersion )                   statusStr = "PCDM_RS_NoVersion";
    else if ( status == PCDM_RS_NoSchema )                    statusStr = "PCDM_RS_NoSchema";
    else if ( status == PCDM_RS_NoDocument )                  statusStr = "PCDM_RS_NoDocument";
    else if ( status == PCDM_RS_ExtensionFailure )            statusStr = "PCDM_RS_ExtensionFailure";
    else if ( status == PCDM_RS_WrongStreamMode )             statusStr = "PCDM_RS_WrongStreamMode";
    else if ( status == PCDM_RS_FormatFailure )               statusStr = "PCDM_RS_FormatFailure";
    else if ( status == PCDM_RS_TypeFailure )                 statusStr = "PCDM_RS_TypeFailure";
    else if ( status == PCDM_RS_TypeNotFoundInSchema )        statusStr = "PCDM_RS_TypeNotFoundInSchema";
    else if ( status == PCDM_RS_UnrecognizedFileFormat )      statusStr = "PCDM_RS_UnrecognizedFileFormat";
    else if ( status == PCDM_RS_MakeFailure )                 statusStr = "PCDM_RS_MakeFailure";
    else if ( status == PCDM_RS_PermissionDenied )            statusStr = "PCDM_RS_PermissionDenied";
    else if ( status == PCDM_RS_DriverFailure )               statusStr = "PCDM_RS_DriverFailure";
    else if ( status == PCDM_RS_AlreadyRetrievedAndModified ) statusStr = "PCDM_RS_AlreadyRetrievedAndModified";
    else if ( status == PCDM_RS_AlreadyRetrieved )            statusStr = "PCDM_RS_AlreadyRetrieved";
    else if ( status == PCDM_RS_UnknownDocument )             statusStr = "PCDM_RS_UnknownDocument";
    else if ( status == PCDM_RS_WrongResource )               statusStr = "PCDM_RS_WrongResource";
    else if ( status == PCDM_RS_ReaderException )             statusStr = "PCDM_RS_ReaderException";
    else if ( status == PCDM_RS_NoModel )                     statusStr = "PCDM_RS_NoModel";

    m_progress.SendLogMessage(LogErr(Normal) << "Reader failed. Error code: %1." << statusStr);

    return false;
  }

  // Remove flag `IsRetrieved` to allow loading one file several times in one session.
  Doc->MetaData()->UnsetDocument();

  // Initialize Data Model.
  this->init(Doc);

  // Success.
  return true;
}

//-----------------------------------------------------------------------------

bool asiAsm_XdeDoc::LoadSTEP(const TCollection_AsciiString& filename)
{
  if ( m_doc.IsNull() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Cannot load into null Document.");
    return false;
  }

  // Prepare reader.
  STEPCAFControl_Reader xdeReader;
  Handle(XSControl_WorkSession) WS = xdeReader.Reader().WS();

  // Read CAD and associated data from file.
  try
  {
    // Read file.
    IFSelect_ReturnStatus outcome = xdeReader.ReadFile( filename.ToCString() );
    //
    if ( outcome != IFSelect_RetDone )
    {
      m_progress.SendLogMessage(LogErr(Normal) << "Cannot read STEP file from disk." );
      //
      this->clearSession(WS);
      return false;
    }

    // Transfer data.
    if ( !xdeReader.Transfer(m_doc) )
    {
      m_progress.SendLogMessage(LogErr(Normal) << "STEP reader failed (error occurred transferring STEP file to XDE)." );
      //
      this->clearSession(WS);
      return false;
    }

    this->clearSession(WS);
    //
    m_progress.SendLogMessage(LogInfo(Normal) << "File '%1' loaded." << filename);
  }
  catch ( ... )
  {
    m_progress.SendLogMessage( LogErr(Normal) << "STEP reader failed (exception on reading STEP file)." );
    return false;
  }

  return true;
}

//-----------------------------------------------------------------------------

bool asiAsm_XdeDoc::SaveAs(const TCollection_AsciiString& filename)
{
  if ( m_doc.IsNull() )
    return false;

  Handle(asiAsm_XdeApp) A = this->getApplication();

  // Write.
  PCDM_StoreStatus status = PCDM_SS_WriteFailure;
  //
  try
  {
    status = A->SaveAs(m_doc, filename);
  }
  catch ( Standard_Failure exc )
  {
    m_progress.SendLogMessage( LogErr(Normal) << "OCCT exception: %1 %2."
                                              << exc.DynamicType()->Name()
                                              << exc.GetMessageString() );
    return false;
  }

  // Check status.
  if ( status != PCDM_SS_OK )
  {
    TCollection_AsciiString statusStr;

         if ( status == PCDM_SS_DriverFailure )      statusStr = "PCDM_SS_DriverFailure";
    else if ( status == PCDM_SS_WriteFailure )       statusStr = "PCDM_SS_WriteFailure";
    else if ( status == PCDM_SS_Doc_IsNull )         statusStr = "PCDM_SS_Doc_IsNull";
    else if ( status == PCDM_SS_No_Obj )             statusStr = "PCDM_SS_No_Obj";
    else if ( status == PCDM_SS_Info_Section_Error ) statusStr = "PCDM_SS_Info_Section_Error";
    else                                             statusStr = "PCDM_SS_Failure";

    m_progress.SendLogMessage(LogErr(Normal) << "Writer failed. Error code: %1." << statusStr);

    return false;
  }

  // Success.
  return true;
}

//-----------------------------------------------------------------------------

bool asiAsm_XdeDoc::IsEmpty() const
{
  if ( m_doc.IsNull() )
    return true;

  TDF_ChildIterator cit( m_doc->Main() );
  const bool isDocEmpty = !cit.More();

  return isDocEmpty;
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::Release()
{
  if ( m_doc.IsNull() )
    return;

  // Close OCAF Document.
  Handle(asiAsm_XdeApp) A = this->getApplication();
  //
  if ( A->CanClose(m_doc) == CDM_CCS_OK )
    A->Close(m_doc);
  //
  m_doc.Nullify();

  // Clear cache.
  m_LECache.Clear();
}

//-----------------------------------------------------------------------------

bool asiAsm_XdeDoc::GetObjectName(const asiAsm_XdePersistentId& id,
                                  TCollection_ExtendedString&   name) const
{
  // Get label for object ID.
  TDF_Label label;
  TDF_Tool::Label(this->m_doc->GetData(), id, label);

  // Access name attribute.
  Handle(TDataStd_Name) nameAttr;
  if ( !label.FindAttribute(TDataStd_Name::GetID(), nameAttr) )
  {
    name = "";
    return false;
  }
  //
  name = nameAttr->Get();
  return true;
}

//-----------------------------------------------------------------------------

bool asiAsm_XdeDoc::GetObjectName(const asiAsm_XdePartId&     id,
                                  TCollection_ExtendedString& name) const
{
  name = this->GetPartName(id);
  return true;
}

//-----------------------------------------------------------------------------

bool asiAsm_XdeDoc::GetObjectName(const asiAsm_XdeAssemblyItemId& id,
                                  TCollection_ExtendedString&     name) const
{
  TDF_Label original = this->GetOriginal(id);

  TCollection_AsciiString oEntry;
  TDF_Tool::Entry(original, oEntry);

  return this->GetObjectName(oEntry, name);
}
//-----------------------------------------------------------------------------

TCollection_ExtendedString
  asiAsm_XdeDoc::GetObjectName(const TDF_Label& label) const
{
  // Get name directly from OCAF attribute.
  Handle(TDataStd_Name) nameAttr;
  if ( !label.FindAttribute(TDataStd_Name::GetID(), nameAttr) )
    return TCollection_ExtendedString("<unnamed>");

  return nameAttr->Get();
}

//-----------------------------------------------------------------------------

TCollection_ExtendedString
  asiAsm_XdeDoc::GetPartName(const asiAsm_XdePartId& part) const
{
  // Get label by part ID.
  TDF_Label label;
  TDF_Tool::Label(this->m_doc->GetData(), part.Entry, label);

  // Get name.
  return this->GetObjectName(label);
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetPartRepresentations(const asiAsm_XdePartId&                  partId,
                                           std::vector<Handle(asiAsm_XdePartRepr)>& reps) const
{
  this->GetPartRepresentations(this->GetLabel(partId), reps);
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetPartRepresentations(const TDF_Label&                         label,
                                           std::vector<Handle(asiAsm_XdePartRepr)>& reps) const
{
  if ( label.IsNull() ) return; // Contract check.

  // Iterate over the part's attributes and use the representation factory
  // to construct representations for the attributes that allow doing this.
  for ( TDF_AttributeIterator itAtt(label); itAtt.More(); itAtt.Next() )
  {
    const Handle(TDF_Attribute)& attr = itAtt.Value();

    // Construct representation.
    Handle(asiAsm_XdePartRepr) repr = asiAsm_XdePartReprFactory::New(attr);
    //
    if ( !repr.IsNull() )
      reps.push_back(repr);
  }
}

//-----------------------------------------------------------------------------

bool asiAsm_XdeDoc::GetPartRepresentation(const asiAsm_XdePartId&     partId,
                                          const Standard_GUID&        guid,
                                          Handle(asiAsm_XdePartRepr)& rep) const
{
  TDF_Label partLab = this->GetLabel(partId);

  // The representation's GUID is identical to the attribute's GUID.
  Handle(TDF_Attribute) attr;
  //
  if ( !partLab.FindAttribute(guid, attr) )
    return false;

  // Construct a representation using the factory.
  rep = asiAsm_XdePartReprFactory::New(attr);
  //
  return !rep.IsNull();
}

//-----------------------------------------------------------------------------

bool asiAsm_XdeDoc::IsAssembly(const TDF_Label& itemLabel) const
{
  if ( itemLabel.IsNull() )
    return false;

  return this->GetShapeTool()->IsAssembly(itemLabel);
}

//-----------------------------------------------------------------------------

bool asiAsm_XdeDoc::IsAssembly(const asiAsm_XdeAssemblyItemId& item) const
{
  return this->IsAssembly( this->GetOriginal(item) );
}

//-----------------------------------------------------------------------------

bool asiAsm_XdeDoc::IsInstance(const TDF_Label& itemLab,
                               TDF_Label&       origin) const
{
  if ( itemLab.IsNull() )
    return false;

  if ( this->GetShapeTool()->IsReference(itemLab) )
  {
    Handle(TDataStd_TreeNode) JumpNode;
    itemLab.FindAttribute(XCAFDoc::ShapeRefGUID(), JumpNode);
    //
    if ( JumpNode->HasFather() )
    {
      origin = JumpNode->Father()->Label(); // Declaration-level origin.
    }
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------

bool asiAsm_XdeDoc::IsInstance(const asiAsm_XdeAssemblyItemId& item,
                               TDF_Label&                      origin) const
{
  TDF_Label label = this->GetLabel(item);
  //
  return this->IsInstance(label, origin);
}

//-----------------------------------------------------------------------------

bool asiAsm_XdeDoc::IsPart(const TDF_Label& label) const
{
  if ( label.IsNull() )
    return false;

  // Check with shape tool.
  Handle(XCAFDoc_ShapeTool) STool = this->GetShapeTool();
  //
  return STool->IsSimpleShape(label);
}

//-----------------------------------------------------------------------------

bool asiAsm_XdeDoc::IsPart(const asiAsm_XdeAssemblyItemId& item) const
{
  // Check original on being a simple shape.
  TDF_Label original = this->GetOriginal(item);
  //
  return this->IsPart(original);
}

//-----------------------------------------------------------------------------

bool asiAsm_XdeDoc::IsOriginal(const TDF_Label& label) const
{
  if ( label.IsNull() )
    return false;

  Handle(XCAFDoc_ShapeTool) STool = this->GetShapeTool();
  if ( !STool->IsShape(label) )
    return false;

  if ( STool->IsTopLevel(label) )
    return true;

  return false;
}

//-----------------------------------------------------------------------------

TDF_Label asiAsm_XdeDoc::GetOriginal(const TDF_Label& itemLabel) const
{
  if ( itemLabel.IsNull() )
    return itemLabel;

  TDF_Label origin;
  //
  if ( !this->IsInstance(itemLabel, origin) )
    return itemLabel;

  return origin;
}

//-----------------------------------------------------------------------------

TDF_Label asiAsm_XdeDoc::GetOriginal(const asiAsm_XdeAssemblyItemId& item) const
{
  return this->GetOriginal( this->GetLabel(item) );
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetOriginals(const Handle(asiAsm_XdeHAssemblyItemIdsMap)& anyItems,
                                 TDF_LabelSequence&                           originalLabels) const
{
  // Loop over the items of interest and collect originals as labels into
  // a map, so that no duplications will be stored.
  NCollection_IndexedMap<TDF_Label, TDF_LabelMapHasher> originals;
  //
  for ( asiAsm_XdeHAssemblyItemIdsMap::Iterator iter(*anyItems); iter.More(); iter.Next() )
  {
    TDF_Label original = this->GetOriginal( iter.Value() );

    const int numOriginals = originals.Size();
    const int idxOriginal  = originals.Add(original);

    if ( numOriginals < idxOriginal )
      originalLabels.Append(original);
  }
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetOriginals(const asiAsm_XdeAssemblyItemIds& anyItems,
                                 TDF_LabelSequence&               originalLabels) const
{
  // Loop over the items of interest and collect originals as labels into
  // a map, so that no duplications will be stored.
  NCollection_IndexedMap<TDF_Label, TDF_LabelMapHasher> originals;
  //
  for ( asiAsm_XdeAssemblyItemIds::Iterator iter(anyItems); iter.More(); iter.Next() )
  {
    TDF_Label original = this->GetOriginal( iter.Value() );

    const int numOriginals = originals.Size();
    const int idxOriginal  = originals.Add(original);

    if ( numOriginals < idxOriginal )
      originalLabels.Append(original);
  }
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetOriginalsWithInstances(const Handle(asiAsm_XdeHAssemblyItemIdsMap)& anyItems,
                                              asiAsm_XdeLabelsToInstancesMap&              origInstances) const
{
  // Loop over the items of interest and collect originals as labels into
  // a map, so that no duplications will be stored.
  for ( asiAsm_XdeHAssemblyItemIdsMap::Iterator iter(*anyItems); iter.More(); iter.Next() )
    this->getOriginalsWithInstances(iter.Value(), origInstances);
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetOriginalsWithInstances(const asiAsm_XdeAssemblyItemIds& anyItems,
                                              asiAsm_XdeLabelsToInstancesMap&  origInstances) const
{
  // Loop over the items of interest and collect originals as labels into
  // a map, so that no duplications will be stored.
  for ( asiAsm_XdeAssemblyItemIds::Iterator iter(anyItems); iter.More(); iter.Next() )
    this->getOriginalsWithInstances(iter.Value(), origInstances);
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetParts(asiAsm_XdePartIds& parts) const
{
  asiAsm_XdeAssemblyItemIds parents;
  this->GetRootAssemblyItems(parents);
  //
  this->GetParts(parents, parts, false);
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetParts(const Handle(asiAsm_XdeHAssemblyItemIdsMap)& anyItems,
                             asiAsm_XdePartIds&                           parts,
                             const bool                                   isAlreadyLeafs) const
{
  // Get leaf assembly items.
  Handle(asiAsm_XdeHAssemblyItemIdsMap) leafItems;
  if ( !isAlreadyLeafs )
  {
    leafItems = new asiAsm_XdeHAssemblyItemIdsMap;
    this->GetLeafAssemblyItems(anyItems, leafItems);
  }

  // Get all original labels.
  TDF_LabelSequence originals;
  this->GetOriginals(isAlreadyLeafs ? anyItems : leafItems, originals);
  //
  this->getParts(originals, parts);
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetParts(const asiAsm_XdeAssemblyItemIds& anyItems,
                             asiAsm_XdePartIds&               parts,
                             const bool                       isAlreadyLeafs) const
{
  // Get leaf assembly items.
  asiAsm_XdeAssemblyItemIds leafItems;
  if ( !isAlreadyLeafs )
    this->GetLeafAssemblyItems(anyItems, leafItems);

  // Get all original labels.
  TDF_LabelSequence originals;
  this->GetOriginals(isAlreadyLeafs ? anyItems : leafItems, originals);
  //
  this->getParts(originals, parts);
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetParts(const asiAsm_XdeAssemblyItemIds&       anyItems,
                             Handle(asiAsm_XdeHAssemblyItemIdsMap)& leafItems,
                             asiAsm_XdePartIds&                     parts) const
{
  // Get leaf assembly items.
  this->GetLeafAssemblyItems(anyItems, leafItems);
  //
  this->GetParts(leafItems, parts, true);
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetParts(const Handle(asiAsm_XdeHAssemblyItemIdsMap)& anyItems,
                             Handle(asiAsm_XdeHAssemblyItemIdsMap)&       leafItems,
                             asiAsm_XdePartIds&                           parts) const
{
  // Get leaf assembly items.
  this->GetLeafAssemblyItems(anyItems, leafItems);
  //
  this->GetParts(leafItems, parts, true);
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetParts(const asiAsm_XdeAssemblyItemIds& anyItems,
                             asiAsm_XdeAssemblyItemIds&       leafItems,
                             asiAsm_XdePartIds&               parts) const
{
  // Get leaf assembly items.
  this->GetLeafAssemblyItems(anyItems, leafItems);
  //
  this->GetParts(leafItems, parts, true);
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::CountParts(NCollection_DataMap<asiAsm_XdePartId, int, asiAsm_XdePartId::Hasher>& quantities) const
{
  // Construct HAG.
  Handle(asiAsm_XdeGraph) hag = new asiAsm_XdeGraph(this);

  // Loop over the HAG elements and collect the information for the elements
  // of PART type. We are interested in their usage occurrences.
  const NCollection_IndexedMap<asiAsm_XdePersistentId>& elems = hag->GetNodes();
  //
  for ( int nid = 1; nid <= elems.Extent(); ++nid )
  {
    if ( hag->GetNodeType(nid) != asiAsm_XdeGraph::NodeType_Part )
      continue;

    const asiAsm_XdePersistentId& pid      = elems(nid);
    const int                     quantity = hag->GetUsageOccurrenceQuantity(nid);

    quantities.Bind(pid, quantity);
  }
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetPartsWithInstances(const Handle(asiAsm_XdeHAssemblyItemIdsMap)& anyItems,
                                          asiAsm_XdePartsToInstancesMap&               partsInstances,
                                          const bool                                   isAlreadyLeafs) const
{
  // Get leaf assembly items.
  Handle(asiAsm_XdeHAssemblyItemIdsMap) leafItems;
  if ( !isAlreadyLeafs )
  {
    leafItems = new asiAsm_XdeHAssemblyItemIdsMap;
    this->GetLeafAssemblyItems(anyItems, leafItems);
  }

  // Get map of original labels to instances.
  asiAsm_XdeLabelsToInstancesMap origInstances;
  this->GetOriginalsWithInstances(isAlreadyLeafs ? anyItems : leafItems, origInstances);
  //
  this->getPartsWithInstances(origInstances, partsInstances);
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetPartsWithInstances(const asiAsm_XdeAssemblyItemIds& anyItems,
                                          asiAsm_XdePartsToInstancesMap&   partsInstances,
                                          const bool                       isAlreadyLeafs) const
{
  // Get leaf assembly items.
  asiAsm_XdeAssemblyItemIds leafItems;
  if ( !isAlreadyLeafs )
    this->GetLeafAssemblyItems(anyItems, leafItems);

  // Get map of original labels to instances.
  asiAsm_XdeLabelsToInstancesMap origInstances;
  this->GetOriginalsWithInstances(isAlreadyLeafs ? anyItems : leafItems, origInstances);
  //
  this->getPartsWithInstances(origInstances, partsInstances);
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetAsPartId(const asiAsm_XdeAssemblyItemId& assemblyEntryId,
                                asiAsm_XdePartId&               partEntryId)
{
  TDF_Label original = this->GetOriginal(assemblyEntryId);
  TDF_Tool::Entry(original, partEntryId.Entry);
}

//-----------------------------------------------------------------------------

TDF_Label asiAsm_XdeDoc::GetLabelOfModel() const
{
  return m_doc->Main().Root();
}

//-----------------------------------------------------------------------------

TDF_Label asiAsm_XdeDoc::GetLabel(const asiAsm_XdeAssemblyItemId& item) const
{
  if ( item.IsNull() )
    return TDF_Label();

  const TCollection_AsciiString& entry = item.GetLastEntry();

  return this->GetLabel(entry);
}

//-----------------------------------------------------------------------------

TDF_Label asiAsm_XdeDoc::GetLabel(const asiAsm_XdePartId& part) const
{
  if ( part.IsNull() )
    return TDF_Label();

  return this->GetLabel(part.Entry);
}

//-----------------------------------------------------------------------------

TDF_Label asiAsm_XdeDoc::GetLabel(const asiAsm_XdePersistentId& id) const
{
  if ( id.IsEmpty() )
    return TDF_Label();

  TDF_Label label;
  TDF_Tool::Label(this->m_doc->GetData(), id, label);

  return label;
}

//-----------------------------------------------------------------------------

TopoDS_Shape asiAsm_XdeDoc::GetShape(const asiAsm_XdeAssemblyItemId& item,
                                     const bool                      doTransform) const
{
  const Handle(XCAFDoc_ShapeTool)& STool = this->GetShapeTool();
  //
  TDF_Label       origin;
  TopLoc_Location T;

  if ( doTransform )
  {
    for ( int p = 1; p <= item.GetPathLength(); ++p )
    {
      const TCollection_AsciiString& entry = item(p);
      //
      TDF_Label L;
      TDF_Tool::Label(this->m_doc->GetData(), entry, L);

      // Accumulate transformation
      if ( p == item.GetPathLength() )
        origin = L;
      else
        T = T * STool->GetLocation(L);
    }
  }
  else // Initialize origin with the last item
    TDF_Tool::Label(this->m_doc->GetData(), item.GetLastEntry(), origin);

  if ( origin.IsNull() )
    return TopoDS_Shape();

  // Get part shape and apply proper transformation
  TopoDS_Shape shape = STool->GetShape(origin);
  //
  if ( !shape.IsNull() && doTransform )
    shape.Move(T);

  return shape;
}

//-----------------------------------------------------------------------------

TopoDS_Shape asiAsm_XdeDoc::GetShape(const asiAsm_XdePartId& part) const
{
  TDF_Label L;
  TDF_Tool::Label(this->m_doc->GetData(), part.Entry, L);

  Handle(TNaming_NamedShape) NS;
  if ( !L.FindAttribute(TNaming_NamedShape::GetID(), NS) )
    return TopoDS_Shape();

  return TNaming_Tool::GetShape(NS);
}

//-----------------------------------------------------------------------------

TopoDS_Shape asiAsm_XdeDoc::GetShape(const TDF_Label& label) const
{
  if ( label.IsNull() )
    return TopoDS_Shape();

  return this->GetShapeTool()->GetShape(label);
}

//-----------------------------------------------------------------------------

bool asiAsm_XdeDoc::GetParent(const asiAsm_XdeAssemblyItemId& item,
                              asiAsm_XdeAssemblyItemId&       parent) const
{
  if ( item.GetPathLength() <= 1 )
    return false;

  parent = item;
  parent.Remove( item.GetPathLength() );
  return true;
}

//-----------------------------------------------------------------------------

TopLoc_Location
  asiAsm_XdeDoc::GetParentLocation(const asiAsm_XdeAssemblyItemId& item,
                                   const bool                      doTransform) const
{
  const Handle(XCAFDoc_ShapeTool)& STool = this->GetShapeTool();
  //
  TopLoc_Location T;

  if ( doTransform )
  {
    for ( int p = 1; p < item.GetPathLength(); ++p )
    {
      const TCollection_AsciiString& entry = item(p);
      //
      TDF_Label L;
      TDF_Tool::Label(this->m_doc->GetData(), entry, L);
      if ( !L.IsNull() )
      {
        // Accumulate transformation.
        T = T * STool->GetLocation(L);
      }
    }
  }
  else // Initialize T with the parent's location.
  {
    if ( item.GetPathLength() > 1 )
    {
      TDF_Label L;
      TDF_Tool::Label(this->m_doc->GetData(), item(item.GetPathLength() - 1), L);

      if ( !L.IsNull() )
        T = STool->GetLocation(L);
    }
  }

  return T;
}

//-----------------------------------------------------------------------------

TopLoc_Location asiAsm_XdeDoc::GetOwnLocation(const asiAsm_XdeAssemblyItemId& item) const
{
  return this->GetShapeTool()->GetLocation( this->GetLabel(item) );
}

//-----------------------------------------------------------------------------

TopoDS_Shape asiAsm_XdeDoc::GetOneShape() const
{
  // Get all parts.
  TDF_LabelSequence labels;
  Handle(XCAFDoc_ShapeTool) STool = this->GetShapeTool();
  STool->GetFreeShapes(labels);
  //
  if ( !labels.Length() )
    return TopoDS_Shape();
  //
  if ( labels.Length() == 1 )
    return STool->GetShape( labels.First() );

  // Put everything into compound and return.
  TopoDS_Compound C;
  BRep_Builder B;
  B.MakeCompound(C);
  for ( int i = 1; i <= labels.Length(); ++i )
  {
    TopoDS_Shape S = STool->GetShape( labels(i) );
    B.Add(C, S);
  }
  return C;
}

//-----------------------------------------------------------------------------

TopoDS_Shape asiAsm_XdeDoc::GetOneShape(const asiAsm_XdeAssemblyItemIds& items) const
{
  // Create result container.
  TopoDS_Compound result;
  BRep_Builder BB;
  BB.MakeCompound(result);

  // Extract shapes.
  for ( asiAsm_XdeAssemblyItemIds::Iterator iter(items); iter.More(); iter.Next() )
  {
    TopoDS_Shape shape = this->GetShape( iter.Value() );
    BB.Add(result, shape);
  }
  return result;
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetLabelsOfRoots(TDF_LabelSequence& labels) const
{
  Handle(XCAFDoc_ShapeTool) STool = this->GetShapeTool();
  STool->GetFreeShapes(labels);
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetRootAssemblyItems(asiAsm_XdeAssemblyItemIds& items) const
{
  TDF_LabelSequence roots;
  this->GetLabelsOfRoots(roots);

  items.Clear();

  for ( TDF_LabelSequence::Iterator it(roots); it.More(); it.Next() )
  {
    TCollection_AsciiString entry;
    TDF_Tool::Entry(it.Value(), entry);

    asiAsm_XdeAssemblyItemId item = asiAsm_XdeAssemblyItemId::FromPath(entry);
    items.Append(item);
  }
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetLeafAssemblyItems(asiAsm_XdeAssemblyItemIds& items) const
{
  asiAsm_XdeAssemblyItemIds parents;
  this->GetRootAssemblyItems(parents);
  //
  this->GetLeafAssemblyItems(parents, items);
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetLeafAssemblyItems(const Handle(asiAsm_XdeHAssemblyItemIdsMap)& items) const
{
  asiAsm_XdeAssemblyItemIds parents;
  this->GetRootAssemblyItems(parents);
  //
  this->GetLeafAssemblyItems(parents, items);
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetLeafAssemblyItems(const asiAsm_XdeAssemblyItemIds&             parents,
                                         const Handle(asiAsm_XdeHAssemblyItemIdsMap)& items) const
{
  asiAsm_XdeAssemblyItemIds dumpItems;
  Handle(asiAsm_XdeHAssemblyItemIdsMap) traversed = new asiAsm_XdeHAssemblyItemIdsMap;

  for ( asiAsm_XdeAssemblyItemIds::Iterator ait(parents); ait.More(); ait.Next() )
    this->getLeafItems(ait.Value(), items, dumpItems, traversed);
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetLeafAssemblyItems(const Handle(asiAsm_XdeHAssemblyItemIdsMap)& parents,
                                         const Handle(asiAsm_XdeHAssemblyItemIdsMap)& items) const
{
  asiAsm_XdeAssemblyItemIds dumpItems;
  Handle(asiAsm_XdeHAssemblyItemIdsMap) traversed = new asiAsm_XdeHAssemblyItemIdsMap();

  for ( asiAsm_XdeHAssemblyItemIdsMap::Iterator ait(*parents); ait.More(); ait.Next() )
    this->getLeafItems(ait.Value(), items, dumpItems, traversed);
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetLeafAssemblyItems(const asiAsm_XdeAssemblyItemId&              parent,
                                         const Handle(asiAsm_XdeHAssemblyItemIdsMap)& items) const
{
  asiAsm_XdeAssemblyItemIds oneElemList;
  oneElemList.Append(parent);
  this->GetLeafAssemblyItems(oneElemList, items);
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetLeafAssemblyItems(const asiAsm_XdeAssemblyItemId& parent,
                                         asiAsm_XdeAssemblyItemIds&      items) const
{
  asiAsm_XdeAssemblyItemIds oneElemList;
  oneElemList.Append(parent);
  this->GetLeafAssemblyItems(oneElemList, items);
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetLeafAssemblyItems(const asiAsm_XdeAssemblyItemIds& parents,
                                         asiAsm_XdeAssemblyItemIds&       items) const
{
  Handle(asiAsm_XdeHAssemblyItemIdsMap) traversed = new asiAsm_XdeHAssemblyItemIdsMap();

  for ( asiAsm_XdeAssemblyItemIds::Iterator ait(parents); ait.More(); ait.Next() )
    this->getLeafItems(ait.Value(), NULL, items, traversed);
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetLabelsOfReplicas(const TDF_Label&   partLabel,
                                        TDF_LabelSequence& replicas)
{
  Handle(TDataStd_TreeNode) TN;
  if ( !partLabel.FindAttribute(XCAFDoc::ShapeRefGUID(), TN) )
    return;

  for ( TDataStd_ChildNodeIterator nit(TN); nit.More(); nit.Next() )
    replicas.Append( nit.Value()->Label() );
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetAssemblyItemsForPart(const asiAsm_XdePartId&    part,
                                            asiAsm_XdeAssemblyItemIds& items) const
{
  // Get label by part ID.
  TDF_Label label;
  TDF_Tool::Label(this->m_doc->GetData(), part.Entry, label);
  //
  if ( label.IsNull() )
    return;

  // Gather assembly items.
  // Handle cases when part is Free shape (in terms of ShapeTool),
  // i.e. has no instances.
  if ( this->GetShapeTool()->IsFree(label) )
    items.Append( asiAsm_XdeAssemblyItemId(part.Entry) );
  else
    this->GetAssemblyItemsForPart(label, items);
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetAssemblyItemsForPart(const TDF_Label&           original,
                                            asiAsm_XdeAssemblyItemIds& items) const
{
  asiAsm_XdeAssemblyItemIds allItems;
  this->GetLeafAssemblyItems(allItems);

  // Choose proper ones.
  for ( asiAsm_XdeAssemblyItemIds::Iterator it(allItems); it.More(); it.Next() )
    this->getAssemblyItemsForPart(original, it.Value(), NULL, items);
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetAssemblyItemsForPart(const TDF_Label&                             original,
                                            const Handle(asiAsm_XdeHAssemblyItemIdsMap)& items) const
{
  asiAsm_XdeAssemblyItemIds allItems;
  this->GetLeafAssemblyItems(allItems);

  // Choose proper ones.
  asiAsm_XdeAssemblyItemIds dumpItems;
  for ( asiAsm_XdeAssemblyItemIds::Iterator it(allItems); it.More(); it.Next() )
    this->getAssemblyItemsForPart(original, it.Value(), items, dumpItems);
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetAssemblyItemsForParts(const TDF_LabelMap&        originals,
                                             asiAsm_XdeAssemblyItemIds& items) const
{
  asiAsm_XdeAssemblyItemIds allItems;
  this->GetLeafAssemblyItems(allItems);

  // Choose proper ones.
  for ( asiAsm_XdeAssemblyItemIds::Iterator it(allItems); it.More(); it.Next() )
  {
    const asiAsm_XdeAssemblyItemId& item = it.Value();
    //
    TDF_Label itemLab = this->GetLabel(item);
    TDF_Label itemOriginalLab;
    //
    if ( itemLab.IsNull() || !this->IsInstance(itemLab, itemOriginalLab) )
      continue;

    if ( originals.Contains(itemOriginalLab) )
      items.Append(item);
  }

  // Check root shapes.
  TDF_LabelSequence freeShapes;
  this->GetShapeTool()->GetFreeShapes(freeShapes);
  //
  for ( TDF_LabelSequence::Iterator it(freeShapes); it.More(); it.Next() )
  {
    if ( this->IsPart( it.Value() ) && originals.Contains( it.Value() ) )
    {
      TCollection_AsciiString entry;
      TDF_Tool::Entry(it.Value(), entry);
      const asiAsm_XdeAssemblyItemId& item = asiAsm_XdeAssemblyItemId::FromPath(entry);
      items.Append(item);
    }
  }
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetAssemblyItemsForParts(const asiAsm_XdePartIds&   parts,
                                             asiAsm_XdeAssemblyItemIds& items) const
{
  TDF_LabelMap labelsMap;
  for ( asiAsm_XdePartIds::Iterator pit(parts); pit.More(); pit.Next() )
  {
    TDF_Label label;
    TDF_Tool::Label(this->m_doc->GetData(), pit.Value().Entry, label);

    if ( label.IsNull() )
      continue;

    // Handle cases when part is Free shape (in terms of ShapeTool),
    // i.e. has no instances.
    if ( this->GetShapeTool()->IsFree(label) )
      items.Append( asiAsm_XdeAssemblyItemId(pit.Value().Entry) );
    else
      labelsMap.Add(label);
  }

  this->GetAssemblyItemsForParts(labelsMap, items);
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetAssemblyItemsForParts(const TDF_LabelMap&                          originals,
                                             const Handle(asiAsm_XdeHAssemblyItemIdsMap)& items) const
{
  asiAsm_XdeAssemblyItemIds itemList;
  this->GetAssemblyItemsForParts(originals, itemList);
  //
  for ( asiAsm_XdeAssemblyItemIds::Iterator it(itemList); it.More(); it.Next() )
    items->Add( it.Value() );
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetPartners(const asiAsm_XdeAssemblyItemId& anyItem,
                                asiAsm_XdeAssemblyItemIds&      partners) const
{
  this->GetPartners(this->GetOriginal(anyItem), partners);
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetPartners(const TDF_Label&           original,
                                asiAsm_XdeAssemblyItemIds& partners) const
{
  // Loop over the parts and sub-assemblies in depth-first order.
  for ( asiAsm_XdeDocIterator ait(this); ait.More(); ait.Next() )
  {
    asiAsm_XdeAssemblyItemId currentItem     = ait.Current();
    TDF_Label                currentOriginal = this->GetOriginal(currentItem);

    if ( currentOriginal == original )
    {
      partners.Append(currentItem);

#if defined COUT_DEBUG
      std::cout << "Next found partner is ";
      currentItem.Dump(std::cout);
#endif
    }
  }
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetPartners(const asiAsm_XdeAssemblyItemIds& anyItems,
                                asiAsm_XdeAssemblyItemIds&       partners) const
{
  TDF_LabelMap originals;
  for ( asiAsm_XdeAssemblyItemIds::Iterator it(anyItems); it.More(); it.Next() )
  {
    originals.Add( this->GetOriginal( it.Value() ) );
  }
  this->GetAssemblyItemsForParts(originals, partners);
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetPartners(const Handle(asiAsm_XdeHAssemblyItemIdsMap)& anyItems,
                                Handle(asiAsm_XdeHAssemblyItemIdsMap)&       partners) const
{
  TDF_LabelMap originals;
  for ( asiAsm_XdeHAssemblyItemIdsMap::Iterator it(*anyItems); it.More(); it.Next() )
  {
    originals.Add( this->GetOriginal( it.Value() ) );
  }
  this->GetAssemblyItemsForParts(originals, partners);
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::GetPartners(const Handle(asiAsm_XdeHAssemblyItemIdsMap)& anyItems,
                                asiAsm_XdeAssemblyItemIds&                   partners) const
{
  TDF_LabelMap originals;
  for ( int i = 1; i <= anyItems->Extent(); ++i )
  {
    originals.Add( this->GetOriginal( anyItems->FindKey(i) ) );
  }
  this->GetAssemblyItemsForParts(originals, partners);
}

//-----------------------------------------------------------------------------

bool asiAsm_XdeDoc::GetColor(const asiAsm_XdePartId& partId,
                             Quantity_Color&         color) const
{
  Quantity_ColorRGBA colorRGBA;
  const bool isOk = this->GetColor(partId, colorRGBA);

  if ( isOk )
    color = colorRGBA.GetRGB();

  return isOk;
}

//-----------------------------------------------------------------------------

bool asiAsm_XdeDoc::GetColor(const asiAsm_XdePartId& partId,
                             Quantity_ColorRGBA&     color) const
{
  TDF_Label label = this->GetLabel(partId);

  return this->GetColor(label, color);
}

//-----------------------------------------------------------------------------

bool asiAsm_XdeDoc::GetColor(const TDF_Label&    label,
                             Quantity_ColorRGBA& color) const
{
  bool isColorFound = false;

  Handle(XCAFDoc_ColorTool) colorTool = this->GetColorTool();
  Handle(XCAFDoc_ShapeTool) shapeTool = this->GetShapeTool();

  if ( !colorTool.IsNull() )
  {
    // Get the source label.
    TDF_Label refLabel = label;
    //
    if ( shapeTool->IsReference(label) )
      shapeTool->GetReferredShape(label, refLabel);

    // Get one of the possibly available colors.
    isColorFound = colorTool->GetColor(refLabel, XCAFDoc_ColorSurf, color);
    //
    if ( !isColorFound )
      isColorFound = colorTool->GetColor(refLabel, XCAFDoc_ColorGen, color);
    //
    if ( !isColorFound )
      isColorFound = colorTool->GetColor(refLabel, XCAFDoc_ColorCurv, color);
  }

  return isColorFound;
}

//-----------------------------------------------------------------------------

bool asiAsm_XdeDoc::GetSubShapeColor(const asiAsm_XdePartId& partId,
                                     const TopoDS_Shape&     subShape,
                                     Quantity_ColorRGBA&     color) const
{
  Quantity_ColorRGBA partColor;
  //
  if ( this->GetColor(partId, partColor) )
    return false;

  Handle(XCAFDoc_ShapeTool) shapeTool = this->GetShapeTool();
  Handle(XCAFDoc_ColorTool) colorTool = this->GetColorTool();

  // Get part label.
  TDF_Label partLab = this->GetLabel(partId);

  // Find the subshape's attachment label.
  TDF_Label subShapeL;
  if ( !shapeTool->FindSubShape(partLab, subShape, subShapeL) )
  {
    return false;
  }

  // Get the color's attachment label.
  TDF_Label subShapeColorLab;
  if ( !colorTool->GetColor(subShapeL, XCAFDoc_ColorSurf, subShapeColorLab) &&
       !colorTool->GetColor(subShapeL, XCAFDoc_ColorGen,  subShapeColorLab) )
  {
    return false;
  }

  // Get color.
  Quantity_Color storedColor;
  colorTool->GetColor(subShapeColorLab, storedColor);

  color.SetRGB(storedColor);
  return true;
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::DumpAssemblyItems(Standard_OStream& out) const
{
  for ( asiAsm_XdeDocIterator ait(this); ait.More(); ait.Next() )
  {
    asiAsm_XdeAssemblyItemId item = ait.Current();

    // Get direct label.
    TDF_Label L;
    const TCollection_AsciiString& entry = item.GetLastEntry();
    TDF_Tool::Label(m_doc->GetData(), entry, L);

    // Get label of the referenced original.
    if ( this->GetShapeTool()->IsReference(L) ) // If true, then it is a reference to part + location.
    {
      TDF_Label refLabel;
      if ( this->GetShapeTool()->GetReferredShape(L, refLabel) ) // Get the real underlying part.
        L = refLabel;
    }

    // Retrieve name.
    TCollection_AsciiString name;
    Handle(TDataStd_Name) nodeName;
    //
    if ( L.FindAttribute(TDataStd_Name::GetID(), nodeName) )
      name = nodeName->Get();
    else
      name = "Unnamed";

    out << name.ToCString() << ", " << item.ToString().ToCString() << "\n";
  }
}

//-----------------------------------------------------------------------------

Handle(TDocStd_Document)& asiAsm_XdeDoc::ChangeDocument()
{
  return m_doc;
}

//-----------------------------------------------------------------------------

const Handle(TDocStd_Document)& asiAsm_XdeDoc::GetDocument() const
{
  return m_doc;
}

//-----------------------------------------------------------------------------

Handle(XCAFDoc_ShapeTool) asiAsm_XdeDoc::GetShapeTool() const
{
  return XCAFDoc_DocumentTool::ShapeTool( m_doc->Main() );
}

//-----------------------------------------------------------------------------

Handle(XCAFDoc_ColorTool) asiAsm_XdeDoc::GetColorTool() const
{
  return XCAFDoc_DocumentTool::ColorTool( m_doc->Main() );
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::init(const Handle(TDocStd_Document)& doc)
{
  // Store the pointer to the passed Document in the member field.
  m_doc = doc;
}

//-----------------------------------------------------------------------------

Handle(TDocStd_Document) asiAsm_XdeDoc::newDocument()
{
  Handle(TDocStd_Document) D;
  Handle(asiAsm_XdeApp)    A = this->getApplication();

  // Create XDE Document and return.
  A->NewDocument(BinXCAF, D);
  return D;
}

//-----------------------------------------------------------------------------

Handle(asiAsm_XdeApp) asiAsm_XdeDoc::getApplication()
{
  return asiAsm_XdeApp::Instance();
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::getLeafItems(asiAsm_XdeAssemblyItemId                     parent,
                                 const Handle(asiAsm_XdeHAssemblyItemIdsMap)& itemsMap,
                                 asiAsm_XdeAssemblyItemIds&                   items,
                                 const Handle(asiAsm_XdeHAssemblyItemIdsMap)& traversed) const
{
  // Check current assembly item id to be traversed.
  if ( traversed->Contains(parent) )
    return;

  // Take out shape tool
  Handle(XCAFDoc_ShapeTool) STool = this->GetShapeTool();

  // Loop over the parts and sub-assemblies in depth-first order.
  // ...

  std::stack<asiAsm_XdeAssemblyItemId> m_fringe;

  TDF_Tool::Label(m_doc->GetData(), parent.GetLastEntry(), parent.m_label);
  //
  if ( parent.m_label.IsNull() )
    return;

  m_fringe.push(parent);

  TCollection_AsciiString entry;

  while ( !m_fringe.empty() )
  {
    asiAsm_XdeAssemblyItemId topId = m_fringe.top();
    m_fringe.pop();

    const int numTraversed = traversed->Size();
    const int idx          = traversed->Add(topId);

    if ( numTraversed >= idx )
      continue;

    TDF_Label original;

    // Check if the current item is a reference to part (normally solid)
    if ( ( __isInstance(STool, topId.m_label, original) && STool->IsSimpleShape(original) ) ||
         ( STool->IsSimpleShape(topId.m_label) ) )
    {
      if ( itemsMap.IsNull() )
        items.Append(topId);
      else
        itemsMap->Add(topId);
    }
    else
    {
      const TDF_Label& labelToContinue = original.IsNull() ? topId.m_label
                                                           : original;

      std::vector<TDF_Label> components;
      if ( STool->IsAssembly(labelToContinue) )
        __getComponents(labelToContinue, components);
      else if ( STool->IsComponent(labelToContinue) )
        components.push_back(labelToContinue);

      for ( int l = int( components.size() ) - 1; l >= 0; --l )
      {
        // Set labels to iterate
        const TDF_Label& label = components[l];

        // Get entry to form assembly item ID.
        this->__entry(label, entry);
        //
        asiAsm_XdeAssemblyItemId result;
        result.m_label = label;
        result << topId.ToString();
        result << entry;

        m_fringe.push(result);
      }
    }
  }
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::getParts(const TDF_LabelSequence& originals,
                             asiAsm_XdePartIds&       parts) const
{
  // Convert labels to part IDs
  for ( TDF_LabelSequence::Iterator it(originals); it.More(); it.Next() )
  {
    // Part ID is nothing but an entry
    TCollection_AsciiString partId;
    TDF_Tool::Entry(it.Value(), partId);
    //
    parts.Append( asiAsm_XdePartId::FromEntry(partId) );
  }
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::getPartsWithInstances(const asiAsm_XdeLabelsToInstancesMap& origInstances,
                                          asiAsm_XdePartsToInstancesMap&        partsInstances) const
{
  // Convert labels to part IDs
  asiAsm_XdeLabelsToInstancesMap::Iterator it(origInstances);
  for ( ; it.More(); it.Next() )
  {
    const TDF_Label&              original  = it.Key();
    asiAsm_XdeAssemblyItemIdList& instances = it.ChangeValue();

    // Part ID is nothing but an entry
    TCollection_AsciiString partId;
    TDF_Tool::Entry(original, partId);
    //
    const int ind = partsInstances.Add( asiAsm_XdePartId::FromEntry(partId),
                                        asiAsm_XdeAssemblyItemIdList() );
    partsInstances(ind).Append(instances);
  }
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::getOriginalsWithInstances(const asiAsm_XdeAssemblyItemId& item,
                                              asiAsm_XdeLabelsToInstancesMap& origInstances) const
{
  TDF_Label original = this->GetOriginal( item );
  asiAsm_XdeAssemblyItemIdList* instances = origInstances.ChangeSeek(original);
  //
  if ( instances == 0L )
  {
    const int ind = origInstances.Add( original, asiAsm_XdeAssemblyItemIdList() );
    instances     = &origInstances.ChangeFromIndex(ind);
  }
  instances->Append( item );
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::getAssemblyItemsForPart(const TDF_Label&                             original,
                                            const asiAsm_XdeAssemblyItemId&              item,
                                            const Handle(asiAsm_XdeHAssemblyItemIdsMap)& itemsMap,
                                            asiAsm_XdeAssemblyItemIds&                   items) const
{
  TDF_Label itemLab = this->GetLabel(item);
  TDF_Label itemOriginalLab;

  // We take original label of the assembly item to compare it to the
  // label in question. If the leaf item is actually a part, we take the
  // part itself.
  if ( !this->IsInstance(itemLab, itemOriginalLab) )
  {
    if ( this->IsPart(itemLab) )
      itemOriginalLab = itemLab;
    else
      return;
  }

  if ( itemOriginalLab == original )
  {
    if ( !itemsMap.IsNull() )
      itemsMap->Add(item);
    else
      items.Append(item);
  }
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::clearSession(const Handle(XSControl_WorkSession)& WS)
{
  if ( WS.IsNull() )
    return;

  // Clear transfer reader.
  Handle(XSControl_TransferReader) transferReader = WS->TransferReader();
  if ( !transferReader.IsNull() )
    transferReader->Clear(-1);
}

//-----------------------------------------------------------------------------
// Methods with improved efficiency
//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::__getComponents(const TDF_Label&        l,
                                    std::vector<TDF_Label>& labels) const
{
  for ( TDF_ChildIterator lit(l); lit.More(); lit.Next() )
  {
    TDF_Label compLab = lit.Value();
    if (compLab.HasAttribute())
      labels.push_back(compLab);
  }
}

//-----------------------------------------------------------------------------

void asiAsm_XdeDoc::__entry(const TDF_Label&         label,
                            TCollection_AsciiString& entry) const
{
  if ( label.IsNull() )
    return;

  const TCollection_AsciiString* pEntry = m_LECache.Seek(label);
  if ( pEntry )
  {
    entry = *pEntry;
    return;
  }

  if ( !entry.IsEmpty() )
    entry.Clear();

  TColStd_ListOfInteger tags;
  TDF_Tool::TagList(label, tags);

  const int numTags = tags.Size();
  int idx = 1;
  for ( TColStd_ListOfInteger::Iterator it(tags); it.More(); it.Next(), ++idx )
  {
    entry += it.Value();
    //
    if ( idx < numTags )
      entry += ":";
  }

  m_LECache.Bind(label, entry);
}

//-----------------------------------------------------------------------------

bool asiAsm_XdeDoc::__isInstance(const Handle(XCAFDoc_ShapeTool)& ST,
                                 const TDF_Label&                 itemLab,
                                 TDF_Label&                       originLab) const
{
  if ( ST->IsReference(itemLab) )
  {
    Handle(TDataStd_TreeNode) JumpNode;
    itemLab.FindAttribute(XCAFDoc::ShapeRefGUID(), JumpNode);
    //
    if ( JumpNode->HasFather() )
      originLab = JumpNode->Father()->Label(); // Declaration-level origin.

    return true;
  }
  return false;
}
