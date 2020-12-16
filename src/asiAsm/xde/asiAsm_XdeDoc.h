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

// OpenCascade includes
#include <TDocStd_Document.hxx>

// Active Data includes
#include <ActAPI_IPlotter.h>
#include <ActAPI_IProgressNotifier.h>

// OpenCascade includes
#include <TDF_LabelSequence.hxx>
#include <XCAFApp_Application.hxx>
#include <XCAFDoc_DocumentTool.hxx>

// Forward declarations.
class XSControl_WorkSession;

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
  asiAsm_EXPORT virtual void
    NewDocument();

  //! Loads STEP file to populate the internal XDE Document.
  //! \param[in] filename name of the STEP file to load.
  //! \return true in case of success, false -- otherwise.
  asiAsm_EXPORT virtual bool
    LoadSTEP(const TCollection_AsciiString& filename);

  //! \return true if the Assembly Document is empty, i.e. the XDE Document
  //!         is either null or contains no roots.
  asiAsm_EXPORT virtual bool
    IsEmpty() const;

  //! Releases all resources occupied by this Assembly Document instance.
  asiAsm_EXPORT virtual void
    Release();

/* API */
public:

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
  //! \param[in]  partLabel part label.
  //! \param[out] replicas  labels of replicas.
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

  //! Dumps assembly hierarchy to the passed output stream.
  //! \param[in,out] output stream where to dump.
  asiAsm_EXPORT void
    DumpAssemblyItems(Standard_OStream& out) const;

public:

  //! Non-const accessor for the underlying OCAF Document.
  //! \return OCAF Document.
  Handle(TDocStd_Document)& ChangeDocument()
  {
    return m_doc;
  }

  //! Accessor for the underlying OCAF Document.
  //! \return OCAF Document.
  const Handle(TDocStd_Document)& GetDocument() const
  {
    return m_doc;
  }

  //! \return shape tool.
  Handle(XCAFDoc_ShapeTool) GetShapeTool() const
  {
    return XCAFDoc_DocumentTool::ShapeTool( m_doc->Main() );
  }

// Construction & initialization:
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
  asiAsm_EXPORT Handle(XCAFApp_Application)
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
