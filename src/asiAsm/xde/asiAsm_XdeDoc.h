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

#ifndef asiAsm_XdeDoc_h
#define asiAsm_XdeDoc_h

// asiAsm includes
#include <asiAsm_XdePersistentIds.h>

// Active Data includes
#include <ActAPI_IPlotter.h>
#include <ActAPI_IProgressNotifier.h>

// OpenCascade includes
#include <TDF_LabelDataMap.hxx>
#include <TDF_LabelSequence.hxx>
#include <TDocStd_Document.hxx>

// Forward declarations;
class XCAFDoc_ShapeTool;
class XCAFDoc_ColorTool;
class XSControl_WorkSession;
class Quantity_ColorRGBA;
class asiAsm_XdeApp;
class asiAsm_XdeGraph;
class asiAsm_XdePartRepr;

//-----------------------------------------------------------------------------

//! \ingroup ASIASM
//!
//! Map of parts to instances.
typedef NCollection_IndexedDataMap<asiAsm_XdePartId,
                                   asiAsm_XdeAssemblyItemIdList,
                                   asiAsm_XdePartId::Hasher> asiAsm_XdePartsToInstancesMap;

//! \ingroup ASIASM
//!
//! Map of labels to instances.
typedef NCollection_IndexedDataMap<TDF_Label,
                                   asiAsm_XdeAssemblyItemIdList,
                                   TDF_LabelMapHasher> asiAsm_XdeLabelsToInstancesMap;

//-----------------------------------------------------------------------------

//! \ingroup ASIASM
//!
//! Facade class exposing all services for working with an XDE document as
//! an assembly. There should be only one instance of this class for each STEP
//! file under inspection.
//!
//! This class gives read-only access to XDE Document. It is assumed here that
//! XDE Document serves as a data transfer object for subsequent processing
//! from within application.
class asiAsm_XdeDoc : public Standard_Transient
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAsm_XdeDoc, Standard_Transient)

public:

  //! Auxiliary data map to support compound-parts expansion logic.
  typedef NCollection_DataMap< TopoDS_Shape,
                               std::pair<TDF_Label, TopLoc_Location>,
                               TopTools_ShapeMapHasher> t_expansionMap;

public:

  //! Ctor. Created DETACHED Model instance, i.e. the facade is not attached
  //! to any OCAF Document, so you cannot read data unless you load it.
  //!
  //! \param[in] progress progress notifier.
  //! \param[in] plotter  imperative plotter.
  asiAsm_EXPORT
    asiAsm_XdeDoc(ActAPI_ProgressEntry progress = nullptr,
                  ActAPI_PlotterEntry  plotter  = nullptr);

/* Construction and initialization */
public:

  //! Creates new empty XDE Document under this Assembly Document facade.
  asiAsm_EXPORT void
    NewDocument();

  //! Loads file of any supported format.
  //! \param[in] filename name of the file to load.
  //! \return true in case of success, false -- otherwise.
  asiAsm_EXPORT bool
    Load(const TCollection_AsciiString& filename);

  //! Loads the native XDE Document.
  //! \param[in] filename name of the file containing CAF Document to open.
  //! \return true in case of success, false -- otherwise.
  asiAsm_EXPORT bool
    LoadNative(const TCollection_AsciiString& filename);

  //! Loads STEP file to populate the internal XDE Document.
  //! \param[in] filename name of the STEP file to load.
  //! \return true in case of success, false -- otherwise.
  asiAsm_EXPORT bool
    LoadSTEP(const TCollection_AsciiString& filename);

  //! Saves the document to file.
  //! \param[in] filename target filename.
  //! \return true in case of success, false -- otherwise.
  asiAsm_EXPORT bool
    SaveAs(const TCollection_AsciiString& filename);

  //! \return true if the Assembly Document is empty, i.e. the XDE Document
  //!         is either null or contains no roots.
  asiAsm_EXPORT bool
    IsEmpty() const;

  //! Releases all resources occupied by this Assembly Document instance.
  asiAsm_EXPORT void
    Release();

/* API */
public:

  //! Finds assembly items having the passed object name.
  //! \param[in]  name  the name in question.
  //! \param[out] items the unordered collection of unique items found.
  //! \return true if anything was found, false -- otherwise.
  asiAsm_EXPORT bool
    FindItems(const std::string&                     name,
              Handle(asiAsm_XdeHAssemblyItemIdsMap)& items) const;

  //! Sets name for the label.
  //! \param[in] label label.
  //! \param[in] name  name to set.
  asiAsm_EXPORT void
    SetObjectName(const TDF_Label&                  label,
                  const TCollection_ExtendedString& name);

  //! Returns the name which is associated directly with the given object ID.
  //! \param[in]  id   object ID.
  //! \param[out] name object name (empty string if no name is available).
  //! \return true if name is available, false -- otherwise.
  asiAsm_EXPORT bool
    GetObjectName(const asiAsm_XdePersistentId& id,
                  TCollection_ExtendedString&   name) const;

  //! Returns the name which is associated directly with the given part ID.
  //! \param[in]  id   part ID.
  //! \param[out] name object name (empty string if no name is available).
  //! \return true if name is available, false -- otherwise.
  asiAsm_EXPORT bool
    GetObjectName(const asiAsm_XdePartId&     id,
                  TCollection_ExtendedString& name) const;

  //! Returns the name which is associated with the given assembly item.
  //! \param[in]  id   assembly item ID.
  //! \param[out] name extracted object name.
  asiAsm_EXPORT bool
    GetObjectName(const asiAsm_XdeAssemblyItemId& id,
                  TCollection_ExtendedString&     name) const;

  //! Returns objects's name.
  //! \param label [in] label.
  //! \return name of the object.
  asiAsm_EXPORT TCollection_ExtendedString
    GetObjectName(const TDF_Label& label) const;

  //! Returns part's name.
  //! \param part [in] part ID.
  //! \return name of the part.
  asiAsm_EXPORT TCollection_ExtendedString
    GetPartName(const asiAsm_XdePartId& part) const;

  //! Returns all stored part's representations.
  //! \param[in]  partId part of interest.
  //! \param[out] reps   available part representations.
  asiAsm_EXPORT void
    GetPartRepresentations(const asiAsm_XdePartId&                  partId,
                           std::vector<Handle(asiAsm_XdePartRepr)>& reps) const;

  //! Returns all stored part's representations.
  //! \param[in]  partId part of interest.
  //! \param[out] reps   available part representations.
  asiAsm_EXPORT void
    GetPartRepresentations(const TDF_Label&                         label,
                           std::vector<Handle(asiAsm_XdePartRepr)>& reps) const;

  //! Returns the part's representation for the given ID.
  //! \param[in]  partId part of interest.
  //! \param[in]  guid   GUID of the representation of interest.
  //! \param[out] reps   part representation or null if such a representation
  //!                    does not exist.
  //! \return true if the representation exists, false -- otherwise.
  asiAsm_EXPORT bool
    GetPartRepresentation(const asiAsm_XdePartId&     partId,
                          const Standard_GUID&        guid,
                          Handle(asiAsm_XdePartRepr)& rep) const;

  //! Checks whether the label is assembly.
  //! \return true/false.
  asiAsm_EXPORT bool
    IsAssembly(const TDF_Label& itemLabel) const;

  //! Checks whether the passed item represents assembly.
  //! \param[in] item assembly item in question.
  //! \return true/false.
  asiAsm_EXPORT bool
    IsAssembly(const asiAsm_XdeAssemblyItemId& item) const;

  //! Checks whether the passed label represents an instance or not.
  //! \param[in] itemLab label to check.
  //! \param[in] origin  label of simple (origin) shape.
  //! \return true/false.
  asiAsm_EXPORT bool
    IsInstance(const TDF_Label& itemLab,
               TDF_Label&       origin) const;

  //! Checks whether the passed assembly item is instance or not.
  //! \param[in] item   assembly item to check.
  //! \param[in] origin label of simple (origin) shape.
  //! \return true/false.
  asiAsm_EXPORT bool
    IsInstance(const asiAsm_XdeAssemblyItemId& item,
               TDF_Label&                      origin) const;

  //! Checks whether the given label is a label of a part.
  //! \param[in] label label to check.
  //! \return true/false.
  asiAsm_EXPORT bool
    IsPart(const TDF_Label& label) const;

  //! Checks whether the given assembly item is a part or not.
  //! \param[in] item assembly item to check.
  //! \return true/false.
  asiAsm_EXPORT bool
    IsPart(const asiAsm_XdeAssemblyItemId& item) const;

  //! Checks whether the passed label represents an original or not.
  //! \param[in] itemLab label to check.
  //! \return true/false.
  asiAsm_EXPORT bool
    IsOriginal(const TDF_Label& label) const;

  //! Accessor for the original label.
  //! \param[in] itemLabel label of interest.
  //! \return original label.
  asiAsm_EXPORT TDF_Label
    GetOriginal(const TDF_Label& itemLabel) const;

  //! Accessor for the original label.
  //! \param[in] item assembly item of interest.
  //! \return original label.
  asiAsm_EXPORT TDF_Label
    GetOriginal(const asiAsm_XdeAssemblyItemId& item) const;

  //! Returns part ID for the given assembly item.
  //! \param[in] item the assembly item of interest.
  //! \return original part.
  asiAsm_EXPORT asiAsm_XdePartId
    GetPart(const asiAsm_XdeAssemblyItemId& item) const;

  //! For the given list of assembly items, this method extracts originals
  //! without duplications. Use this method if you wish to work on originals,
  //! but be aware that any modification there will affect all the instances.
  //!
  //! \param[in]  anyItems       your items of interest.
  //! \param[out] originalLabels extracted originals.
  asiAsm_EXPORT void
    GetOriginals(const Handle(asiAsm_XdeHAssemblyItemIdsMap)& anyItems,
                 TDF_LabelSequence&                           originalLabels) const;

  //! For the given list of assembly items, this method extracts originals
  //! without duplications. Use this method if you wish to work on originals,
  //! but be aware that any modification there will affect all the instances.
  //!
  //! \param[in]  anyItems       your items of interest.
  //! \param[out] originalLabels extracted originals.
  asiAsm_EXPORT void
    GetOriginals(const asiAsm_XdeAssemblyItemIds& anyItems,
                 TDF_LabelSequence&               originalLabels) const;

  //! For the given list of assembly items, this method extracts originals
  //! without duplications, and fills in the map of originals to their instances.
  //! Use this method if you wish to work on originals,
  //! but be aware that any modification there will affect all the instances.
  //!
  //! \param[in]  anyItems      your items of interest.
  //! \param[out] origInstances map original -> instances.
  asiAsm_EXPORT void
    GetOriginalsWithInstances(const asiAsm_XdeAssemblyItemIds& anyItems,
                              asiAsm_XdeLabelsToInstancesMap&  origInstances) const;

  //! For the given list of assembly items, this method extracts originals
  //! without duplications, and fills in the map of originals to their instances.
  //! Use this method if you wish to work on originals,
  //! but be aware that any modification there will affect all the instances.
  //!
  //! \param[in]  anyItems      your items of interest.
  //! \param[out] origInstances map original -> instances.
  asiAsm_EXPORT void
    GetOriginalsWithInstances(const Handle(asiAsm_XdeHAssemblyItemIdsMap)& anyItems,
                              asiAsm_XdeLabelsToInstancesMap&              origInstances) const;

  //! This method extracts all parts without duplications.
  //! Basically, this method first takes all leaf assembly
  //! items and then takes their originals.
  //!
  //! \param[out] parts extracted parts.
  asiAsm_EXPORT void
    GetParts(asiAsm_XdePartIds& parts) const;

  //! For the given list of any assembly items, this method extracts parts
  //! without duplications. Basically, this method first takes all leaf assembly
  //! items and then takes their originals.
  //!
  //! \param[in]  anyItems       any assembly items.
  //! \param[out] parts          extracted parts.
  //! \param[in]  isAlreadyLeafs disable leafs searching for passed items
  //!                            in case of true value
  asiAsm_EXPORT void
    GetParts(const Handle(asiAsm_XdeHAssemblyItemIdsMap)& anyItems,
             asiAsm_XdePartIds&                           parts,
             const bool                                   isAlreadyLeafs = false) const;

  //! For the given list of any assembly items, this method extracts parts
  //! without duplications. Basically, this method first takes all leaf assembly
  //! items and then takes their originals.
  //!
  //! \param[in]  anyItems       any assembly items.
  //! \param[out] parts          extracted parts.
  //! \param[in]  isAlreadyLeafs disable leafs searching for passed items
  //!                            in case of true value
  asiAsm_EXPORT void
    GetParts(const asiAsm_XdeAssemblyItemIds& anyItems,
             asiAsm_XdePartIds&               parts,
             const bool                       isAlreadyLeafs = false) const;

  //! For the given list of any assembly items, this method extracts parts
  //! without duplications. Basically, this method first takes all leaf assembly
  //! items and then takes their originals.
  //!
  //! \param[in]  anyItems  any assembly items.
  //! \param[out] leafItems leaf assembly items extracted down the road.
  //! \param[out] parts     extracted parts.
  asiAsm_EXPORT void
    GetParts(const asiAsm_XdeAssemblyItemIds&       anyItems,
             Handle(asiAsm_XdeHAssemblyItemIdsMap)& leafItems,
             asiAsm_XdePartIds&                     parts) const;

  //! For the given list of any assembly items, this method extracts parts
  //! without duplications. Basically, this method first takes all leaf assembly
  //! items and then takes their originals.
  //!
  //! \param[in]  anyItems  any assembly items.
  //! \param[out] leafItems leaf assembly items extracted down the road.
  //! \param[out] parts     extracted parts.
  asiAsm_EXPORT void
    GetParts(const asiAsm_XdeAssemblyItemIds& anyItems,
             asiAsm_XdeAssemblyItemIds&       leafItems,
             asiAsm_XdePartIds&               parts) const;

  //! For the given list of any assembly items, this method extracts parts
  //! without duplications. Basically, this method first takes all leaf assembly
  //! items and then takes their originals.
  //!
  //! \param[in]  anyItems  any assembly items.
  //! \param[out] leafItems leaf assembly items extracted down the road.
  //! \param[out] parts     extracted parts.
  asiAsm_EXPORT void
    GetParts(const Handle(asiAsm_XdeHAssemblyItemIdsMap)& anyItems,
             Handle(asiAsm_XdeHAssemblyItemIdsMap)&       leafItems,
             asiAsm_XdePartIds&                           parts) const;

  //! Counts the number of occurrences for each part by visiting all the
  //! arcs in the corresponding HAG.
  //! \param[out] quantities id-to-quantity map.
  asiAsm_EXPORT void
    CountParts(NCollection_DataMap<asiAsm_XdePartId, int, asiAsm_XdePartId::Hasher>& quantities) const;

  //! For the given list of any assembly items, this method extracts parts
  //! without duplications, and fills in the map of parts to their instances.
  //! Basically, this method first takes all leaf assembly items and then
  //! takes their originals.
  //!
  //! \param[in]  anyItems       any assembly items.
  //! \param[out] partsInstances part-to-instances map.
  //! \param[in]  isAlreadyLeafs disable leafs searching for passed items
  //!                            in case of true value
  asiAsm_EXPORT void
    GetPartsWithInstances(const asiAsm_XdeAssemblyItemIds& anyItems,
                          asiAsm_XdePartsToInstancesMap&   partsToInstances,
                          const bool                       isAlreadyLeafs = false) const;

  //! For the given list of any assembly items, this method extracts parts
  //! without duplications, and fills in the map of parts to their instances.
  //! Basically, this method first takes all leaf assembly items and then
  //! takes their originals.
  //!
  //! \param[in]  anyItems       any assembly items.
  //! \param[out] partsInstances part-to-instances map.
  //! \param[in]  isAlreadyLeafs disable leafs searching for passed items
  //!                            in case of true value
  asiAsm_EXPORT void
    GetPartsWithInstances(const Handle(asiAsm_XdeHAssemblyItemIdsMap)& anyItems,
                          asiAsm_XdePartsToInstancesMap&               partsToInstances,
                          const bool                                   isAlreadyLeafs = false) const;

  //! For the given entry ID of the assembly item, this method extracts
  //! entry ID of the corresponding original.
  //!
  //! \attention This method does not extract leaf items for the passed item ID,
  //!            so calling it you have to be sure that the passed item ID
  //!            corresponds to a part.
  //!
  //! \param[in]  assemblyEntryId item ID of interest.
  //! \param[out] partId          extracted part ID.
  asiAsm_EXPORT void
    GetAsPartId(const asiAsm_XdeAssemblyItemId& assemblyEntryId,
                asiAsm_XdePartId&               partId);

  //! Accessor for the underlying CAF TDF Label.
  //! \return CAF Label.
  asiAsm_EXPORT TDF_Label
    GetLabelOfModel() const;

  //! Accessor for the label representing the given object ID.
  //! \param[in] id object ID of interest.
  //! \return object's label.
  asiAsm_EXPORT TDF_Label
    GetLabel(const asiAsm_XdePersistentId& id) const;

  //! Accessor for the ultimate OCAF label corresponding to the given item.
  //! \param[in] item assembly item of interest.
  //! \return ultimate label.
  asiAsm_EXPORT TDF_Label
    GetLabel(const asiAsm_XdeAssemblyItemId& item) const;

  //! Accessor for the label representing the given part.
  //! \param[in] part part of interest.
  //! \return part's label.
  asiAsm_EXPORT TDF_Label
    GetLabel(const asiAsm_XdePartId& part) const;

  //! Returns shape for the given assembly item.
  //! \param[in] item        assembly item of interest.
  //! \param[in] doTransform whether to apply transformation.
  //! \return shape.
  asiAsm_EXPORT TopoDS_Shape
    GetShape(const asiAsm_XdeAssemblyItemId& item,
             const bool                      doTransform = true) const;

  //! Returns shape for a part.
  //! \param[in] part part to get shape for.
  //! \return stored shape.
  asiAsm_EXPORT TopoDS_Shape
    GetShape(const asiAsm_XdePartId& part) const;

  //! Extracts shape from the label. Use this method for originals and prefer
  //! using GetShape() with core_AssemblyItem signature for references.
  //! \param[in] label label to extract the stored shape for.
  //! \return stored shape.
  asiAsm_EXPORT TopoDS_Shape
    GetShape(const TDF_Label& label) const;

  //! Extracts parent assembly item for the given item.
  //! \param[in]  item   assembly item of interest.
  //! \param[out] parent parent assembly item.
  //! \return true if parent exists, false -- otherwise.
  asiAsm_EXPORT bool
    GetParent(const asiAsm_XdeAssemblyItemId& item,
              asiAsm_XdeAssemblyItemId&       parent) const;

  //! Returns parent's location for the given assembly item.
  //! \param[in] item        assembly item of interest.
  //! \param[in] doTransform whether to apply transformation.
  //! \return location.
  asiAsm_EXPORT TopLoc_Location
    GetParentLocation(const asiAsm_XdeAssemblyItemId& item,
                      const bool                      doTransform = true) const;

  //! Returns own (without parent's) location for the given assembly item.
  //! \param[in] item        assembly item of interest.
  //! \return location.
  asiAsm_EXPORT TopLoc_Location
    GetOwnLocation(const asiAsm_XdeAssemblyItemId& item) const;

  //! \return geometry only.
  asiAsm_EXPORT TopoDS_Shape
    GetOneShape() const;

  //! Collects shapes corresponding to the given assembly items.
  //! \param items [in] assembly items.
  //! \return compound of assembly item shapes.
  asiAsm_EXPORT TopoDS_Shape
    GetOneShape(const asiAsm_XdeAssemblyItemIds& items) const;

  //! Returns OCAF labels corresponding to roots.
  //! \param[out] labels labels of roots
  asiAsm_EXPORT void
    GetLabelsOfRoots(TDF_LabelSequence& labels) const;

  //! Returns root assembly items.
  //! \param[out] items roots assembly items
  asiAsm_EXPORT void
    GetRootAssemblyItems(asiAsm_XdeAssemblyItemIds& items) const;

  //! Returns assembly items corresponding to individual parts. The good point
  //! about this method is that it returns the user-ready data, i.e. only those
  //! items which the user normally sees in an assembly explorer (in UI).
  //! It means that only references (instances) will be returned if they exist,
  //! so no original shapes will appear in the list. However, if no instances
  //! exist, the original item will be returned, so they will not be missed.
  //!
  //! \param[out] items assembly items representing ultimate leafs in
  //!                   the assembly structure.
  asiAsm_EXPORT void
    GetLeafAssemblyItems(asiAsm_XdeAssemblyItemIds& items) const;

  //! Returns assembly items corresponding to individual parts. The good point
  //! about this method is that it returns the user-ready data, i.e. only those
  //! items which the user normally sees in an assembly explorer (in UI).
  //! It means that only references (instances) will be returned if they exist,
  //! so no original shapes will appear in the list. However, if no instances
  //! exist, the original item will be returned, so they will not be missed.
  //!
  //! \param[out] items assembly items representing ultimate leafs in
  //!                   the assembly structure.
  asiAsm_EXPORT void
    GetLeafAssemblyItems(const Handle(asiAsm_XdeHAssemblyItemIdsMap)& items) const;

  //! Extracts all leafs from the given assembly items. It can be the item
  //! itself from the input list, or, in case of sub-assembly input, the
  //! ultimate children accessed recursively. This method never returns
  //! sub-assemblies, only the parts should be expected in its output.
  //!
  //! \param[in]  parents parent (sub-)assemblies (parts are also allowed: they
  //!                     will be directly transferred to the output).
  //! \param[out] items   output collection of items (sub-assemblies will never
  //!                     appear here).
  asiAsm_EXPORT void
    GetLeafAssemblyItems(const asiAsm_XdeAssemblyItemIds&             parents,
                         const Handle(asiAsm_XdeHAssemblyItemIdsMap)& items) const;

  //! Extracts all leafs from the given assembly items. It can be the item
  //! itself from the input list, or, in case of sub-assembly input, the
  //! ultimate children accessed recursively. This method never returns
  //! sub-assemblies, only the parts should be expected in its output.
  //!
  //! \param[in]  parents parent (sub-)assemblies (parts are also allowed: they
  //!                     will be directly transferred to the output).
  //! \param[out] items   output collection of items (sub-assemblies will never
  //!                     appear here).
  asiAsm_EXPORT void
    GetLeafAssemblyItems(const Handle(asiAsm_XdeHAssemblyItemIdsMap)& parents,
                         const Handle(asiAsm_XdeHAssemblyItemIdsMap)& items) const;

  //! Returns all ultimate components for the given assembly item (the parent one).
  //! If the passed parent is itself a part, it will be simply transferred to
  //! the output without any additional processing.
  //!
  //! \param[in]  parent parent assembly item to gather all parts for.
  //! \param[out] items  output collection of ultimate items.
  asiAsm_EXPORT void
    GetLeafAssemblyItems(const asiAsm_XdeAssemblyItemId&              parent,
                         const Handle(asiAsm_XdeHAssemblyItemIdsMap)& items) const;

  //! Extracts all parts from the given assembly items. It can be the item
  //! itself from the input list, or, in case of sub-assembly input, the
  //! ultimate children accessed recursively. This method never returns
  //! sub-assemblies, only the parts should be expected in its output.
  //!
  //! \param[in]  parents parent (sub-)assemblies (parts are also allowed: they
  //!                     will be directly transferred to the output).
  //! \param[out] items   output collection of items (sub-assemblies will never
  //!                     appear here).
  asiAsm_EXPORT void
    GetLeafAssemblyItems(const asiAsm_XdeAssemblyItemIds& parents,
                         asiAsm_XdeAssemblyItemIds&       items) const;

  //! Returns all ultimate components for the given assembly item (the parent one).
  //! If the passed parent is itself a part, it will be simply transferred to
  //! the output without any additional processing.
  //!
  //! \param[in]  parent parent assembly item to gather all parts for.
  //! \param[out] items  output collection of ultimate items.
  asiAsm_EXPORT void
    GetLeafAssemblyItems(const asiAsm_XdeAssemblyItemId& parent,
                         asiAsm_XdeAssemblyItemIds&      items) const;

  //! Collects all replicas for the given list.
  //! \param[in]  label    original's label.
  //! \param[out] replicas labels of replicas.
  asiAsm_EXPORT void
    GetLabelsOfReplicas(const TDF_Label&   partLabel,
                        TDF_LabelSequence& replicas);

  //! Gathers all assembly items for the given part.
  //! Put part as assembly item if it is Free shape (has no instances).
  //! \param[in]  part  part ID.
  //! \param[out] items assembly items referring to the given part.
  asiAsm_EXPORT void
    GetAssemblyItemsForPart(const asiAsm_XdePartId&    part,
                            asiAsm_XdeAssemblyItemIds& items) const;

  //! Gathers all assembly items for the given parts.
  //! Put parts as assembly item if it is Free shape (has no instances).
  //! \param[in]  parts part IDs.
  //! \param[out] items assembly items referring to the given parts.
  asiAsm_EXPORT void
    GetAssemblyItemsForParts(const asiAsm_XdePartIds&   parts,
                             asiAsm_XdeAssemblyItemIds& items) const;

  //! Gathers all assembly items for the labels map.
  //! \param[in]  original original labels map.
  //! \param[out] items    assembly items referring to the given original.
  asiAsm_EXPORT void
    GetAssemblyItemsForParts(const TDF_LabelMap&        originals,
                             asiAsm_XdeAssemblyItemIds& items) const;

  //! Gathers all assembly items for the labels map.
  //! \param[in]  original original labels map.
  //! \param[out] items    assembly items referring to the given original.
  asiAsm_EXPORT void
    GetAssemblyItemsForParts(const TDF_LabelMap&                          originals,
                             const Handle(asiAsm_XdeHAssemblyItemIdsMap)& items) const;

  //! Gathers all assembly items for the given original.
  //! \param[in]  original original label.
  //! \param[out] items    assembly items referring to the given original.
  asiAsm_EXPORT void
    GetAssemblyItemsForPart(const TDF_Label&           original,
                            asiAsm_XdeAssemblyItemIds& items) const;

  //! Gathers all assembly items for the given original.
  //! \param[in]  original original label.
  //! \param[out] items    assembly items referring to the given original.
  asiAsm_EXPORT void
    GetAssemblyItemsForPart(const TDF_Label&                             original,
                            const Handle(asiAsm_XdeHAssemblyItemIdsMap)& items) const;

  //! For the given assembly item, this method will search for all partners,
  //! i.e. the assembly items sharing the same original.
  //! \param[in]  anyItem  item to search partners for.
  //! \param[out] partners partner items.
  asiAsm_EXPORT void
    GetPartners(const asiAsm_XdeAssemblyItemId& anyItem,
                asiAsm_XdeAssemblyItemIds&      partners) const;

  //! For the given assembly item, this method will search for all partners,
  //! i.e. the assembly items sharing the same original.
  //! \param[in]  anyItem  item to search partners for.
  //! \param[out] partners partner items.
  asiAsm_EXPORT void
    GetPartners(const TDF_Label&           original,
                asiAsm_XdeAssemblyItemIds& partners) const;

  //! For the given assembly items, this method will search for all partners,
  //! i.e. the assembly items sharing the same original.
  //! \param[in]  anyItems items to search partners for.
  //! \param[out] partners partner items.
  asiAsm_EXPORT void
    GetPartners(const asiAsm_XdeAssemblyItemIds& anyItems,
                asiAsm_XdeAssemblyItemIds&       partners) const;

  //! For the given assembly items, this method will search for all partners,
  //! i.e. the assembly items sharing the same original.
  //! \param[in]  anyItems items to search partners for.
  //! \param[out] partners partner items.
  asiAsm_EXPORT void
    GetPartners(const Handle(asiAsm_XdeHAssemblyItemIdsMap)& anyItems,
                asiAsm_XdeAssemblyItemIds&                   partners) const;

  //! For the given assembly items, this method will search for all partners,
  //! i.e. the assembly items sharing the same original.
  //! \param[in]  anyItems items to search partners for.
  //! \param[out] partners partner items.
  asiAsm_EXPORT void
    GetPartners(const Handle(asiAsm_XdeHAssemblyItemIdsMap)& anyItems,
                Handle(asiAsm_XdeHAssemblyItemIdsMap)&       partners) const;

  //! Returns RGB color associated with the given part.
  //! \param[in]  partId part ID in question.
  //! \param[out] color  associated color.
  //! \return true if color exists, false -- otherwise.
  asiAsm_EXPORT bool
    GetColor(const asiAsm_XdePartId& partId,
             Quantity_Color&         color) const;

  //! Returns RGBA color associated with the given part.
  //! \param[in]  partId part ID in question.
  //! \param[out] color  associated color.
  //! \return true if color exists, false -- otherwise.
  asiAsm_EXPORT bool
    GetColor(const asiAsm_XdePartId& partId,
             Quantity_ColorRGBA&     color) const;

  //! Returns color associated with the given label.
  //! \param[in]  label OCAF label of the prototype in question.
  //! \param[out] color associated color.
  //! \return true if color exists, false -- otherwise.
  asiAsm_EXPORT bool
    GetColor(const TDF_Label&    label,
             Quantity_ColorRGBA& color) const;

  //! Returns the alpha color component associated with the given part.
  //! \param[in]  partId the part in question.
  //! \param[out] alpha  the associated alpha color component.
  //! \return the Boolean flag indicating whether the accociated color with a
  //!         transparency component was found or not.
  asiAsm_EXPORT bool
    GetColorAlpha(const asiAsm_XdePartId& partId,
                  double&                 alpha);

  //! Returns color associated with the subshape of the input part.
  //! \param[in]  partId   part ID in question.
  //! \param[in]  subShape subshape to get color for.
  //! \param[out] color    associated color.
  //! \return true if color exists, false -- otherwise.
  asiAsm_EXPORT bool
    GetSubShapeColor(const asiAsm_XdePartId& partId,
                     const TopoDS_Shape&     subShape,
                     Quantity_ColorRGBA&     color) const;

  //! Sets the given color to the shape on the passed label.
  //! \param[in] label label with shape to set the passed color for.
  //! \param[in] color color to set.
  asiAsm_EXPORT void
    SetColor(const TDF_Label&      label,
             const Quantity_Color& color);

  //! Sets the given color to the shape on the passed label.
  //! \param[in] label        label with shape to set the passed color for.
  //! \param[in] color        color to set.
  //! \param[in] changeTransp indicates whether to keep the transparency
  //!                         as a distinct data chunk or make it a color's
  //!                         transparency component.
  asiAsm_EXPORT void
    SetColor(const TDF_Label&          label,
             const Quantity_ColorRGBA& color,
             const bool                changeTransp);

  //! Takes the common color from the part's faces and assigns it to the
  //! part itself. This function works only if all faces are colorized
  //! identically.
  //!
  //! \param[in] partId ID of the part in question.
  //! \param[in] force  whether to override possibly existing part's color.
  //! \return false if nothing was done, true -- otherwise.
  asiAsm_EXPORT bool
    AutoColorizePart(const asiAsm_XdePartId& partId,
                     const bool              force = false);

  //! Updates boundary representation (shape) of the given part. The passed
  //! history is used to update the related metadata (e.g., colors of faces).
  //! If no history is available, it is safe to pass `nullptr`.
  //! \param[in] partLab            label of the part to update the shape for.
  //! \param[in] newShape           new shape to set.
  //! \param[in] history            modification history to update the related metadata.
  //! \param[in] doUpdateAssemblies indicates whether to update assemblies at the end.
  asiAsm_EXPORT void
    UpdatePartShape(const TDF_Label&                 partLab,
                    const TopoDS_Shape&              newShape,
                    const Handle(BRepTools_History)& history,
                    const bool                       doUpdateAssemblies = true);

  //! XDE-specific function to call each time you modify part's geometry.
  //! Without calling this function, you'll end up with TopoDS compounds
  //! nested into your assemblies having the old geometries. That's a shitty
  //! surprise to discover once, for example, you export the document back
  //! to STEP format.
  //!
  //! Find out more details here: http://quaoar.su/blog/page/step-iges-xde-opencascade
  //!
  //! The `asiAsm_XdeDoc` is responsible for calling this method internally in its
  //! API functions. Be careful not to forget calling it if you prefer modifying the parts
  //! outside this our API. Else, forget about it.
  asiAsm_EXPORT void
    UpdateAssemblies();

  //! Turns the passed part into a subassembly in the case if this part contains
  //! TopoDS_Compound as its boundary representation.
  //! \param[in] partId           the part ID in question.
  //! \param[in] updateAssemblies the Boolean flag indicating whether to call `UpdateAssemblies()`
  //!                             method upon completion.
  //! \return true if the part of expanded, false -- otherwise (e.g., if the part is not
  //!         of a compound geometric type).
  asiAsm_EXPORT bool
    ExpandCompound(const asiAsm_XdePartId& partId,
                   const bool              updateAssemblies = true);

  //! Expands the compound parts by turning them into subassemblies. This
  //! function does recursive processing, so that the nested compound parts
  //! will be expanded as well. It goes like this until a non-compound part is
  //! reached.
  //! \param[in] items the assembly items to start expansion from.
  asiAsm_EXPORT void
    ExpandCompounds(const asiAsm_XdeAssemblyItemIds& items);

  //! Creates a new part with the given shape as a primary representation.
  //! \param[in] shape the shape to add as a part.
  //! \param[in] name  the part's name.
  //! \return ID of the newly added part.
  asiAsm_EXPORT asiAsm_XdePartId
    AddPart(const TopoDS_Shape& shape = TopoDS_Shape(),
            const std::string&  name  = "");

  //! Creates a new part with the given name.
  //! \param[in] name the part's name.
  //! \return ID of the newly added part.
  asiAsm_EXPORT asiAsm_XdePartId
    AddPart(const std::string& name);

  //! Removes all given parts with their instances from the document.
  //! \param[in] parts              parts to remove.
  //! \param[in] doUpdateAssemblies indicates whether to update assemblies
  //!                               upon removal of parts.
  //! \return true in case of success, false -- otherwise.
  asiAsm_EXPORT bool
    RemoveParts(const asiAsm_XdePartIds& parts,
                const bool               doUpdateAssemblies = true);

  //! Removes all empty assemblies and their components.
  asiAsm_EXPORT void
    RemoveAllEmptyAssemblies();

public:

  //! Dumps assembly hierarchy to the passed output stream.
  //! \param[in,out] output stream where to dump.
  asiAsm_EXPORT void
    DumpAssemblyItems(Standard_OStream& out) const;

  //! Non-const accessor for the underlying OCAF Document.
  //! \return OCAF Document.
  asiAsm_EXPORT Handle(TDocStd_Document)&
    ChangeDocument();

  //! Accessor for the underlying OCAF Document.
  //! \return OCAF Document.
  asiAsm_EXPORT const Handle(TDocStd_Document)&
    GetDocument() const;

  //! \return shape tool.
  asiAsm_EXPORT Handle(XCAFDoc_ShapeTool)
    GetShapeTool() const;

  //! \return color tool.
  asiAsm_EXPORT Handle(XCAFDoc_ColorTool)
    GetColorTool() const;

protected:

  //! Initializes Data Model with the passed CAF Document and prepares integral
  //! Data Model Engines.
  //! \param[in] doc CAF Document to initialize the Model with.
  asiAsm_EXPORT void
    init(const Handle(TDocStd_Document)& doc);

  //! Creates new CAF Document.
  //! \return just allocated CAF Document.
  asiAsm_EXPORT Handle(TDocStd_Document)
    newDocument();

  //! \return instance of XDE Application which manages XDE Document.
  asiAsm_EXPORT Handle(asiAsm_XdeApp)
    getApplication();

  //! Starting from the given assembly item, iterates assembly structure downwards
  //! collecting all paths (assembly item IDs) which are recognized as leaf items.
  //! \param[in]  parent    assembly item ID in question.
  //! \param[out] itemsMap  map of collected leaf items (filled in priority in case of not NULL value).
  //! \param[out] items     vector of collected leaf items.
  //! \param[out] traversed map of already traversed items.
  asiAsm_EXPORT void
    getLeafItems(asiAsm_XdeAssemblyItemId                     parent,
                 const Handle(asiAsm_XdeHAssemblyItemIdsMap)& itemsMap,
                 asiAsm_XdeAssemblyItemIds&                   items,
                 const Handle(asiAsm_XdeHAssemblyItemIdsMap)& traversed) const;

  //! For the given list of originals, this method extracts parts.
  //!
  //! \param[in]  originals sequence of originals.
  //! \param[out] parts     extracted parts.
  asiAsm_EXPORT void
    getParts(const TDF_LabelSequence& originals,
             asiAsm_XdePartIds&       parts) const;

  //! Auxiliary method to convert labels to part IDs.
  //!
  //! \param[in]  origInstances  map of labels to instances to processed.
  //! \param[out] partsInstances output map of part IDs.
  asiAsm_EXPORT void
    getPartsWithInstances(const asiAsm_XdeLabelsToInstancesMap& origInstances,
                          asiAsm_XdePartsToInstancesMap&        partsInstances) const;

  //! Add the item of interest to the proper label in the given map.
  //!
  //! \param[in,out] item          item of interest.
  //! \param[out]    origInstances output map of labels to instances.
  asiAsm_EXPORT void
    getOriginalsWithInstances(const asiAsm_XdeAssemblyItemId& item,
                              asiAsm_XdeLabelsToInstancesMap& origInstances) const;

  //! Auxiliary method to get assembly items for part.
  //!
  //! \param[in]  original label of original.
  //! \param[in]  item     current assembly item to processed (check).
  //! \param[out] itemsMap output map of assembly item ID (filled in priority in case of not NULL value).
  //! \param[out] items    output vector of assembly item IDs.
  asiAsm_EXPORT void
    getAssemblyItemsForPart(const TDF_Label&                             original,
                            const asiAsm_XdeAssemblyItemId&              item,
                            const Handle(asiAsm_XdeHAssemblyItemIdsMap)& itemsMap,
                            asiAsm_XdeAssemblyItemIds&                   items) const;

  //! Clears working session which is an internal data structure used by
  //! data translators of OpenCascade.
  //! \param[in] WS working session to release.
  asiAsm_EXPORT void
    clearSession(const Handle(XSControl_WorkSession)& WS);

  //! This method recursively turns the compound parts addressed by the
  //! passed assembly items into subassemblies.
  //! \param[in]     items     the assembly items in question.
  //! \param[in,out] processed the processed prototypes' labels.
  asiAsm_EXPORT void
    expandCompoundsRecursively(const asiAsm_XdeAssemblyItemIds& items,
                               TDF_LabelMap&                    processed);

  //! Internal mechanics for compounds expansion. This method works the
  //! at TDF_Label's level.
  asiAsm_EXPORT void
    expand(const TDF_Label&                                      expandedLabel,
           const TopLoc_Location&                                curLoc,
           t_expansionMap&                                       subshapeMap,
           std::vector< std::pair<TDF_Label, TopLoc_Location> >& newParts);

  //! Propagates color attributes (of all types) from the given assembly label
  //! down to its components recursively.
  asiAsm_EXPORT void
    propagateColor(const TDF_Label&          assemblyLabel,
                   const bool                isSurfColoredAssembly,
                   const Quantity_ColorRGBA& surfColor,
                   const bool                isCurvColoredAssembly,
                   const Quantity_ColorRGBA& curvColor,
                   const bool                isGenColoredAssembly,
                   const Quantity_ColorRGBA& genColor);

  //! Returns a "common" color of a part taking into account the colors of its
  //! faces. If all faces of the part have the same color, the method
  //! returns this unique color and sets the output argument `isOnFaces` to `true`.
  //! If at least one face has a different color or has no color, the output
  //! argument `isOnFaces` is set to `false`.
  //!
  //! \param[in]  part              ID of the part in question.
  //! \param[out] color             resulting color.
  //! \param[out] isOnFaces         indicates whether the method identifies
  //!                               that all faces have the same color.
  //! \param[in]  isIgnorePartColor determines the priority of color selection:
  //!                               true  -- common color of faces is selected, color
  //!                                        of the part itself is ignored;
  //!                               false -- if the part has a color associated,
  //!                                        that color will be returned without
  //!                                        checking the colors of sub-shapes.
  asiAsm_EXPORT void
    getCommonColor(const asiAsm_XdePartId& partId,
                   Quantity_Color&         color,
                   bool&                   isOnFaces,
                   const bool              isIgnorePartColor = false) const;

  //! Copies all OCAF/XDE attributes from the `from` label to the `to` label.
  //! \param[in] from the source label.
  //! \param[in] to   the target label.
  asiAsm_EXPORT void
    copyAttributes(const TDF_Label from,
                   TDF_Label&      to);

  //! Adda a new component with the given location to the passed assembly.
  //! \param[in] assemblyLabel the target assembly's label.
  //! \param[in] compLabel     the component to add.
  //! \param[in] location      the location to attach to the added instance.
  //! \return label of the newly added component.
  asiAsm_EXPORT TDF_Label
    addComponent(const TDF_Label&       assemblyLabel,
                 const TDF_Label&       compLabel,
                 const TopLoc_Location& location);

  //! Finds assembly items targeting a persistent label with the specified
  //! name. This is a recursive DFS method.
  //! \param[in]     asmGraph the assembly graph to iterate over.
  //! \param[in]     parentId the parent node's ID in the assembly graph.
  //! \param[in]     name     the name being searched for.
  //! \param[in,out] path     the current path to the item.
  //! \param[out]    items    the found items.
  asiAsm_EXPORT void
    findItemsRecursively(const Handle(asiAsm_XdeGraph)&         asmGraph,
                         const int                              parentId,
                         const std::string&                     name,
                         std::vector<int>&                      path,
                         Handle(asiAsm_XdeHAssemblyItemIdsMap)& items) const;

  //! Removes the given assembly in the case it's empty.
  //! \param[in] assembly the assembly label to remove.
  //! \param[in] isUpdate the Boolean flag indicating whether to perform update.
  //! \return true in case of success, false otherwise.
  asiAsm_EXPORT bool
    removeEmptyAssembly(const TDF_Label& assembly,
                        const bool       isUpdate = true);

  //! Removes the passed (sub)assembly together with all its hierarchically
  //! nested components in the case its hierarchy is orphan (i.e., it contains
  //! no real product data).
  //! \param[in] assembly the target assembly label.
  asiAsm_EXPORT void
    removeEmptySubAssemblies(const TDF_Label& assembly);

private:

  //! This function is a lightweight analogue of GetComponents() method of
  //! XCAFDoc_ShapeTool. This function does not perform any additional checks
  //! and simply takes child labels for the given father label. It also
  //! populates a vector instead of a sequence for the sake to provide
  //! efficient direct access.
  void __getComponents(const TDF_Label& l, std::vector<TDF_Label>& labels) const;

  //! Faster analogue of TDF_Tool::Entry() which uses cache of labels and their
  //! entries.
  //! \param[in]  label label to access entry for.
  //! \param[out] entry entry composed dynamically or taken from cache.
  void __entry(const TDF_Label&         label,
               TCollection_AsciiString& entry) const;

  //! Faster analogue of IsInstance() method from xde_AssemblyDoc. The difference
  //! here is that Shape Tool is not accessed but simply passed.
  //! \param[in]  ST        XDE Shape Tool.
  //! \param[in]  itemLab   label to check.
  //! \param[out] originLab label of the prototype entity (part or subassembly).
  //! \return true/false.
  bool __isInstance(const Handle(XCAFDoc_ShapeTool)& ST,
                    const TDF_Label&                 itemLab,
                    TDF_Label&                       originLab) const;

  //! Adds the passed shape as a part without any checks. This method does not
  //! use `XCAFDoc_ShapeTool` of XDE, so it adds labels/attributes in the most
  //! straighforward way.
  //! \param[in] shape the shape to add as a new part.
  //! \param[in] name  the part name.
  //! \return the newly added label.
  TDF_Label __addPart(const TopoDS_Shape& shape,
                      const std::string&  name = "");

  //! Adds a subshape's label under the passed part's label. This method does not
  //! use `XCAFDoc_ShapeTool` of XDE, so it adds labels/attributes in the most
  //! straighforward way.
  //! \param[in] partLabel the part's label.
  //! \param[in] subshape  the subshape to add a label for.
  //! \return the newly created label.
  TDF_Label __addSubShape(const TDF_Label&    partLabel,
                          const TopoDS_Shape& subshape);

protected:

  Handle(TDocStd_Document) m_doc; //!< Underlying XCAF document.

  //! Label-entry cache for fast accessing entries by labels. Using this map,
  //! we can speed up a bit the code like TDF_Tool::Entry() which uses
  //! dynamic memory allocations to make strings from numeric tags.
  //!
  //! This caching technique is quite safe because labels are never deleted
  //! in OCAF sessions.
  mutable NCollection_DataMap<TDF_Label,
                              TCollection_AsciiString,
                              TDF_LabelMapHasher> m_LECache;

  /* Diagnostics tools */

  mutable ActAPI_ProgressEntry m_progress; //!< Progress notifier.
  mutable ActAPI_PlotterEntry  m_plotter;  //!< Imperative plotter.

};

#endif
