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
#include <asiAlgo_Version.h>

// OpenCascade includes
#include <APIHeaderSection_MakeHeader.hxx>
#include <BRep_Builder.hxx>
#include <CDM_MetaData.hxx>
#include <gp_Quaternion.hxx>
#include <IGESCAFControl_Reader.hxx>
#include <Interface_Static.hxx>
#include <Quantity_ColorRGBA.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <STEPCAFControl_Writer.hxx>
#include <STEPControl_Controller.hxx>
#include <TColStd_HSequenceOfExtendedString.hxx>
#include <TDataStd_ChildNodeIterator.hxx>
#include <TDataStd_Name.hxx>
#include <TDataStd_TreeNode.hxx>
#include <TDataStd_UAttribute.hxx>
#include <TDF_AttributeIterator.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_RelocationTable.hxx>
#include <TNaming_Builder.hxx>
#include <TNaming_NamedShape.hxx>
#include <TNaming_Tool.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_MapOfOrientedShape.hxx>
#include <XCAFDoc.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_GraphNode.hxx>
#include <XCAFDoc_LayerTool.hxx>
#include <XCAFDoc_Location.hxx>
#include <XCAFDoc_MaterialTool.hxx>
#include <XCAFDoc_ShapeMapTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XSControl_TransferReader.hxx>
#include <XSControl_WorkSession.hxx>

//-----------------------------------------------------------------------------

using namespace asiAsm::xde;

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

Doc::Doc(ActAPI_ProgressEntry progress,
         ActAPI_PlotterEntry  plotter)
: Standard_Transient (),
  m_progress         (progress),
  m_plotter          (plotter)
{
  this->NewDocument();
}

//-----------------------------------------------------------------------------

void Doc::NewDocument()
{
  this->init( this->newDocument() ); // Initialize internal structure.
}

//-----------------------------------------------------------------------------

bool Doc::Load(const TCollection_AsciiString& filename)
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

bool Doc::LoadNative(const TCollection_AsciiString& filename)
{
  if ( !m_doc.IsNull() )
    this->Release();

  Handle(App) A = this->getApplication();
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
  catch ( Standard_Failure& exc )
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

bool Doc::LoadSTEP(const TCollection_AsciiString& filename)
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
      m_progress.SendLogMessage(LogErr(Normal) << "STEP reader failed (error occurred transferring STEP model to XDE)." );
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

bool Doc::LoadIGES(const TCollection_AsciiString& filename)
{
  if ( m_doc.IsNull() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Cannot load into null Document.");
    return false;
  }

  IGESCAFControl_Reader reader;
  Handle(XSControl_WorkSession) WS = reader.WS();

  // Read CAD and associated data from file.
  try
  {
    // Read file.
    IFSelect_ReturnStatus outcome = reader.ReadFile( filename.ToCString() );
    //
    if ( outcome != IFSelect_RetDone )
    {
      m_progress.SendLogMessage(LogErr(Normal) << "Cannot read IGES file from disk." );
      //
      this->clearSession(WS);
      return false;
    }

    // Transfer data.
    if ( !reader.Transfer(m_doc) )
    {
      m_progress.SendLogMessage(LogErr(Normal) << "IGES reader failed (error occurred transferring IGES model to XDE)." );
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
    m_progress.SendLogMessage(LogErr(Normal) << "IGES reader failed (exception occurred).");
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------------

bool Doc::SaveAs(const TCollection_AsciiString& filename)
{
  if ( m_doc.IsNull() )
    return false;

  Handle(App) A = this->getApplication();

  // Write.
  PCDM_StoreStatus status = PCDM_SS_WriteFailure;
  //
  try
  {
    status = A->SaveAs(m_doc, filename);
  }
  catch ( Standard_Failure& exc )
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

bool Doc::SaveSTEP(const TCollection_AsciiString& filename)
{
  STEPControl_Controller::Init();
  try
  {
    STEPCAFControl_Writer writer;
    {
      // Save information
      Handle(StepData_StepModel) stepModel = writer.ChangeWriter().Model();
      if (stepModel.IsNull())
        return false;

      APIHeaderSection_MakeHeader headerMaker(stepModel);

      Handle(TCollection_HAsciiString) author = new TCollection_HAsciiString(ASITUS_APP_NAME);
      Handle(TCollection_HAsciiString) originatingSystem = new TCollection_HAsciiString(ASITUS_APP_NAME);
      Handle(TCollection_HAsciiString) organization = new TCollection_HAsciiString(ASITUS_APP_NAME);

      headerMaker.SetAuthorValue(1, author);
      headerMaker.SetOriginatingSystem(originatingSystem);
      headerMaker.SetOrganizationValue(1, organization);

      STEPControl_StepModelType mode = STEPControl_AsIs;
      switch (Interface_Static::IVal("write.step.mode"))
      {
        default:
        case 0: mode = STEPControl_AsIs;                   break;
        case 1: mode = STEPControl_FacetedBrep;            break;
        case 2: mode = STEPControl_ShellBasedSurfaceModel; break;
        case 3: mode = STEPControl_ManifoldSolidBrep;      break;
        case 4: mode = STEPControl_GeometricCurveSet;      break;
      }

      Standard_CString multiFile = NULL;
      int              extMode = Interface_Static::IVal("write.step.extern.mode");
      //
      if (extMode != 0)
      {
        // get prefix for file
        multiFile = Interface_Static::CVal("write.step.extern.prefix");
      }

      // Disable writing of GDT if not AP242
      int ap = Interface_Static::IVal("write.step.schema");
      if (ap != 5)
        writer.SetDimTolMode(0);

      if (!writer.Transfer(m_doc, mode, multiFile))
      {
        m_progress.SendLogMessage(LogErr(Normal) << "Transfer failed.");
        return false;
      }
    }
    {
      m_progress.SetMessageKey("Flush model into file");
      if (writer.Write(filename.ToCString()) != IFSelect_RetDone)
      {
        m_progress.SendLogMessage(LogErr(Normal) << "STEP writer failed (error while flushing produced model into file).");
        return false;
      }
    }
    return true;
  }
  catch ( ... )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "STEP writer failed (exception occurred).");
    return false;
  }
}

//-----------------------------------------------------------------------------

bool Doc::IsEmpty() const
{
  if ( m_doc.IsNull() )
    return true;

  TDF_ChildIterator cit( m_doc->Main() );
  const bool isDocEmpty = !cit.More();

  return isDocEmpty;
}

//-----------------------------------------------------------------------------

void Doc::Release()
{
  if ( m_doc.IsNull() )
    return;

  // Close OCAF Document.
  Handle(App) A = this->getApplication();
  //
  if ( A->CanClose(m_doc) == CDM_CCS_OK )
    A->Close(m_doc);
  //
  m_doc.Nullify();

  // Clear cache.
  m_LECache.Clear();
}

//-----------------------------------------------------------------------------

bool Doc::FindItems(const std::string&            name,
                    Handle(HAssemblyItemIdsMap)& items) const
{
  items = new HAssemblyItemIdsMap;

  // Prepare assembly graph.
  Handle(Graph) asmGraph = new Graph(this);

  // DFS starting from roots.
  const TColStd_PackedMapOfInteger& roots = asmGraph->GetRoots();
  //
  for ( TColStd_MapIteratorOfPackedMapOfInteger rit(roots); rit.More(); rit.Next() )
  {
    const int rootId = rit.Key();

    std::vector<int> path = {rootId};

    this->findItemsRecursively(asmGraph, rootId, name, path, items);
  }

  return !items->IsEmpty();
}

//-----------------------------------------------------------------------------

void Doc::SetObjectName(const TDF_Label&                  label,
                        const TCollection_ExtendedString& name)
{
  TDataStd_Name::Set(label, name);
}

//-----------------------------------------------------------------------------

bool Doc::GetObjectName(const PersistentId&         id,
                        TCollection_ExtendedString& name) const
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

bool Doc::GetObjectName(const PartId&               id,
                        TCollection_ExtendedString& name) const
{
  name = this->GetPartName(id);
  return true;
}

//-----------------------------------------------------------------------------

bool Doc::GetObjectName(const AssemblyItemId&       id,
                        TCollection_ExtendedString& name) const
{
  TDF_Label original = this->GetOriginal(id);

  TCollection_AsciiString oEntry;
  TDF_Tool::Entry(original, oEntry);

  return this->GetObjectName(oEntry, name);
}
//-----------------------------------------------------------------------------

TCollection_ExtendedString
  Doc::GetObjectName(const TDF_Label& label) const
{
  // Get name directly from OCAF attribute.
  Handle(TDataStd_Name) nameAttr;
  if ( !label.FindAttribute(TDataStd_Name::GetID(), nameAttr) )
    return TCollection_ExtendedString("<unnamed>");

  return nameAttr->Get();
}

//-----------------------------------------------------------------------------

TCollection_ExtendedString
  Doc::GetPartName(const PartId& part) const
{
  // Get label by part ID.
  TDF_Label label;
  TDF_Tool::Label(this->m_doc->GetData(), part.Entry, label);

  // Get name.
  return this->GetObjectName(label);
}

//-----------------------------------------------------------------------------

void Doc::GetPartRepresentations(const PartId&                  partId,
                                 std::vector<Handle(PartRepr)>& reps) const
{
  this->GetPartRepresentations(this->GetLabel(partId), reps);
}

//-----------------------------------------------------------------------------

void Doc::GetPartRepresentations(const TDF_Label&               label,
                                 std::vector<Handle(PartRepr)>& reps) const
{
  if ( label.IsNull() ) return; // Contract check.

  // Iterate over the part's attributes and use the representation factory
  // to construct representations for the attributes that allow doing this.
  for ( TDF_AttributeIterator itAtt(label); itAtt.More(); itAtt.Next() )
  {
    const Handle(TDF_Attribute)& attr = itAtt.Value();

    // Construct representation.
    Handle(PartRepr) repr = PartReprFactory::New(attr);
    //
    if ( !repr.IsNull() )
      reps.push_back(repr);
  }
}

//-----------------------------------------------------------------------------

bool Doc::GetPartRepresentation(const PartId&        partId,
                                const Standard_GUID& guid,
                                Handle(PartRepr)&    rep) const
{
  TDF_Label partLab = this->GetLabel(partId);

  // The representation's GUID is identical to the attribute's GUID.
  Handle(TDF_Attribute) attr;
  //
  if ( !partLab.FindAttribute(guid, attr) )
    return false;

  // Construct a representation using the factory.
  rep = PartReprFactory::New(attr);
  //
  return !rep.IsNull();
}

//-----------------------------------------------------------------------------

bool Doc::IsAssembly(const TDF_Label& itemLabel) const
{
  if ( itemLabel.IsNull() )
    return false;

  return this->GetShapeTool()->IsAssembly(itemLabel);
}

//-----------------------------------------------------------------------------

bool Doc::IsAssembly(const AssemblyItemId& item) const
{
  return this->IsAssembly( this->GetOriginal(item) );
}

//-----------------------------------------------------------------------------

bool Doc::IsInstance(const TDF_Label& itemLab,
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

bool Doc::IsInstance(const AssemblyItemId& item,
                     TDF_Label&            origin) const
{
  TDF_Label label = this->GetLabel(item);
  //
  return this->IsInstance(label, origin);
}

//-----------------------------------------------------------------------------

bool Doc::IsPart(const TDF_Label& label) const
{
  if ( label.IsNull() )
    return false;

  // Check with shape tool.
  Handle(XCAFDoc_ShapeTool) STool = this->GetShapeTool();
  //
  return STool->IsSimpleShape(label);
}

//-----------------------------------------------------------------------------

bool Doc::IsPart(const AssemblyItemId& item) const
{
  // Check original on being a simple shape.
  TDF_Label original = this->GetOriginal(item);
  //
  return this->IsPart(original);
}

//-----------------------------------------------------------------------------

bool Doc::IsOriginal(const TDF_Label& label) const
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

TDF_Label Doc::GetOriginal(const TDF_Label& itemLabel) const
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

TDF_Label Doc::GetOriginal(const AssemblyItemId& item) const
{
  return this->GetOriginal( this->GetLabel(item) );
}

//-----------------------------------------------------------------------------

PartId
  Doc::GetPart(const AssemblyItemId& item) const
{
  TDF_Label prototypeLab = this->GetOriginal(item);
  //
  if ( prototypeLab.IsNull() )
    return PartId();

  TCollection_AsciiString entry;
  __entry(prototypeLab, entry);

  return PartId::FromEntry(entry);
}

//-----------------------------------------------------------------------------

void Doc::GetOriginals(const Handle(HAssemblyItemIdsMap)& anyItems,
                       TDF_LabelSequence&                 originalLabels) const
{
  // Loop over the items of interest and collect originals as labels into
  // a map, so that no duplications will be stored.
  NCollection_IndexedMap<TDF_Label, TDF_LabelMapHasher> originals;
  //
  for ( HAssemblyItemIdsMap::Iterator iter(*anyItems); iter.More(); iter.Next() )
  {
    TDF_Label original = this->GetOriginal( iter.Value() );

    const int numOriginals = originals.Size();
    const int idxOriginal  = originals.Add(original);

    if ( numOriginals < idxOriginal )
      originalLabels.Append(original);
  }
}

//-----------------------------------------------------------------------------

void Doc::GetOriginals(const AssemblyItemIds& anyItems,
                       TDF_LabelSequence&     originalLabels) const
{
  // Loop over the items of interest and collect originals as labels into
  // a map, so that no duplications will be stored.
  NCollection_IndexedMap<TDF_Label, TDF_LabelMapHasher> originals;
  //
  for ( AssemblyItemIds::Iterator iter(anyItems); iter.More(); iter.Next() )
  {
    TDF_Label original = this->GetOriginal( iter.Value() );

    const int numOriginals = originals.Size();
    const int idxOriginal  = originals.Add(original);

    if ( numOriginals < idxOriginal )
      originalLabels.Append(original);
  }
}

//-----------------------------------------------------------------------------

void Doc::GetOriginalsWithInstances(const Handle(HAssemblyItemIdsMap)& anyItems,
                                    LabelsToInstancesMap&              origInstances) const
{
  // Loop over the items of interest and collect originals as labels into
  // a map, so that no duplications will be stored.
  for ( HAssemblyItemIdsMap::Iterator iter(*anyItems); iter.More(); iter.Next() )
    this->getOriginalsWithInstances(iter.Value(), origInstances);
}

//-----------------------------------------------------------------------------

void Doc::GetOriginalsWithInstances(const AssemblyItemIds& anyItems,
                                    LabelsToInstancesMap&  origInstances) const
{
  // Loop over the items of interest and collect originals as labels into
  // a map, so that no duplications will be stored.
  for ( AssemblyItemIds::Iterator iter(anyItems); iter.More(); iter.Next() )
    this->getOriginalsWithInstances(iter.Value(), origInstances);
}

//-----------------------------------------------------------------------------

void Doc::GetParts(PartIds& parts) const
{
  AssemblyItemIds parents;
  this->GetRootAssemblyItems(parents);
  //
  this->GetParts(parents, parts, false);
}

//-----------------------------------------------------------------------------

void Doc::GetParts(const Handle(HAssemblyItemIdsMap)& anyItems,
                   PartIds&                           parts,
                   const bool                         isAlreadyLeafs) const
{
  // Get leaf assembly items.
  Handle(HAssemblyItemIdsMap) leafItems;
  if ( !isAlreadyLeafs )
  {
    leafItems = new HAssemblyItemIdsMap;
    this->GetLeafAssemblyItems(anyItems, leafItems);
  }

  // Get all original labels.
  TDF_LabelSequence originals;
  this->GetOriginals(isAlreadyLeafs ? anyItems : leafItems, originals);
  //
  this->getParts(originals, parts);
}

//-----------------------------------------------------------------------------

void Doc::GetParts(const AssemblyItemIds& anyItems,
                   PartIds&               parts,
                   const bool             isAlreadyLeafs) const
{
  if ( anyItems.IsEmpty() )
  {
    // Fallback to gathering parts over the entire model.
    this->GetParts(parts);
    return;
  }

  // Get leaf assembly items.
  AssemblyItemIds leafItems;
  if ( !isAlreadyLeafs )
    this->GetLeafAssemblyItems(anyItems, leafItems);

  // Get all original labels.
  TDF_LabelSequence originals;
  this->GetOriginals(isAlreadyLeafs ? anyItems : leafItems, originals);
  //
  this->getParts(originals, parts);
}

//-----------------------------------------------------------------------------

void Doc::GetParts(const AssemblyItemIds&       anyItems,
                   Handle(HAssemblyItemIdsMap)& leafItems,
                   PartIds&                     parts) const
{
  // Get leaf assembly items.
  this->GetLeafAssemblyItems(anyItems, leafItems);
  //
  this->GetParts(leafItems, parts, true);
}

//-----------------------------------------------------------------------------

void Doc::GetParts(const Handle(HAssemblyItemIdsMap)& anyItems,
                   Handle(HAssemblyItemIdsMap)&       leafItems,
                   PartIds&                           parts) const
{
  // Get leaf assembly items.
  this->GetLeafAssemblyItems(anyItems, leafItems);
  //
  this->GetParts(leafItems, parts, true);
}

//-----------------------------------------------------------------------------

void Doc::GetParts(const AssemblyItemIds& anyItems,
                   AssemblyItemIds&       leafItems,
                   PartIds&               parts) const
{
  // Get leaf assembly items.
  this->GetLeafAssemblyItems(anyItems, leafItems);
  //
  this->GetParts(leafItems, parts, true);
}

//-----------------------------------------------------------------------------

void Doc::CountParts(NCollection_DataMap<PartId, int, PartId::Hasher>& quantities) const
{
  // Construct HAG.
  Handle(Graph) hag = new Graph(this);

  // Loop over the HAG elements and collect the information for the elements
  // of PART type. We are interested in their usage occurrences.
  const NCollection_IndexedMap<PersistentId>& elems = hag->GetNodes();
  //
  for ( int nid = 1; nid <= elems.Extent(); ++nid )
  {
    if ( hag->GetNodeType(nid) != Graph::NodeType_Part )
      continue;

    const PersistentId& pid      = elems(nid);
    const int           quantity = hag->GetUsageOccurrenceQuantity(nid);

    quantities.Bind(pid, quantity);
  }
}

//-----------------------------------------------------------------------------

void Doc::GetPartsWithInstances(const Handle(HAssemblyItemIdsMap)& anyItems,
                                PartsToInstancesMap&               partsInstances,
                                const bool                         isAlreadyLeafs) const
{
  // Get leaf assembly items.
  Handle(HAssemblyItemIdsMap) leafItems;
  if ( !isAlreadyLeafs )
  {
    leafItems = new HAssemblyItemIdsMap;
    this->GetLeafAssemblyItems(anyItems, leafItems);
  }

  // Get map of original labels to instances.
  LabelsToInstancesMap origInstances;
  this->GetOriginalsWithInstances(isAlreadyLeafs ? anyItems : leafItems, origInstances);
  //
  this->getPartsWithInstances(origInstances, partsInstances);
}

//-----------------------------------------------------------------------------

void Doc::GetPartsWithInstances(const AssemblyItemIds& anyItems,
                                PartsToInstancesMap&   partsInstances,
                                const bool             isAlreadyLeafs) const
{
  // Get leaf assembly items.
  AssemblyItemIds leafItems;
  if ( !isAlreadyLeafs )
    this->GetLeafAssemblyItems(anyItems, leafItems);

  // Get map of original labels to instances.
  LabelsToInstancesMap origInstances;
  this->GetOriginalsWithInstances(isAlreadyLeafs ? anyItems : leafItems, origInstances);
  //
  this->getPartsWithInstances(origInstances, partsInstances);
}

//-----------------------------------------------------------------------------

void Doc::GetAsPartId(const AssemblyItemId& assemblyEntryId,
                      PartId&               partEntryId)
{
  TDF_Label original = this->GetOriginal(assemblyEntryId);
  TDF_Tool::Entry(original, partEntryId.Entry);
}

//-----------------------------------------------------------------------------

TDF_Label Doc::GetLabelOfModel() const
{
  return m_doc->Main().Root();
}

//-----------------------------------------------------------------------------

TDF_Label Doc::GetLabel(const AssemblyItemId& item) const
{
  if ( item.IsNull() )
    return TDF_Label();

  const TCollection_AsciiString& entry = item.GetLastEntry();

  return this->GetLabel(entry);
}

//-----------------------------------------------------------------------------

TDF_Label Doc::GetLabel(const PartId& part) const
{
  if ( part.IsNull() )
    return TDF_Label();

  return this->GetLabel(part.Entry);
}

//-----------------------------------------------------------------------------

TDF_Label Doc::GetLabel(const PersistentId& id) const
{
  if ( id.IsEmpty() )
    return TDF_Label();

  TDF_Label label;
  TDF_Tool::Label(this->m_doc->GetData(), id, label);

  return label;
}

//-----------------------------------------------------------------------------

TopoDS_Shape Doc::GetShape(const AssemblyItemId& item,
                           const bool            doTransform) const
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

TopoDS_Shape Doc::GetShape(const PartId& part) const
{
  TDF_Label L;
  TDF_Tool::Label(this->m_doc->GetData(), part.Entry, L);

  Handle(TNaming_NamedShape) NS;
  if ( !L.FindAttribute(TNaming_NamedShape::GetID(), NS) )
    return TopoDS_Shape();

  return TNaming_Tool::GetShape(NS);
}

//-----------------------------------------------------------------------------

TopoDS_Shape Doc::GetShape(const TDF_Label& label) const
{
  if ( label.IsNull() )
    return TopoDS_Shape();

  return this->GetShapeTool()->GetShape(label);
}

//-----------------------------------------------------------------------------

bool Doc::GetParent(const AssemblyItemId& item,
                    AssemblyItemId&       parent) const
{
  if ( item.GetPathLength() <= 1 )
    return false;

  parent = item;
  parent.Remove( item.GetPathLength() );
  return true;
}

//-----------------------------------------------------------------------------

TopLoc_Location
  Doc::GetParentLocation(const AssemblyItemId& item,
                         const bool            doTransform) const
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

TopLoc_Location Doc::GetOwnLocation(const AssemblyItemId& item) const
{
  return this->GetShapeTool()->GetLocation( this->GetLabel(item) );
}

//-----------------------------------------------------------------------------

TopoDS_Shape Doc::GetOneShape() const
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

TopoDS_Shape Doc::GetOneShape(const AssemblyItemIds& items) const
{
  // Create result container.
  TopoDS_Compound result;
  BRep_Builder BB;
  BB.MakeCompound(result);

  // Extract shapes.
  for ( AssemblyItemIds::Iterator iter(items); iter.More(); iter.Next() )
  {
    TopoDS_Shape shape = this->GetShape( iter.Value() );
    BB.Add(result, shape);
  }
  return result;
}

//-----------------------------------------------------------------------------

void Doc::GetLabelsOfRoots(TDF_LabelSequence& labels) const
{
  Handle(XCAFDoc_ShapeTool) STool = this->GetShapeTool();
  STool->GetFreeShapes(labels);
}

//-----------------------------------------------------------------------------

void Doc::GetRootAssemblyItems(AssemblyItemIds& items) const
{
  TDF_LabelSequence roots;
  this->GetLabelsOfRoots(roots);

  items.Clear();

  for ( TDF_LabelSequence::Iterator it(roots); it.More(); it.Next() )
  {
    TCollection_AsciiString entry;
    TDF_Tool::Entry(it.Value(), entry);

    AssemblyItemId item = AssemblyItemId::FromPath(entry);
    items.Append(item);
  }
}

//-----------------------------------------------------------------------------

void Doc::GetLeafAssemblyItems(AssemblyItemIds& items) const
{
  AssemblyItemIds parents;
  this->GetRootAssemblyItems(parents);
  //
  this->GetLeafAssemblyItems(parents, items);
}

//-----------------------------------------------------------------------------

void Doc::GetLeafAssemblyItems(const Handle(HAssemblyItemIdsMap)& items) const
{
  AssemblyItemIds parents;
  this->GetRootAssemblyItems(parents);
  //
  this->GetLeafAssemblyItems(parents, items);
}

//-----------------------------------------------------------------------------

void Doc::GetLeafAssemblyItems(const AssemblyItemIds&             parents,
                               const Handle(HAssemblyItemIdsMap)& items) const
{
  AssemblyItemIds dumpItems;
  Handle(HAssemblyItemIdsMap) traversed = new HAssemblyItemIdsMap;

  for ( AssemblyItemIds::Iterator ait(parents); ait.More(); ait.Next() )
    this->getLeafItems(ait.Value(), items, dumpItems, traversed);
}

//-----------------------------------------------------------------------------

void Doc::GetLeafAssemblyItems(const Handle(HAssemblyItemIdsMap)& parents,
                               const Handle(HAssemblyItemIdsMap)& items) const
{
  AssemblyItemIds dumpItems;
  Handle(HAssemblyItemIdsMap) traversed = new HAssemblyItemIdsMap();

  for ( HAssemblyItemIdsMap::Iterator ait(*parents); ait.More(); ait.Next() )
    this->getLeafItems(ait.Value(), items, dumpItems, traversed);
}

//-----------------------------------------------------------------------------

void Doc::GetLeafAssemblyItems(const AssemblyItemId&              parent,
                               const Handle(HAssemblyItemIdsMap)& items) const
{
  AssemblyItemIds oneElemList;
  oneElemList.Append(parent);
  this->GetLeafAssemblyItems(oneElemList, items);
}

//-----------------------------------------------------------------------------

void Doc::GetLeafAssemblyItems(const AssemblyItemId& parent,
                               AssemblyItemIds&      items) const
{
  AssemblyItemIds oneElemList;
  oneElemList.Append(parent);
  this->GetLeafAssemblyItems(oneElemList, items);
}

//-----------------------------------------------------------------------------

void Doc::GetLeafAssemblyItems(const AssemblyItemIds& parents,
                               AssemblyItemIds&       items) const
{
  if ( parents.IsEmpty() )
  {
    // Return all leaves.
    this->GetLeafAssemblyItems(items);
    return;
  }

  Handle(HAssemblyItemIdsMap) traversed = new HAssemblyItemIdsMap();

  for ( AssemblyItemIds::Iterator ait(parents); ait.More(); ait.Next() )
    this->getLeafItems(ait.Value(), NULL, items, traversed);
}

//-----------------------------------------------------------------------------

void Doc::GetLabelsOfReplicas(const TDF_Label&   label,
                              TDF_LabelSequence& replicas)
{
  Handle(TDataStd_TreeNode) TN;
  if ( !label.FindAttribute(XCAFDoc::ShapeRefGUID(), TN) )
    return;

  for ( TDataStd_ChildNodeIterator nit(TN); nit.More(); nit.Next() )
    replicas.Append( nit.Value()->Label() );
}

//-----------------------------------------------------------------------------

void Doc::GetAssemblyItemsForPart(const PartId&    part,
                                  AssemblyItemIds& items) const
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
    items.Append( AssemblyItemId(part.Entry) );
  else
    this->GetAssemblyItemsForPart(label, items);
}

//-----------------------------------------------------------------------------

void Doc::GetAssemblyItemsForPart(const TDF_Label& original,
                                  AssemblyItemIds& items) const
{
  AssemblyItemIds allItems;
  this->GetLeafAssemblyItems(allItems);

  // Choose proper ones.
  for ( AssemblyItemIds::Iterator it(allItems); it.More(); it.Next() )
    this->getAssemblyItemsForPart(original, it.Value(), NULL, items);
}

//-----------------------------------------------------------------------------

void Doc::GetAssemblyItemsForPart(const TDF_Label&                   original,
                                  const Handle(HAssemblyItemIdsMap)& items) const
{
  AssemblyItemIds allItems;
  this->GetLeafAssemblyItems(allItems);

  // Choose proper ones.
  AssemblyItemIds dumpItems;
  for ( AssemblyItemIds::Iterator it(allItems); it.More(); it.Next() )
    this->getAssemblyItemsForPart(original, it.Value(), items, dumpItems);
}

//-----------------------------------------------------------------------------

void Doc::GetAssemblyItemsForParts(const TDF_LabelMap& originals,
                                   AssemblyItemIds&    items) const
{
  AssemblyItemIds allItems;
  this->GetLeafAssemblyItems(allItems);

  // Choose proper ones.
  for ( AssemblyItemIds::Iterator it(allItems); it.More(); it.Next() )
  {
    const AssemblyItemId& item = it.Value();
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
      const AssemblyItemId& item = AssemblyItemId::FromPath(entry);
      items.Append(item);
    }
  }
}

//-----------------------------------------------------------------------------

void Doc::GetAssemblyItemsForParts(const PartIds&   parts,
                                   AssemblyItemIds& items) const
{
  TDF_LabelMap labelsMap;
  for ( PartIds::Iterator pit(parts); pit.More(); pit.Next() )
  {
    TDF_Label label;
    TDF_Tool::Label(this->m_doc->GetData(), pit.Value().Entry, label);

    if ( label.IsNull() )
      continue;

    // Handle cases when part is Free shape (in terms of ShapeTool),
    // i.e. has no instances.
    if ( this->GetShapeTool()->IsFree(label) )
      items.Append( AssemblyItemId(pit.Value().Entry) );
    else
      labelsMap.Add(label);
  }

  this->GetAssemblyItemsForParts(labelsMap, items);
}

//-----------------------------------------------------------------------------

void Doc::GetAssemblyItemsForParts(const TDF_LabelMap&                originals,
                                   const Handle(HAssemblyItemIdsMap)& items) const
{
  AssemblyItemIds itemList;
  this->GetAssemblyItemsForParts(originals, itemList);
  //
  for ( AssemblyItemIds::Iterator it(itemList); it.More(); it.Next() )
    items->Add( it.Value() );
}

//-----------------------------------------------------------------------------

void Doc::GetPartners(const AssemblyItemId& anyItem,
                      AssemblyItemIds&      partners) const
{
  this->GetPartners(this->GetOriginal(anyItem), partners);
}

//-----------------------------------------------------------------------------

void Doc::GetPartners(const TDF_Label& original,
                      AssemblyItemIds& partners) const
{
  // Loop over the parts and sub-assemblies in depth-first order.
  for ( DocIterator ait(this); ait.More(); ait.Next() )
  {
    AssemblyItemId currentItem     = ait.Current();
    TDF_Label      currentOriginal = this->GetOriginal(currentItem);

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

void Doc::GetPartners(const AssemblyItemIds& anyItems,
                      AssemblyItemIds&       partners) const
{
  TDF_LabelMap originals;
  for ( AssemblyItemIds::Iterator it(anyItems); it.More(); it.Next() )
  {
    originals.Add( this->GetOriginal( it.Value() ) );
  }
  this->GetAssemblyItemsForParts(originals, partners);
}

//-----------------------------------------------------------------------------

void Doc::GetPartners(const Handle(HAssemblyItemIdsMap)& anyItems,
                      Handle(HAssemblyItemIdsMap)&       partners) const
{
  TDF_LabelMap originals;
  for ( HAssemblyItemIdsMap::Iterator it(*anyItems); it.More(); it.Next() )
  {
    originals.Add( this->GetOriginal( it.Value() ) );
  }
  this->GetAssemblyItemsForParts(originals, partners);
}

//-----------------------------------------------------------------------------

void Doc::GetPartners(const Handle(HAssemblyItemIdsMap)& anyItems,
                      AssemblyItemIds&                   partners) const
{
  TDF_LabelMap originals;
  for ( int i = 1; i <= anyItems->Extent(); ++i )
  {
    originals.Add( this->GetOriginal( anyItems->FindKey(i) ) );
  }
  this->GetAssemblyItemsForParts(originals, partners);
}

//-----------------------------------------------------------------------------

bool Doc::GetColor(const PartId&   partId,
                   Quantity_Color& color) const
{
  Quantity_ColorRGBA colorRGBA;
  const bool isOk = this->GetColor(partId, colorRGBA);

  if ( isOk )
    color = colorRGBA.GetRGB();

  return isOk;
}

//-----------------------------------------------------------------------------

bool Doc::GetColor(const PartId&       partId,
                   Quantity_ColorRGBA& color) const
{
  TDF_Label label = this->GetLabel(partId);

  return this->GetColor(label, color);
}

//-----------------------------------------------------------------------------

bool Doc::GetColor(const TDF_Label&    label,
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

bool Doc::GetColorAlpha(const PartId& partId,
                        double&       alpha)
{
  Handle(XCAFDoc_ShapeTool) ST = this->GetShapeTool();

  // Get color for the part itself.
  Quantity_ColorRGBA colorRGBA;
  bool isColorFound = this->GetColor(partId, colorRGBA);

  if ( isColorFound )
  {
    alpha = colorRGBA.Alpha();
  }
  else /* Try getting color from any of the subshapes as a fallback solution */
  {
    TDF_LabelSequence subShapes;
    ST->GetSubShapes(this->GetLabel(partId), subShapes);
    //
    if ( subShapes.IsEmpty() )
      return isColorFound;

    TDF_LabelSequence::Iterator it(subShapes);
    //
    if ( it.More() )
      isColorFound = this->GetColor(it.Value(), colorRGBA);

    if ( isColorFound )
      alpha = colorRGBA.Alpha();
  }

  return isColorFound;
}

//-----------------------------------------------------------------------------

bool Doc::GetSubShapeColor(const PartId&       partId,
                           const TopoDS_Shape& subShape,
                           Quantity_ColorRGBA& color) const
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

bool Doc::ExpandCompound(const PartId& partId,
                         const bool    updateAssemblies)
{
  // Contract check 1: null or non-compound shapes are out of interest.
  TopoDS_Shape partShape = this->GetShape(partId);
  //
  if ( partShape.IsNull() || (partShape.ShapeType() != TopAbs_COMPOUND) )
  {
    return false;
  }

  // Contract check 2: empty compounds are out of interest either.
  if ( asiAlgo_Utils::IsEmptyShape(partShape) )
  {
    return false;
  }

  // Prepare working tools.
  Handle(XCAFDoc_ShapeTool) shapeTool = this->GetShapeTool();
  Handle(XCAFDoc_ColorTool) colorTool = this->GetColorTool();

  TDF_Label partLabel = this->GetLabel(partId);

  // Save subshapes of expanded part.
  t_expansionMap             oldSubshapes;
  TDF_LabelSequence          childrenToDelete;
  Handle(TNaming_NamedShape) nsAttribute;
  //
  for ( TDF_ChildIterator subIt(partLabel); subIt.More(); subIt.Next() )
  {
    TDF_Label curSubL = subIt.Value();

    // Skip empty labels.
    if ( !curSubL.FindAttribute(TNaming_NamedShape::GetID(), nsAttribute) )
      continue;

    TopoDS_Shape curShape = shapeTool->GetShape(curSubL);
    std::pair<TDF_Label, TopLoc_Location> sub( curSubL, curShape.Location() );

    if ( !oldSubshapes.IsBound( curShape.Located( TopLoc_Location() ) ) )
      oldSubshapes.Bind( curShape.Located( TopLoc_Location() ), sub);

    curSubL.ForgetAttribute( TNaming_NamedShape::GetID() );
    childrenToDelete.Append(curSubL);
  }

  // Expand on label.
  std::vector< std::pair<TDF_Label, TopLoc_Location> > newParts;
  TopLoc_Location auxLoc;
  //
  this->expand(partLabel, auxLoc, oldSubshapes, newParts);

  // Update attributes.
  for ( t_expansionMap::Iterator subIt(oldSubshapes); subIt.More(); subIt.Next() )
  {
    // Try to find as top level shape.
    TopoDS_Shape curShape  = subIt.Key();
    TDF_Label    newShapeL = shapeTool->FindShape(curShape);

    // Try to find as subshape.
    if ( newShapeL.IsNull() )
    {
      TopLoc_Location initLoc = subIt.Value().second;
      std::vector<std::pair<TDF_Label, TopLoc_Location>>::const_iterator partIt = newParts.cbegin();
      //
      for ( ; partIt != newParts.cend(); partIt++ )
      {
        TDF_Label       lab = (*partIt).first;
        TopLoc_Location loc = (*partIt).second.Inverted() * initLoc;
        //
        if ( asiAlgo_Utils::IsIdentity(loc) )
          loc = TopLoc_Location();

        if ( shapeTool->IsSubShape( lab, curShape.Located(loc) ) )
        {
          // Quickly create a subshape's label without spending much time on whatever checks.
          newShapeL = this->__addSubShape( lab, curShape.Located(loc) );
          break;
        }
      }
    }

    if ( newShapeL.IsNull() )
      continue;

    // Copy attributes.
    this->copyAttributes(subIt.Value().first, newShapeL);
  }

  // Remove old subshapes from the expanded part.
  TDF_LabelSequence::const_iterator delIt = childrenToDelete.cbegin();
  for ( ; delIt != childrenToDelete.cend(); delIt++ )
    (*delIt).ForgetAllAttributes();

  // Update assemblies.
  if ( updateAssemblies )
    this->UpdateAssemblies();

  // Pass the colors to the expanded entities as the part becomes an
  // assembly now, and we need to have colors at its ultimate sub-parts.
  Quantity_ColorRGBA surfColor, curvColor, genColor;
  //
  const bool isSurf = colorTool->GetColor(partLabel, XCAFDoc_ColorSurf, surfColor),
             isCurv = colorTool->GetColor(partLabel, XCAFDoc_ColorCurv, curvColor),
             isGen  = colorTool->GetColor(partLabel, XCAFDoc_ColorGen, genColor);
  //
  this->propagateColor(partLabel,
                       isSurf,
                       surfColor,
                       isCurv,
                       curvColor,
                       isGen,
                       genColor);

  return true;
}

//-----------------------------------------------------------------------------

void Doc::SetColor(const TDF_Label&      label,
                   const Quantity_Color& color)
{
  if ( label.IsNull() )
    return;

  TCollection_AsciiString partId;
  this->__entry(label, partId);

  Quantity_ColorRGBA colorRGBA(color), oldColorRGBA;
  //
  if ( this->GetColor( PartId::FromEntry(partId), oldColorRGBA) )
    colorRGBA.SetAlpha( oldColorRGBA.Alpha() );

  this->SetColor(label, colorRGBA, true);
}

//-----------------------------------------------------------------------------

void Doc::SetColor(const TDF_Label&          label,
                   const Quantity_ColorRGBA& color,
                   const bool                changeTransp)
{
  if ( label.IsNull() )
    return;

  /*
   * Colors should be set to parts' labels only. Assigning color to instances or
   * subassemblies is allowed, but we avoid it to simplify the data architecture.
   */

  Handle(XCAFDoc_ShapeTool) ST = this->GetShapeTool();
  //
  if ( ST->IsReference(label) )
  {
    // Set color to the prototype.
    TDF_Label protoLab;
    ST->GetReferredShape(label, protoLab);
    //
    this->SetColor(protoLab, color, changeTransp);
  }
  else if ( ST->IsAssembly(label) )
  {
    // Set color to all assembly components.
    TDF_LabelSequence components;
    ST->GetComponents(label, components, true);
    //
    for ( TDF_LabelSequence::Iterator cit(components); cit.More(); cit.Next() )
    {
      this->SetColor(cit.Value(), color, changeTransp);
    }
  }
  else /* Part */
  {
    TopoDS_Shape shape = ST->GetShape(label);
    //
    if ( shape.IsNull() )
      return;

    Quantity_ColorRGBA colorRGBA(color);
    //
    if ( !changeTransp )
    {
      double alpha = 1.0;
      //
      if ( this->GetColorAlpha(PartId::FromLabel(label), alpha) )
        colorRGBA.SetAlpha( (float) alpha );
    }

    Handle(XCAFDoc_ColorTool) CT = XCAFDoc_DocumentTool::ColorTool(m_doc->Main());

    Handle(TDataStd_TreeNode) colorAttr;
    bool isGenColor = label.FindAttribute(XCAFDoc::ColorRefGUID(XCAFDoc_ColorGen), colorAttr);

    if ( shape.ShapeType() == TopAbs_EDGE )
    {
      CT->SetColor(label, colorRGBA, XCAFDoc_ColorCurv);
    }
    else
    {
      CT->SetColor(label, colorRGBA, XCAFDoc_ColorSurf);
      CT->SetColor(label, colorRGBA, XCAFDoc_ColorCurv);
    }
    if ( isGenColor )
    {
      CT->SetColor(label, colorRGBA, XCAFDoc_ColorGen);
    }

    TDF_LabelSequence subshapes;
    ST->GetSubShapes(label, subshapes);
    //
    for ( TDF_LabelSequence::Iterator iter(subshapes); iter.More(); iter.Next() )
    {
      if ( this->GetShape( iter.Value() ).ShapeType() == TopAbs_EDGE )
      {
        CT->SetColor(iter.Value(), colorRGBA, XCAFDoc_ColorCurv);
      }
      else
      {
        CT->SetColor(iter.Value(), colorRGBA, XCAFDoc_ColorSurf);
        CT->SetColor(iter.Value(), colorRGBA, XCAFDoc_ColorCurv);
      }

      isGenColor = iter.Value().FindAttribute(XCAFDoc::ColorRefGUID(XCAFDoc_ColorGen), colorAttr);
      if ( isGenColor )
      {
        CT->SetColor(iter.Value(), colorRGBA, XCAFDoc_ColorGen);
      }
    }
  }
}

//-----------------------------------------------------------------------------

bool Doc::AutoColorizePart(const PartId& part,
                           const bool    force)
{
  Quantity_Color color;
  bool isOnFaces = false;
  //
  this->getCommonColor(part, color, isOnFaces, force);
  //
  if ( !isOnFaces )
    return false;

  // If we are here, then all faces are colorized identically. Therefore,
  // we can now colorize the part.
  TDF_Label partLab = this->GetLabel(part);

  // Override part's color.
  this->SetColor(partLab, color);
  //
  m_progress.SendLogMessage( LogInfo(Normal) << "Automatically adjusted color for part '%1'."
                                             << this->GetPartName(part) );

  return true;
}

//-----------------------------------------------------------------------------

void Doc::ResetColors()
{
  // Get color tool.
  Handle(XCAFDoc_ColorTool)
    CT = XCAFDoc_DocumentTool::ColorTool( m_doc->Main() );

  TDF_LabelSequence colorLabs;
  CT->GetColors(colorLabs);

  for ( TDF_LabelSequence::Iterator lit(colorLabs); lit.More(); lit.Next() )
  {
    TDF_Label colorLab = lit.Value();
    CT->RemoveColor(colorLab);
  }
}

//-----------------------------------------------------------------------------

void Doc::UpdatePartShape(const TDF_Label&                 partLab,
                          const TopoDS_Shape&              newShape,
                          const Handle(BRepTools_History)& history,
                          const bool                       doUpdateAssemblies)
{
  Handle(XCAFDoc_ShapeTool) ST = this->GetShapeTool();
  Handle(XCAFDoc_ColorTool) CT = this->GetColorTool();

  // If new and old shapes are equal, there's nothing to do.
  TopoDS_Shape oldShape = ST->GetShape(partLab);
  //
  if ( oldShape.IsNull() || oldShape.IsEqual(newShape) )
    return;

  TDF_LabelDataMap  map;
  TDF_LabelSequence labelsToDelete;
  TDF_LabelSequence subshapes;
  //
  ST->GetSubShapes(partLab, subshapes);

  // Set new shape to model.
  TDF_Label newMainLabel = ST->FindShape(newShape);
  if ( newMainLabel.IsNull() )
  {
    ST->SetShape(partLab, newShape);
    newMainLabel = partLab;
  }

  if ( newMainLabel.IsNull() )
  {
    // Delete all necessary labels and return.
    for ( int i = 1; i <= labelsToDelete.Length(); i++ )
    {
      labelsToDelete.Value(i).ForgetAllAttributes();
    }

    if ( doUpdateAssemblies )
      this->UpdateAssemblies();

    return;
  }

  TDF_LabelSequence unsupportedLabels;

  for ( int i = 1; i <= subshapes.Length(); i++ )
  {
    TopoDS_Shape sub  = ST->GetShape(subshapes.Value(i));
    TDF_Label    subL = subshapes.Value(i);

    // Check if history is supported. Otherwise, an assert will be raised
    // by OCCT and we crash.
    const bool isHistorySupported = BRepTools_History::IsSupportedType(sub);
    //
    if ( !isHistorySupported && sub.ShapeType() < TopAbs_FACE )
    {
      unsupportedLabels.Append(subL);
    }

    // Deleted subshapes.
    if ( history.IsNull() || ( isHistorySupported && history->IsRemoved(sub) ) )
    {
      // Do not forget all attributes at this step for PMI update.
      TDF_Label nullLabel;
      labelsToDelete.Append(subL);
      subL.ForgetAttribute(TNaming_NamedShape::GetID());
      subL.ForgetAttribute(XCAFDoc_ShapeMapTool::GetID());
      map.Bind(subL, nullLabel);
    }
    else if ( isHistorySupported )
    {
      // Generated subshapes.
      const TopTools_ListOfShape& genList = history->Generated(sub);
      //
      if ( genList.Size() > 0 )
      {
        TopTools_ListOfShape::Iterator genIt(genList);
        for ( ; genIt.More(); genIt.Next() )
        {
          TDF_Label newSubL = ST->AddSubShape(partLab, genIt.Value());
          //
          if ( !newSubL.IsEqual(subL) )
          {
            Quantity_ColorRGBA color;
            for ( int typeIt = 0; typeIt <= 2; typeIt++ )
            {
              if ( CT->GetColor(subL, XCAFDoc_ColorType(typeIt), color) )
                CT->SetColor(newSubL, color, XCAFDoc_ColorType(typeIt));
            }

            if ( !CT->IsVisible(subL) )
              CT->SetVisibility(newSubL, false);
          }
        }
      }
      // Modified subshapes.
      const TopTools_ListOfShape& modList = history->Modified(sub);
      if ( modList.Size() > 0 )
      {
        TopTools_ListOfShape::Iterator modIt(modList);
        for ( ; modIt.More(); modIt.Next() )
        {
          TDF_Label newSubL = ST->AddSubShape(partLab, modIt.Value());
          if ( !newSubL.IsEqual(subL) )
          {
            labelsToDelete.Append(subL);
            map.Bind(subL, newSubL);
            this->copyAttributes(subL, newSubL);
          }
        }
      }
    }
  }

  // Propagate colors of unsupported shapes.
  // Subshapes compounds and shells are not supported by BRepTools_History, only faces,
  // so iterate faces of unsupported types of subshapes and add new faces-subshapes
  // for correct update.
  if ( history.IsNull() )
  {
    labelsToDelete.Append(unsupportedLabels);
  }
  else
  {
    for ( TDF_LabelSequence::Iterator it(unsupportedLabels); it.More(); it.Next() )
    {
      // Iterate all faces.
      TDF_Label subL = it.Value();
      TDF_Label colorSurfL, colorGenL;
      CT->GetColor(subL, XCAFDoc_ColorSurf, colorSurfL);
      CT->GetColor(subL, XCAFDoc_ColorGen, colorGenL);

      if ( colorSurfL.IsNull() && colorGenL.IsNull() )
      {
        labelsToDelete.Append(subL);
        subL.ForgetAttribute(TNaming_NamedShape::GetID());
        subL.ForgetAttribute(XCAFDoc_ShapeMapTool::GetID());
        continue;
      }

      // Name.
      Handle(TDataStd_Name) nameAttr;
      TCollection_ExtendedString nameStr;
      bool hasName = subL.FindAttribute(TDataStd_Name::GetID(), nameAttr);
      if ( hasName )
        nameStr = nameAttr->Get();

      // Shape.
      TopoDS_Shape sub = ST->GetShape(subL);

      labelsToDelete.Append(subL);
      subL.ForgetAttribute(TNaming_NamedShape::GetID());
      subL.ForgetAttribute(XCAFDoc_ShapeMapTool::GetID());

      TopExp_Explorer faceExp(sub, TopAbs_FACE);
      for ( ; faceExp.More(); faceExp.Next() )
      {
        TopoDS_Shape face = faceExp.Current();
        if ( history->IsRemoved(face) )
          continue;

        // Generated subshapes.
        TopTools_ListOfShape genList = history->Generated(face);
        if ( genList.Size() > 0 )
        {
          TopTools_ListOfShape::Iterator genIt(genList);
          for ( ; genIt.More(); genIt.Next() )
          {
            TDF_Label newSubL;
            newSubL = ST->AddSubShape(partLab, genIt.Value());
            if ( !newSubL.IsNull() )
            {
              if ( !colorSurfL.IsNull() )
                CT->SetColor(newSubL, colorSurfL, XCAFDoc_ColorSurf);
              if ( !colorGenL.IsNull() )
                CT->SetColor(newSubL, colorGenL, XCAFDoc_ColorGen);
            }
          }
        }

        // Modified subshapes.
        TopTools_ListOfShape modList = history->Modified(face);

        // If face was not modified, add it to model as is, it is necessary, because
        // old compounds/shell subshape will be deleted.
        if ( modList.Size() == 0 )
          modList.Append(face);

        TopTools_ListOfShape::Iterator modIt(modList);
        for ( ; modIt.More(); modIt.Next() )
        {
          TDF_Label newSubL;

          // If face is already existed as subshape, do not rewrite its color.
          if ( ST->FindSubShape(partLab, modIt.Value(), newSubL) )
            continue;

          newSubL = ST->AddSubShape(partLab, modIt.Value());
          if ( !newSubL.IsNull() )
          {
            if ( hasName )
              TDataStd_Name::Set(newSubL, nameStr);
            if ( !colorSurfL.IsNull() )
              CT->SetColor(newSubL, colorSurfL, XCAFDoc_ColorSurf);
            if ( !colorGenL.IsNull() )
              CT->SetColor(newSubL, colorGenL, XCAFDoc_ColorGen);
          }
        }
      }
    }
  }

  for ( int i = 1; i <= labelsToDelete.Length(); i++ )
    labelsToDelete.Value(i).ForgetAllAttributes();

  if ( doUpdateAssemblies )
    this->UpdateAssemblies();
}

//-----------------------------------------------------------------------------

void Doc::UpdatePartShape(const PartId&                    partId,
                          const TopoDS_Shape&              newShape,
                          const Handle(BRepTools_History)& history,
                          const bool                       doUpdateAssemblies)
{
  TDF_Label partLab = this->GetLabel(partId);
  //
  if ( partLab.IsNull() )
    return;

  this->UpdatePartShape(partLab, newShape, history, doUpdateAssemblies);
}

//-----------------------------------------------------------------------------

void Doc::UpdateAssemblies(const bool force)
{
  Handle(XCAFDoc_ShapeTool) ST = this->GetShapeTool();

  // We start from the free shapes (roots in the assembly structure).
  TDF_LabelSequence aRootLabels;
  ST->GetFreeShapes(aRootLabels);

  // Iterate over the free shapes
  TDF_LabelMap anUpdated;
  for ( TDF_LabelSequence::Iterator anIt(aRootLabels); anIt.More(); anIt.Next() )
  {
    TDF_Label aRefLabel = anIt.Value();
    if ( ST->IsReference(aRefLabel)) 
    {
      ST->GetReferredShape(aRefLabel, aRefLabel);
    }
    const TDF_Label& aRootLab = aRefLabel;
    TopoDS_Shape anAssemblyShape;
    this->updateComponent(aRootLab, force, anAssemblyShape, anUpdated);
  }
}

//-----------------------------------------------------------------------------

void Doc::ExpandCompounds(const AssemblyItemIds& items)
{
  // Continue recursively.
  TDF_LabelMap processed;
  //
  this->expandCompoundsRecursively(items, processed);

  // Make assemblies consistent in terms of the contained geometries.
  this->UpdateAssemblies();
}

//-----------------------------------------------------------------------------

TDF_Label Doc::CreateEmptyPart()
{
  BRep_Builder builder;
  TopoDS_Compound comp;
  builder.MakeCompound(comp);
  TDF_Label result = GetShapeTool()->AddShape(comp, false);
  return result;
}

//-----------------------------------------------------------------------------

TDF_Label Doc::CreateEmptyAssembly()
{
  BRep_Builder builder;
  TopoDS_Compound comp;
  builder.MakeCompound(comp);
  TDF_Label result = GetShapeTool()->AddShape(comp, true);
  return result;
}

//-----------------------------------------------------------------------------

PartId Doc::AddPart(const TopoDS_Shape& shape,
                    const std::string&  name)
{
  TDF_Label lab = this->__addPart(shape, name);

  TCollection_AsciiString entry;
  this->__entry(lab, entry);

  return PartId(entry);
}

//-----------------------------------------------------------------------------

PartId Doc::AddPart(const std::string& name)
{
  return this->AddPart(TopoDS_Shape(), name);
}

//-----------------------------------------------------------------------------

TDF_Label Doc::AddSubShape(const PartId&       partId,
                           const TopoDS_Shape& subshape)
{
  return this->__addSubShape( this->GetLabel(partId), subshape );
}

//-----------------------------------------------------------------------------

bool Doc::RemoveParts(const PartIds& parts,
                      const bool     doUpdateAssemblies)
{
  Handle(XCAFDoc_ShapeTool) ShapeTool = this->GetShapeTool();

  // Get labels to clean up.
  TDF_LabelSequence list2Delete;
  //
  for ( PartIds::Iterator pit(parts); pit.More(); pit.Next() )
  {
    const PartId& part      = pit.Value();
    TDF_Label     partLabel = this->GetLabel(part);

    if ( partLabel.IsNull() )
    {
      m_progress.SendLogMessage( LogErr(Normal) << "No persistent part can be found for ID %1."
                                                << part.ToString() );
      return false;
    }

    // Instances.
    TDF_LabelSequence curList2Delete;
    this->GetLabelsOfReplicas(partLabel, curList2Delete);
    //
    list2Delete.Append(curList2Delete);
    list2Delete.Append(partLabel);
  }

  // Remove data.
  for ( TDF_LabelSequence::Iterator lit(list2Delete); lit.More(); lit.Next() )
  {
    const TDF_Label& label = lit.Value();

    if ( ShapeTool->IsSimpleShape(label) )
    {
      ShapeTool->RemoveShape(label);
    }
    else
    {
      label.ForgetAllAttributes();
    }
  }

  // Update assemblies.
  if ( doUpdateAssemblies )
    this->UpdateAssemblies();

  return true;
}

//-----------------------------------------------------------------------------

void Doc::RemoveAllEmptyAssemblies()
{
  Handle(XCAFDoc_ShapeTool) ST = this->GetShapeTool();

  // Remove subassemblies for each top-level "shape".
  TDF_LabelSequence topShapes;
  ST->GetShapes(topShapes);
  //
  for ( TDF_LabelSequence::Iterator it(topShapes); it.More(); it.Next() )
  {
    TDF_Label label = it.Value();
    //
    if ( ST->IsAssembly(label) || ST->IsReference(label) )
      this->removeEmptySubAssemblies(label);
  }

  // Update necessary structures.
  this->UpdateAssemblies();
}

//-----------------------------------------------------------------------------

void Doc::TransformItem(const AssemblyItemId& item,
                        const double          tx,
                        const double          ty,
                        const double          tz,
                        const double          rx,
                        const double          ry,
                        const double          rz,
                        const bool            doUpdateAssemblies)
{
  TDF_Label lab = this->GetLabel(item);

  // Prepare transformation.
  gp_Vec Translation(tx, ty, tz);
  gp_Quaternion RX(gp::DX(), rx/180.*M_PI);
  gp_Quaternion RY(gp::DY(), ry/180.*M_PI);
  gp_Quaternion RZ(gp::DZ(), rz/180.*M_PI);
  //
  gp_Trsf T;
  T.SetRotation(RZ*RY*RX);
  T.SetTranslationPart(Translation);

  Handle(XCAFDoc_Location) attr = XCAFDoc_Location::Set(lab, T);

  if ( doUpdateAssemblies )
    this->UpdateAssemblies(true); // Forcibly, as location is the only thing changed.
}

//-----------------------------------------------------------------------------

void Doc::DumpAssemblyItems(Standard_OStream& out) const
{
  for ( DocIterator ait(this); ait.More(); ait.Next() )
  {
    AssemblyItemId item = ait.Current();

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

Handle(TDocStd_Document)& Doc::ChangeDocument()
{
  return m_doc;
}

//-----------------------------------------------------------------------------

const Handle(TDocStd_Document)& Doc::GetDocument() const
{
  return m_doc;
}

//-----------------------------------------------------------------------------

Handle(XCAFDoc_ShapeTool) Doc::GetShapeTool() const
{
  return XCAFDoc_DocumentTool::ShapeTool( m_doc->Main() );
}

//-----------------------------------------------------------------------------

Handle(XCAFDoc_ColorTool) Doc::GetColorTool() const
{
  return XCAFDoc_DocumentTool::ColorTool( m_doc->Main() );
}

//-----------------------------------------------------------------------------

void Doc::init(const Handle(TDocStd_Document)& doc)
{
  // Store the pointer to the passed Document in the member field.
  m_doc = doc;
}

//-----------------------------------------------------------------------------

Handle(TDocStd_Document) Doc::newDocument()
{
  Handle(TDocStd_Document) D;
  Handle(App)              A = this->getApplication();

  // Create XDE Document and return.
  A->NewDocument(BinXCAF, D);
  return D;
}

//-----------------------------------------------------------------------------

Handle(App) Doc::getApplication()
{
  return App::Instance();
}

//-----------------------------------------------------------------------------

void Doc::getLeafItems(AssemblyItemId                     parent,
                       const Handle(HAssemblyItemIdsMap)& itemsMap,
                       AssemblyItemIds&                   items,
                       const Handle(HAssemblyItemIdsMap)& traversed) const
{
  // Check current assembly item id to be traversed.
  if ( traversed->Contains(parent) )
    return;

  // Take out shape tool
  Handle(XCAFDoc_ShapeTool) STool = this->GetShapeTool();

  // Loop over the parts and sub-assemblies in depth-first order.
  // ...

  std::stack<AssemblyItemId> m_fringe;

  TDF_Tool::Label(m_doc->GetData(), parent.GetLastEntry(), parent.m_label);
  //
  if ( parent.m_label.IsNull() )
    return;

  m_fringe.push(parent);

  TCollection_AsciiString entry;

  while ( !m_fringe.empty() )
  {
    AssemblyItemId topId = m_fringe.top();
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
        AssemblyItemId result;
        result.m_label = label;
        result << topId.ToString();
        result << entry;

        m_fringe.push(result);
      }
    }
  }
}

//-----------------------------------------------------------------------------

void Doc::getParts(const TDF_LabelSequence& originals,
                   PartIds&                 parts) const
{
  // Convert labels to part IDs
  for ( TDF_LabelSequence::Iterator it(originals); it.More(); it.Next() )
  {
    // Part ID is nothing but an entry
    TCollection_AsciiString partId;
    TDF_Tool::Entry(it.Value(), partId);
    //
    parts.Append( PartId::FromEntry(partId) );
  }
}

//-----------------------------------------------------------------------------

void Doc::getPartsWithInstances(const LabelsToInstancesMap& origInstances,
                                PartsToInstancesMap&        partsInstances) const
{
  // Convert labels to part IDs
  LabelsToInstancesMap::Iterator it(origInstances);
  for ( ; it.More(); it.Next() )
  {
    const TDF_Label&    original  = it.Key();
    AssemblyItemIdList& instances = it.ChangeValue();

    // Part ID is nothing but an entry
    TCollection_AsciiString partId;
    TDF_Tool::Entry(original, partId);
    //
    const int ind = partsInstances.Add( PartId::FromEntry(partId),
                                        AssemblyItemIdList() );
    partsInstances(ind).Append(instances);
  }
}

//-----------------------------------------------------------------------------

void Doc::getOriginalsWithInstances(const AssemblyItemId& item,
                                    LabelsToInstancesMap& origInstances) const
{
  TDF_Label           original  = this->GetOriginal( item );
  AssemblyItemIdList* instances = origInstances.ChangeSeek(original);
  //
  if ( instances == 0L )
  {
    const int ind = origInstances.Add( original, AssemblyItemIdList() );
    instances     = &origInstances.ChangeFromIndex(ind);
  }
  instances->Append( item );
}

//-----------------------------------------------------------------------------

void Doc::getAssemblyItemsForPart(const TDF_Label&                   original,
                                  const AssemblyItemId&              item,
                                  const Handle(HAssemblyItemIdsMap)& itemsMap,
                                  AssemblyItemIds&                   items) const
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

void Doc::clearSession(const Handle(XSControl_WorkSession)& WS)
{
  if ( WS.IsNull() )
    return;

  // Clear transfer reader.
  Handle(XSControl_TransferReader) transferReader = WS->TransferReader();
  if ( !transferReader.IsNull() )
    transferReader->Clear(-1);
}

//-----------------------------------------------------------------------------

void Doc::expandCompoundsRecursively(const AssemblyItemIds& items,
                                     TDF_LabelMap&                    processed)
{
  for ( AssemblyItemIds::Iterator aiit(items); aiit.More(); aiit.Next() )
  {
    TDF_Label original = this->GetOriginal( aiit.Value() );
    //
    if ( !processed.Add(original) )
      continue; // Skip the already processed prototypes.

    // Check if we're at the compound part.
    const PartId pid = PartId::FromLabel(original);
    //
    if ( this->GetShape(pid).ShapeType() != TopAbs_COMPOUND )
      continue; // Skip anything but TopoDS_Compound geometries.

    // Expand a single part.
    if ( this->ExpandCompound(pid, false) ) // Assemblies are not updated, we'll do this at one shot.
    {
      // Get the generated leaves and continue expansion on them.
      AssemblyItemIds newLeaves;
      AssemblyItemId  parent(pid);
      //
      this->GetLeafAssemblyItems(parent, newLeaves);

      // Proceed recursively.
      this->expandCompoundsRecursively(newLeaves, processed);
    }
  }
}

//-----------------------------------------------------------------------------

void Doc::expand(const TDF_Label&                                    expandedLabel,
                 const TopLoc_Location&                              curLoc,
                 t_expansionMap&                                     subshapeMap,
                 std::vector<std::pair<TDF_Label, TopLoc_Location>>& newParts)
{
  Handle(XCAFDoc_ShapeTool) shapeTool = this->GetShapeTool();
  TopoDS_Shape              mainShape = this->GetShape(expandedLabel);

  // Mark the expanded label as an assembly. This is done by means of a dedicated
  // User Attribute in XDE.
  TDataStd_UAttribute::Set( expandedLabel, XCAFDoc::AssemblyGUID() );
  //
  for ( TopoDS_Iterator compIt(mainShape); compIt.More(); compIt.Next() )
  {
    const TopoDS_Shape& childShape = compIt.Value();

    // Try to find child shape as already existing part.
    TDF_Label partL;
    const bool
      isAlreadyExist = shapeTool->FindShape( childShape.Located( TopLoc_Location() ), partL );
    //
    if ( !isAlreadyExist )
    {
      // Create new part to link child shape.
      partL = this->__addPart( childShape.Located( TopLoc_Location() ) );
    }

    // Add a new component.
    this->AddComponent( expandedLabel, partL, childShape.Location() );

    // Remove new part from subshapes map.
    std::pair<TDF_Label, TopLoc_Location> oldLabel;
    //
    if ( subshapeMap.Find( childShape.Located( TopLoc_Location() ), oldLabel ) )
    {
      this->copyAttributes(oldLabel.first, partL);

      subshapeMap.UnBind( childShape.Located( TopLoc_Location() ) );
    }

    if ( !isAlreadyExist )
    {
      if ( childShape.ShapeType() == TopAbs_COMPOUND )
      {
        this->expand(partL, curLoc * childShape.Location(), subshapeMap, newParts);
      }
      else
      {
        std::pair<TDF_Label, TopLoc_Location> newPart( partL, curLoc*childShape.Location() );
        newParts.push_back(newPart);
      }
    }
  }
}

//-----------------------------------------------------------------------------

void Doc::propagateColor(const TDF_Label&          assemblyLabel,
                         const bool                isSurfColoredAssembly,
                         const Quantity_ColorRGBA& surfColor,
                         const bool                isCurvColoredAssembly,
                         const Quantity_ColorRGBA& curvColor,
                         const bool                isGenColoredAssembly,
                         const Quantity_ColorRGBA& genColor)
{
  // Get tools.
  Handle(XCAFDoc_ShapeTool) shapeTool = this->GetShapeTool();
  Handle(XCAFDoc_ColorTool) colorTool = this->GetColorTool();

  // Get assembly components.
  TDF_LabelSequence components;
  shapeTool->GetComponents(assemblyLabel, components);

  // For each component (which is an instance), find its original and
  // propagate colors to there.
  Quantity_ColorRGBA surfCompColor, curvCompColor, genCompColor;
  //
  for ( TDF_LabelSequence::Iterator it(components); it.More(); it.Next() )
  {
    TDF_Label original;
    shapeTool->GetReferredShape(it.Value(), original);

    bool isSurf = colorTool->GetColor(original, XCAFDoc_ColorSurf, surfCompColor),
         isCurv = colorTool->GetColor(original, XCAFDoc_ColorCurv, curvCompColor),
         isGen  = colorTool->GetColor(original, XCAFDoc_ColorGen, genCompColor);

    if ( shapeTool->IsAssembly(original) )
    {
      /* Surface color */
      if ( !isSurf )
      {
        isSurf = colorTool->GetColor(it.Value(), XCAFDoc_ColorSurf, surfCompColor);
      }
      if ( !isSurf )
      {
        isSurf        = isSurfColoredAssembly;
        surfCompColor = surfColor;
      }

      /* Curve color */
      if ( !isCurv )
      {
        isCurv = colorTool->GetColor(it.Value(), XCAFDoc_ColorCurv, curvCompColor);
      }
      if ( !isCurv )
      {
        isCurv        = isCurvColoredAssembly;
        curvCompColor = curvColor;
      }

      /* Generic color */
      if ( !isGen )
      {
        isGen = colorTool->GetColor(it.Value(), XCAFDoc_ColorGen, genCompColor);
      }
      if ( !isGen )
      {
        isGen        = isGenColoredAssembly;
        genCompColor = genColor;
      }

      // Go down to the components recursively.
      this->propagateColor(original,
                           isSurf, surfCompColor,
                           isCurv, curvCompColor,
                           isGen,  genCompColor);
    }
    else
    {
      if ( isSurfColoredAssembly && !isSurf )
      {
        colorTool->SetColor(original, surfColor, XCAFDoc_ColorSurf);
      }
      if ( isCurvColoredAssembly && !isCurv )
      {
        colorTool->SetColor(original, curvColor, XCAFDoc_ColorCurv);
      }
      if ( isGenColoredAssembly && !isGen )
      {
        colorTool->SetColor(original, genColor, XCAFDoc_ColorGen);
      }
    }
  }
}

//-----------------------------------------------------------------------------

void Doc::getCommonColor(const PartId&   part,
                         Quantity_Color& color,
                         bool&           isOnFaces,
                         const bool      isIgnorePartColor) const
{
  Quantity_ColorRGBA partColor;

  // The Boolean flag is checked first for a slightly better performance.
  if ( !isIgnorePartColor && this->GetColor(part, partColor) )
  {
    isOnFaces = false;
    return;
  }

  Handle(XCAFDoc_ShapeTool) ST = this->GetShapeTool();
  Handle(XCAFDoc_ColorTool) CT = this->GetColorTool();

  // Get part label.
  TDF_Label partLab = this->GetLabel(part);

  // Get part shape.
  TopoDS_Shape partShape = this->GetShape(part);
  //
  if ( partShape.IsNull() || partShape.ShapeType() >= TopAbs_FACE )
  {
    isOnFaces = false;
    return;
  }

  // Get all faces of a part.
  TopTools_IndexedMapOfShape partFaces;
  TopExp::MapShapes(partShape, TopAbs_FACE, partFaces);

  // Loop over the faces to compare their colors and find those
  // faces having no color assigned.
  Quantity_Color refColor;
  bool isFailed = (partFaces.Size() == 0);
  //
  for ( int f = 1; f <= partFaces.Extent(); ++f )
  {
    const TopoDS_Shape& faceShape = partFaces(f);

    // All faces should be colorized.
    TDF_Label faceLab;
    //
    if ( !ST->FindSubShape(partLab, faceShape, faceLab) )
    {
      isFailed = true;
      break;
    }

    // Get color associated with the face.
    TDF_Label faceColorLab;
    if ( !CT->GetColor(faceLab, XCAFDoc_ColorSurf, faceColorLab) &&
         !CT->GetColor(faceLab, XCAFDoc_ColorGen, faceColorLab) )
    {
      m_progress.SendLogMessage( LogWarn(Normal) << "Common color of subshapes of part '%1' cannot "
                                                    "be obtained because of uncolored face(s)."
                                                 << this->GetPartName(part) );
      isOnFaces = false;
      return;
    }

    // Get color.
    Quantity_Color faceColor;
    CT->GetColor(faceColorLab, faceColor);

    if ( f == 1 )
      refColor = faceColor;
    else if ( refColor != faceColor )
    {
      m_progress.SendLogMessage( LogWarn(Normal) << "Common color of subshapes of part '%1' cannot "
                                                    "be obtained because of faces with different colors."
                                                 << this->GetPartName(part) );
      isOnFaces = false;
      return;
    }
  }

  // Another case is part-compound, all components of which is colored in one color
  if ( isFailed && partShape.ShapeType() != TopAbs_COMPOUND )
  {
    m_progress.SendLogMessage( LogWarn(Normal) << "Common color of subshapes of part '%1' cannot "
                                                  "be obtained because of uncolored face(s)."
                                               << this->GetPartName(part) );
    isOnFaces = false;
    return;
  }

  if ( isFailed && partShape.ShapeType() == TopAbs_COMPOUND )
  {
    bool isFirst = true;
    for ( TopoDS_Iterator it(partShape); it.More(); it.Next() )
    {
      const TopoDS_Shape& compShape = it.Value();
      if ( compShape.ShapeType() > TopAbs_FACE )
        continue;

      // All components should be colorized.
      TDF_Label subShapeL;
      if ( !ST->FindSubShape(partLab, compShape, subShapeL) )
      {
        m_progress.SendLogMessage( LogWarn(Normal) << "Common color of subshapes of part '%1' cannot "
                                                      "be obtained because not all subshapes are colorized; "
                                                      "try expanding compounds first."
                                                   << this->GetPartName(part) );
        isOnFaces = false;
        return;
      }

      // Get color associated with the component.
      TDF_Label colorLab;
      if ( !CT->GetColor(subShapeL, XCAFDoc_ColorSurf, colorLab) &&
           !CT->GetColor(subShapeL, XCAFDoc_ColorGen, colorLab) )
      {
        m_progress.SendLogMessage( LogWarn(Normal) << "Common color of subshapes of part '%1' cannot "
                                                      "be obtained because not all subshapes are colorized; "
                                                      "try expanding compounds first."
                                                   << this->GetPartName(part) );
        isOnFaces = false;
        return;
      }

      // Get color.
      Quantity_Color subColor;
      CT->GetColor(colorLab, subColor);

      if ( isFirst )
      {
        refColor = subColor;
        isFirst = false;
      }
      else if ( refColor != subColor )
      {
        m_progress.SendLogMessage( LogWarn(Normal) << "Common color of subshapes of part '%1' cannot "
                                                      "be obtained because of subshapes with different colors; "
                                                      "try expanding compounds first."
                                                   << this->GetPartName(part) );
        isOnFaces = false;
        return;
      }
      isFailed = false;
    }
  }

  if ( isFailed )
  {
    // If we are here, then part is empty auxiliary compound for quality mesh, just return.
    isOnFaces = false;
    return;
  }

  color     = refColor;
  isOnFaces = true;
}

//-----------------------------------------------------------------------------

void Doc::copyAttributes(const TDF_Label from,
                         TDF_Label&      to)
{
  // Contract check.
  if ( from.IsNull() || to.IsNull() || from.IsEqual(to) )
    return;

  // Copy color.
  Handle(XCAFDoc_ColorTool) newColorTool = XCAFDoc_DocumentTool::ColorTool(to);
  Handle(XCAFDoc_ColorTool) colorTool    = XCAFDoc_DocumentTool::ColorTool(from);
  //
  Quantity_ColorRGBA color;
  if ( colorTool->GetColor(from, XCAFDoc_ColorSurf, color) )
  {
    newColorTool->SetColor(to, color, XCAFDoc_ColorSurf);
  }
  if ( colorTool->GetColor(from, XCAFDoc_ColorCurv, color) )
  {
    newColorTool->SetColor(to, color, XCAFDoc_ColorCurv);
  }
  if ( colorTool->GetColor(from, XCAFDoc_ColorGen, color) )
  {
    newColorTool->SetColor(to, color, XCAFDoc_ColorGen);
  }
  if ( !colorTool->IsVisible(from) )
  {
    newColorTool->SetVisibility(to, false);
  }

  // Copy layers.
  Handle(XCAFDoc_LayerTool)                 newLayerTool = XCAFDoc_DocumentTool::LayerTool(to);
  Handle(XCAFDoc_LayerTool)                 layerTool    = XCAFDoc_DocumentTool::LayerTool(from);
  Handle(TColStd_HSequenceOfExtendedString) layers;
  //
  layerTool->GetLayers(from, layers);
  //
  for ( int j = 1; j <= layers->Length(); ++j )
  {
    newLayerTool->SetLayer(to, layers->Value(j));
  }

  // Copy materials.
  Handle(XCAFDoc_MaterialTool) newMatTool = XCAFDoc_DocumentTool::MaterialTool(to);
  Handle(XCAFDoc_MaterialTool) matTool    = XCAFDoc_DocumentTool::MaterialTool(from);
  Handle(TDataStd_TreeNode)    matNode;
  //
  if ( from.FindAttribute(XCAFDoc::MaterialRefGUID(), matNode) && matNode->HasFather() )
  {
    TDF_Label matL = matNode->Father()->Label();
    Handle(TCollection_HAsciiString) name;
    Handle(TCollection_HAsciiString) description;

    double density;
    Handle(TCollection_HAsciiString) densName;
    Handle(TCollection_HAsciiString) densValType;

    if ( matTool->GetMaterial(matL, name, description, density, densName, densValType) )
    {
      if ( name->Length() != 0 )
        newMatTool->SetMaterial(to, name, description, density, densName, densValType);
    }
  }

  // All attributes.
  Handle(TDF_Attribute) tAtt;
  //
  for ( TDF_AttributeIterator attItr(from); attItr.More(); attItr.Next() )
  {
    const Handle(TDF_Attribute) sAtt = attItr.Value();

    // Protect against color and layer coping without link to colors and layers.
    if ( sAtt->IsKind( STANDARD_TYPE(TDataStd_TreeNode) ) || sAtt->IsKind(STANDARD_TYPE(XCAFDoc_GraphNode) ) )
      continue;

    // Do not copy shape, it is already copied.
    if ( sAtt->IsKind( STANDARD_TYPE(TNaming_NamedShape) ) || sAtt->IsKind(STANDARD_TYPE(XCAFDoc_ShapeMapTool) ) )
      continue;

    // Do not copy location, it should be copied during shape creation.
    if ( sAtt->IsKind( STANDARD_TYPE(XCAFDoc_Location) ) )
      continue;

    const Standard_GUID& id = sAtt->ID();
    //
    if ( !to.FindAttribute(id, tAtt) )
    {
      tAtt = sAtt->NewEmpty();
      to.AddAttribute(tAtt);
    }
    Handle(TDF_RelocationTable) rt = new TDF_RelocationTable();
    sAtt->Paste(tAtt, rt);
  }
}

//-----------------------------------------------------------------------------

TDF_Label Doc::AddComponent(const TDF_Label&       assemblyLabel,
                            const TDF_Label&       compLabel,
                            const TopLoc_Location& location)
{
  TDF_Label result;

  // Use the XDE's shape tool to add a new component.
  if ( assemblyLabel.IsNull() )
  {
    if ( !location.IsIdentity() )
    {
      TopoDS_Shape locatedShape = this->GetShape(compLabel).Located(location);
      result = this->GetShapeTool()->AddShape(locatedShape);
    }
    else
    {
      result = compLabel;
    }
  }
  else
  {
    result = this->GetShapeTool()->AddComponent(assemblyLabel, compLabel, location);
  }

  return result;
}

//-----------------------------------------------------------------------------

void Doc::SetFacets(const TDF_Label&                  partLabel,
                    const Handle(Poly_Triangulation)& mesh)
{
  if (partLabel.IsNull())
    return;

  TopoDS_Face fictiveFace;
  BRep_Builder().MakeFace(fictiveFace);
  BRep_Builder().UpdateFace(fictiveFace, mesh);

  UpdatePartShape(partLabel, fictiveFace, NULL, false);
}

//-----------------------------------------------------------------------------

void Doc::SetMesh(const TDF_Label&                  partLabel,
                  const Handle(Poly_Triangulation)& mesh)
{
  if (partLabel.IsNull())
    return;

  Handle(TDataXtd_Triangulation) attr = TDataXtd_Triangulation::Set(partLabel);
  attr->Set(mesh);
}

//-----------------------------------------------------------------------------

void Doc::findItemsRecursively(const Handle(Graph)&         asmGraph,
                               const int                    parentId,
                               const std::string&           name,
                               std::vector<int>&            path,
                               Handle(HAssemblyItemIdsMap)& items) const
{
  if ( asmGraph->HasChildren(parentId) )
  {
    // Visit children.
    const TColStd_PackedMapOfInteger& children = asmGraph->GetChildren(parentId);
    //
    for ( TColStd_MapIteratorOfPackedMapOfInteger cit(children); cit.More(); cit.Next() )
    {
      const int           childId = cit.Key();
      const PersistentId& pid     = asmGraph->GetPersistentId(childId);

      path.push_back(childId);

      // Get name of the currently iterated item.
      TCollection_ExtendedString currName;
      if ( !this->GetObjectName(pid, currName) )
        continue;

      std::string currNameStr = TCollection_AsciiString(currName).ToCString();
      //
      if ( currNameStr == name )
      {
        // Loop over the parents to gather all persistent IDs.
        AssemblyItemId item;

        for ( std::vector<int>::reverse_iterator pit = path.rbegin(); pit != path.rend(); ++pit )
        {
          // Get node's type in the assembly graph.
          const int       nid  = *pit;
          Graph::NodeType type = asmGraph->GetNodeType(nid);

          // The assembly item ID does not contain prototypes' IDs except
          // the root one by convention.
          if ( ( (type != Graph::NodeType_Part) &&
                 (type != Graph::NodeType_Subassembly) ) || (pit == path.rend() - 1) )
          {
            item.Prepend( asmGraph->GetPersistentId(nid) );
          }
        }

        // Add to the result.
        items->Add(item);
      }

      // Continue recursively.
      this->findItemsRecursively(asmGraph, childId, name, path, items);

      path.pop_back();
    }
  }
}

//-----------------------------------------------------------------------------

bool Doc::removeEmptyAssembly(const TDF_Label& assembly,
                              const bool       isUpdate)
{
  Handle(XCAFDoc_ShapeTool) ST = this->GetShapeTool();
  //
  if ( !ST->IsAssembly(assembly) )
    return false;
  //
  if ( ST->NbComponents(assembly) > 0 )
    return false;

  TDF_LabelSequence users;
  const int nbUsers = ST->GetUsers(assembly, users);
  //
  for ( int i = 1; i <= nbUsers; ++i )
  {
    users.Value(i).ForgetAllAttributes();
  }

  const bool isOk = ST->RemoveShape(assembly);

  if ( isUpdate )
    this->UpdateAssemblies();

  return isOk;
}

//-----------------------------------------------------------------------------

void Doc::removeEmptySubAssemblies(const TDF_Label& assembly)
{
  Handle(XCAFDoc_ShapeTool) ST = this->GetShapeTool();

  TDF_Label asmLab = assembly;
  //
  if ( ST->IsReference(assembly) )
    ST->GetReferredShape(assembly, asmLab);

  TDF_LabelSequence components;
  ST->GetComponents(asmLab, components);
  //
  for ( int i = 1; i <= components.Length(); ++i )
  {
    TDF_Label compLab = components.Value(i);

    if ( ST->IsReference(compLab) )
      ST->GetReferredShape(compLab, compLab);

    if ( ST->IsAssembly(compLab) )
      this->removeEmptySubAssemblies(compLab);
  }

  if ( ST->NbComponents(asmLab) == 0 )
    this->removeEmptyAssembly(asmLab, false);
}

//-----------------------------------------------------------------------------

bool Doc::updateComponent(const TDF_Label& theItemLabel,
                          const bool       force,
                          TopoDS_Shape&    theUpdatedShape,
                          TDF_LabelMap&    theUpdated) const
{
  if ( !IsAssembly(theItemLabel) )
    return false; // Do nothing for non-assemblies.

  Handle(XCAFDoc_ShapeTool) ST = this->GetShapeTool();

  // Get the currently stored compound for the assembly
  TopoDS_Shape aCurrentRootShape;
  ST->GetShape(theItemLabel, aCurrentRootShape);

  // Check if the given assembly is already updated
  if (theUpdated.Contains(theItemLabel)) {
    theUpdatedShape = aCurrentRootShape;
    return Standard_True;
  }

  TopTools_MapOfOrientedShape aCurrentRootShapeMap (aCurrentRootShape.NbChildren());

  // Get components of the assembly
  TDF_LabelSequence aComponentLabs;
  ST->GetComponents(theItemLabel, aComponentLabs);

  // This flag indicates whether to update the compound of the assembly
  Standard_Boolean isModified = force ? true : false;

  // Compare the number of components in XDE structure with the number of
  // components in topological structure. A component may happen to be removed,
  // so we have to update the assembly compound.
  if ( !force )
  {
    const Standard_Integer aNumTopoComponents = aCurrentRootShape.NbChildren();
    //
    if ( aNumTopoComponents != aComponentLabs.Length() )
      isModified = Standard_True;
  }

  // Iterate over the assembly components. If at least one component is
  // modified (this is the recursive check), then the actually stored
  // compound has to be updated
  TopTools_ListOfShape aComponentShapes;
  //
  for ( TDF_LabelSequence::Iterator aCompIt(aComponentLabs); aCompIt.More(); aCompIt.Next() )
  {
    const TDF_Label& aComponentLab = aCompIt.Value();

    // Take the referred assembly item (ultimately, a part for an instance)
    TDF_Label aComponentRefLab;
    ST->GetReferredShape(aComponentLab, aComponentRefLab);

    // Shape comes with some placement transformation here
    TopoDS_Shape aComponentShape;
    ST->GetShape(aComponentLab, aComponentShape);
    TopLoc_Location aComponentLoc = aComponentShape.Location();

    // If the component is a sub-assembly, then its associated compound
    // has to be processed in the same manner
    if ( IsAssembly(aComponentRefLab) )
    {
      // Recursive call
      if ( updateComponent(aComponentRefLab, force, aComponentShape, theUpdated) )
      {
        isModified = Standard_True;
        aComponentShape.Location(aComponentLoc); // Apply placement.
      }
    }
    else
    {
      // Search for a part in the actual compound of the ultimate assembly.
      // If the part is there, then the compound is up-to-date, so it does not require rebuilding
      if (!isModified)
      {
        if (aCurrentRootShapeMap.IsEmpty())
        {
          // optimize search for next labels in aComponentLabs
          for (TopoDS_Iterator aTopoIt(aCurrentRootShape); aTopoIt.More(); aTopoIt.Next())
          {
            aCurrentRootShapeMap.Add (aTopoIt.Value());
          }
        }
        if ( !aCurrentRootShapeMap.Contains(aComponentShape) )
        {
          // Part has been modified somewhere, so the compound has to be rebuilt
          isModified = Standard_True;
        }
      }
    }

    // Fill the list of shapes composing a new compound for the assembly
    aComponentShapes.Append(aComponentShape);
  }

  // If any component is modified, we update the currently stored shape
  if ( isModified )
  {
    TopoDS_Compound anUpdatedCompound;
    BRep_Builder aBB;
    aBB.MakeCompound(anUpdatedCompound);

    // Compose new compound
    for ( TopTools_ListIteratorOfListOfShape aShapeIt(aComponentShapes); aShapeIt.More(); aShapeIt.Next() )
    {
      aBB.Add( anUpdatedCompound, aShapeIt.Value() );
    }

    // Store the updated shape as an output
    theUpdatedShape = anUpdatedCompound;

    // Use topological naming services to store the updated shape in XDE
    TNaming_Builder NB(theItemLabel);
    NB.Generated(theUpdatedShape);
  }

  if (isModified)
    theUpdated.Add(theItemLabel);

  return isModified;
}

//-----------------------------------------------------------------------------
// Methods with improved efficiency
//-----------------------------------------------------------------------------

void Doc::__getComponents(const TDF_Label&        l,
                          std::vector<TDF_Label>& labels) const
{
  for ( TDF_ChildIterator lit(l); lit.More(); lit.Next() )
  {
    TDF_Label compLab = lit.Value();
    //
    if ( compLab.HasAttribute() )
      labels.push_back(compLab);
  }
}

//-----------------------------------------------------------------------------

void Doc::__entry(const TDF_Label&         label,
                  TCollection_AsciiString& entry) const
{
  if ( label.IsNull() )
    return;

  const TCollection_AsciiString* pEntry = m_LECache.Seek(label);
  //
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

bool Doc::__isInstance(const Handle(XCAFDoc_ShapeTool)& ST,
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

//-----------------------------------------------------------------------------

TDF_Label Doc::__addPart(const TopoDS_Shape& shape,
                         const std::string&  name)
{
  // Add new part.
  TDF_Label resultL;
  TDF_TagSource tag;
  resultL = tag.NewChild( this->GetShapeTool()->Label() );

  // Set TNaming_NamedShape attribute.
  TNaming_Builder tnBuild(resultL);
  tnBuild.Generated(shape);

  TCollection_AsciiString nameStr;
  //
  if ( name == "" )
  {
    // Set name by the shape type.
    if ( shape.IsNull() )
    {
      nameStr = "Unnamed";
    }
    else
    {
      std::stringstream stream;
      TopAbs::Print(shape.ShapeType(), stream);
      nameStr = ( stream.str().c_str() );
    }
  }
  else
  {
    nameStr = name.c_str();
  }
  //
  TDataStd_Name::Set( resultL, TCollection_ExtendedString(nameStr) );

  return resultL;
}

//-----------------------------------------------------------------------------

TDF_Label Doc::__addSubShape(const TDF_Label&    partLabel,
                             const TopoDS_Shape& subshape)
{
  TDF_Label resultL;
  TDF_TagSource tag;
  resultL = tag.NewChild(partLabel);

  // Set TNaming_NamedShape attribute.
  TNaming_Builder tnBuild(resultL);
  tnBuild.Generated(subshape);

  return resultL;
}
