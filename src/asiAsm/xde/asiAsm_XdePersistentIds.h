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

#ifndef asiAsm_XdePersistentIds_h
#define asiAsm_XdePersistentIds_h

// asiAsm includes
#include <asiAsm.h>

// asiAlgo includes
#include <asiAlgo_Utils.h>

// OCCT includes
#include <NCollection_Vector.hxx>
#include <NCollection_Sequence.hxx>
#include <NCollection_Shared.hxx>
#include <TCollection_HAsciiString.hxx>
#include <TDF_Label.hxx>
#include <TDF_LabelMapHasher.hxx>
#include <TDF_Tool.hxx>

//-----------------------------------------------------------------------------

namespace asiAsm {
namespace xde {

//! \ingroup ASIASM
//!
//! Persistent object ID which is an entry from an XDE Document.
typedef TCollection_AsciiString PersistentId;

//! \ingroup ASIASM
//!
//! Convenience type for a collection of persistent IDs.
typedef NCollection_Vector<PersistentId> PersistentIds;

//! \ingroup ASIASM
//!
//! List of persistent IDs.
typedef NCollection_List<PersistentId> PersistentIdList;

//! \ingroup ASIASM
//!
//! ID of an item in the assembly structure. The assembly structure in
//! XDE Document is essentially a Hierarchical Assembly Graph (HAG). See
//!
//! [A. Rappoport, A scheme for single instance representation in hierarchical
//!  assembly graphs, in: B. Falcidieno, T. L. Kunii (Eds.), Modeling in
//!  Computer Graphics, Springer Berlin Heidelberg, Berlin, Heidelberg,
//!  1993, pp. 213-223]
//!
//! for the overview together with discussion of instancing issues. An
//! assembly item is an element of a scene tree which is a dual representation
//! of an assembly graph. This element is identified by the path to this item
//! through the nodes of a HAG. We use a string representation of this complex
//! identifier which works quite well on hashing for data maps. The key here
//! is to avoid dynamic allocation/deallocation of memory.
class AssemblyItemId
{
friend class Doc;
friend class DocIterator;

public:

  //! Constructs assembly item ID from the passed shared string.
  //! \param[in] path string to construct an assembly item ID from.
  //! \return newly created assembly item ID.
  static AssemblyItemId FromPath(const Handle(TCollection_HAsciiString)& path)
  {
    return AssemblyItemId(path);
  }

  //! Constructs assembly item ID from the passed string.
  //! \param[in] path string to construct an assembly item ID from.
  //! \return newly created assembly item ID.
  static AssemblyItemId FromPath(const TCollection_AsciiString& path)
  {
    return AssemblyItemId(path);
  }

  //! Constructs assembly item ID from the passed path.
  //! \param[in] path path vector to construct an assembly item ID from.
  //! \return newly created assembly item ID.
  static AssemblyItemId FromPath(const NCollection_Sequence<PersistentId>& path)
  {
    return AssemblyItemId(path);
  }

public:

  //! Hasher for data maps.
  struct Hasher
  {
    //! HashCode() function to be used in OCCT Data Maps.
    //! \param[in] item  item to calculate a hash code for.
    //! \param[in] upper upper index.
    //! \return hash code.
    static int HashCode(const AssemblyItemId& item,
                        const int             upper)
    {
      return ::HashCode( item.ToString(), upper );
    }

    //! IsEqual() function for items to be used in OCCT Data Maps.
    //! \param[in] item1 first item.
    //! \param[in] item2 second item.
    //! \return true in case of equality, false -- otherwise.
    static int IsEqual(const AssemblyItemId& item1,
                       const AssemblyItemId& item2)
    {
      return ::IsEqual( item1.ToString(), item2.ToString() );
    }
  };

public:

  //! Default ctor.
  AssemblyItemId()
  {
    this->m_pathStr = new TCollection_HAsciiString;
  }

  //! Ctor accepting path as a shared string.
  //! \param[in] path string representation of a path.
  AssemblyItemId(const Handle(TCollection_HAsciiString)& path)
  {
    this->m_pathStr = path;
  }

  //! Ctor accepting path as a string.
  //! \param[in] path string representation of a path.
  AssemblyItemId(const TCollection_AsciiString& path)
  {
    this->m_pathStr = new TCollection_HAsciiString(path);
  }

  //! Ctor accepting path as an ordered collection of entries.
  //! \param[in] path path vector to construct an assembly item ID from.
  AssemblyItemId(const NCollection_Sequence<PersistentId>& path)
  {
    if ( this->m_pathStr.IsNull() )
      this->m_pathStr = new TCollection_HAsciiString;

    int numEntries = path.Length();
    int idx = 1;
    for ( NCollection_Sequence<PersistentId>::Iterator it(path); it.More(); it.Next(), ++idx )
    {
      this->m_pathStr = this->m_pathStr->Cat( it.Value().ToCString() );

      if ( idx < numEntries )
        this->m_pathStr = this->m_pathStr->Cat("/");
    }
  }

  //! Ctor accepting path as an vector of entries.
  //! \param[in] path path vector to construct an assembly item ID from.
  AssemblyItemId(const std::vector<PersistentId>& path)
  {
    if ( this->m_pathStr.IsNull() )
      this->m_pathStr = new TCollection_HAsciiString;

    int numEntries = int( path.size() );
    int idx = 1;
    for ( int k = 0; k < numEntries; ++k, ++idx )
    {
      this->m_pathStr = this->m_pathStr->Cat( path[k].ToCString() );

      if ( idx < numEntries )
        this->m_pathStr = this->m_pathStr->Cat("/");
    }
  }

  //! Copy ctor.
  //! \param[in] aiid assembly item ID to copy.
  AssemblyItemId(const AssemblyItemId& aiid)
  {
    this->operator=(aiid);
  }

public:

  //! \return path to the item as a string.
  const TCollection_AsciiString& ToString() const
  {
    return this->m_pathStr->String();
  }

  //! \return path to the item as a string.
  const Handle(TCollection_HAsciiString)& ToHString() const
  {
    return this->m_pathStr;
  }

  //! \param[in] item item to check.
  //! \return true if this item is equal to the passed item.
  bool IsEqual(const AssemblyItemId& item) const
  {
    if ( this->m_pathStr->Length() != item.m_pathStr->Length() )
      return false;

    return ::IsEqual( this->ToString(), item.ToString() );
  }

  //! \return true if this assembly item is not initialized.
  bool IsNull() const
  {
    return m_pathStr.IsNull() || m_pathStr->IsEmpty();
  }

  //! Clears the path.
  void Nullify()
  {
    m_pathStr.Nullify();
  }

  //! Dumps assembly item to the passed output stream.
  //! \param[out] out output stream.
  void Dump(Standard_OStream& out) const
  {
    out << "Assembly Item: " << this->ToString().ToCString() << "\n";
  }

  //! Prepends persistent ID to the assembly item ID.
  //! \param[in] persistentId ID (entry) to prepend.
  //! \return non-const reference to this instance.
  AssemblyItemId& Prepend(const PersistentId& persistentId)
  {
    return this->append(persistentId, true);
  }

public:

  //! Appends persistent ID to the assembly item ID.
  //! \param[in] persistentId ID (entry) to append.
  //! \return non-const reference to this instance.
  AssemblyItemId& operator<<(const PersistentId& persistentId)
  {
    return this->append(persistentId);
  }

  //! Checks if this assembly item ID is equal to the passed one.
  //! \param[in] otherId other assembly item ID.
  //! \return true/false.
  bool operator==(const AssemblyItemId& otherId) const
  {
    return this->ToString() == otherId.ToString();
  }

  //! Checks if this assembly item ID is not equal to the passed one.
  //! \param[in] otherId other assembly item ID.
  //! \return true/false.
  bool operator!=(const AssemblyItemId& otherId) const
  {
    return !this->operator==(otherId);
  }

  //! Direct access operator.
  //! \param[in] oneBasedIdx 1-based index of the entry to access.
  //! \return entry (object ID).
  const TCollection_AsciiString& operator()(const int oneBasedIdx) const
  {
    this->preparePath();

    return this->m_pathCache[oneBasedIdx - 1];
  }

  //! Assignment operator.
  //! \param[in] aaid item to copy.
  void operator=(const AssemblyItemId& aiid)
  {
    this->m_pathStr   = aiid.m_pathStr;
    this->m_label     = aiid.m_label;
    this->m_jumpLabel = aiid.m_jumpLabel;
    this->m_entry     = aiid.m_entry;
    this->m_pathCache = aiid.m_pathCache;
  }

public:

  //! \return path to the item as a sequence.
  const std::vector<TCollection_AsciiString>& GetPath() const
  {
    this->preparePath();

    return this->m_pathCache;
  }

  //! \return length of the path.
  int GetPathLength() const
  {
    this->preparePath();

    return int( this->m_pathCache.size() );
  }

  //! Removes a entry from the path.
  //! \param[in] oneBasedIdx 1-based index of the entry to remove.
  void Remove(const int oneBasedIdx)
  {
    this->preparePath();

    std::vector<TCollection_AsciiString> newChunks;
    //
    for ( size_t k = 0; k < this->m_pathCache.size(); ++k )
    {
      if ( (k + 1) != oneBasedIdx )
        newChunks.push_back(this->m_pathCache[k]);
    }

    Handle(TCollection_HAsciiString)
      newPathStr = new TCollection_HAsciiString;

    int numEntries = int( newChunks.size() );
    int idx = 1;
    for ( size_t k = 0; k < newChunks.size(); ++k, ++idx )
    {
      newPathStr = newPathStr->Cat( new TCollection_HAsciiString(newChunks[k]) );

      if ( idx < numEntries )
        newPathStr = newPathStr->Cat("/");
    }

    this->m_pathStr = newPathStr;
    this->clearPath();
    this->preparePath();
  }

  //! \return last entry in path.
  const TCollection_AsciiString& GetLastEntry() const
  {
    this->preparePath();

    return this->m_pathCache[this->m_pathCache.size() - 1];
  }

  //! \param[in] item ID of the assembly item to check.
  //! \return true if this item is a child of the given item.
  bool IsChild(const AssemblyItemId& item) const
  {
    if ( this->m_pathStr->Length() <= item.m_pathStr->Length() )
      return false;

    this->preparePath();
    item.preparePath();

    for ( int i = 0; i < item.m_pathCache.size(); ++i )
    {
      if ( this->m_pathCache[i] != item.m_pathCache[i] )
        return false;
    }
    return true;
  }

private:

  //! Cleans up the cached path.
  void clearPath()
  {
    m_pathCache.clear();
  }

  //! Builds a collection of entries if not yet available.
  void preparePath() const
  {
    if ( !m_pathCache.size() )
      asiAlgo_Utils::Str::Split(this->m_pathStr->String(), "/", m_pathCache);
  }

  //! Appends passed id to the corresponding place according to passed flag.
  AssemblyItemId& append(const PersistentId& persistentId,
                         const bool          atTheBegining = false)
  {
    this->clearPath();

    if ( this->m_pathStr.IsNull() )
      this->m_pathStr = new TCollection_HAsciiString;

    if ( !this->m_pathStr->IsEmpty() )
    {
      bool shouldAddDelimiter = atTheBegining ? this->m_pathStr->Value(1) != '/'
                                              : this->m_pathStr->Value( this->m_pathStr->Length() ) != '/';
      if ( shouldAddDelimiter )
      {
        Handle(TCollection_HAsciiString) delimiter = new TCollection_HAsciiString("/");
        atTheBegining ? this->m_pathStr->Prepend( delimiter )
                      : this->m_pathStr->AssignCat( delimiter );
      }
    }
    Handle(TCollection_HAsciiString) idToAdd = new TCollection_HAsciiString(persistentId);
    atTheBegining ? this->m_pathStr->Prepend( idToAdd )
                  : this->m_pathStr->AssignCat( idToAdd );
    return *this;
  }

protected:

  //! Unique identifier of the assembly item is the patch to the assembly
  //! component through OCAF/XDE labels.
  Handle(TCollection_HAsciiString) m_pathStr;

  //! Cached path.
  mutable std::vector<TCollection_AsciiString> m_pathCache;

protected:

  TDF_Label    m_label;     //!< OCAF label for the assembly item.
  PersistentId m_entry;     //!< Entry.
  TDF_Label    m_jumpLabel; //!< OCAF label to jump for origin.

};

//-----------------------------------------------------------------------------

//! \ingroup ASIASM
//!
//! Convenience type for a collection of assembly items.
typedef NCollection_Vector<AssemblyItemId> AssemblyItemIds;

//! \ingroup ASIASM
//!
//! List of assembly items.
typedef NCollection_List<AssemblyItemId> AssemblyItemIdList;

//! \ingroup ASIASM
//!
//! Convenience type for a collection of unique assembly items.
typedef NCollection_IndexedMap<AssemblyItemId,
                               AssemblyItemId::Hasher> AssemblyItemIdMap;

//! \ingroup ASIASM
//!
//! Shared collection of assembly items.
typedef NCollection_Shared<AssemblyItemIds> HAssemblyItemIdss;

//! \ingroup ASIASM
//!
//! Shared map of assembly items.
typedef NCollection_Shared<AssemblyItemIdMap> HAssemblyItemIdsMap;

//-----------------------------------------------------------------------------

//! \ingroup ASIASM
//!
//! Part ID. There is one-to-one correspondence between parts and OCAF
//! labels in the assembly structure, so part ID is actually an entry,
//! e.g. "0:1:1:5".
struct PartId
{
  PersistentId Entry;

  static PartId FromEntry(const PersistentId& entry)
  {
    PartId result;
    result.Entry = entry;
    return result;
  }

  static PartId FromLabel(const TDF_Label& label)
  {
    PartId result;
    TDF_Tool::Entry(label, result.Entry);
    return result;
  }

  //! Default ctor.
  PartId() {}

  //! Conversion constructor.
  PartId(const PersistentId& objectId) { Entry = objectId; }

  //! \param[in] other entry to check.
  //! \return true, if entry is is equal to theOther
  bool IsEqual(const PartId& other) const
  {
    return Entry.IsEqual(other.Entry);
  }

  //! \return true if this entry is not initialized.
  bool IsNull() const
  {
    return Entry.IsEmpty();
  }

  //! \return string representation of part ID.
  TCollection_AsciiString ToString() const
  {
    return this->Entry.ToCString();
  }

  //! \param[out] out output stream.
  void Dump(Standard_OStream& out) const
  {
    out << "Part ID: " << this->Entry.ToCString() << "\n";
  }

  //! Conversion operator to core object ID which is essentially the same kind
  //! of string as part ID.
  operator PersistentId() const
  {
    return this->ToString();
  }

  //! \return last tag of the composite part ID. E.g., for "0:1:2:3" this
  //!         method returns "3".
  int GetLastTag() const
  {
    std::vector<TCollection_AsciiString> tags;
    asiAlgo_Utils::Str::Split(this->Entry, ":", tags);
    //
    if ( !tags.empty() )
      return tags[tags.size() - 1].IntegerValue();

    return -1;
  }

  //! Hasher for data maps.
  struct Hasher
  {
    //! HashCode() function to be used in OCCT Data Maps.
    //! \param[in] entry to calculate a hash code for.
    //! \param[in] upper upper index.
    //! \return hash code.
    static int HashCode(const PartId& entry,
                        const int     upper)
    {
      return ::HashCode(entry.Entry, upper);
    }

    //! IsEqual() function for entries to be used in OCCT Data Maps.
    //! \param[in] entry1 first entry.
    //! \param[in] entry2 second entry.
    //! \return true in case of equality, false -- otherwise.
    static bool IsEqual(const PartId& entry1,
                        const PartId& entry2)
    {
      return ::IsEqual(entry1.Entry, entry2.Entry);
    }
  };

};

//! \ingroup ASIASM
//!
//! Convenience type for a collection of part IDs.
typedef NCollection_Vector<PartId> PartIds;

//! \ingroup ASIASM
//!
//! Shared collection of parts.
typedef NCollection_Shared<PartIds> HPartIds;

//! \ingroup ASIASM
//!
//! Convenience type for a collection of unique parts.
typedef NCollection_IndexedMap<PartId, PartId::Hasher> PartIdMap;

//! \ingroup ASIASM
//!
//! Shared map of parts.
typedef NCollection_Shared<PartIdMap> HPartIdMap;

//! \ingroup ASIASM
//!
//! Map of parts to instances.
typedef NCollection_IndexedDataMap<PartId,
                                   AssemblyItemIdList,
                                   PartId::Hasher> PartsToInstancesMap;

//! \ingroup ASIASM
//!
//! Map of labels to instances.
typedef NCollection_IndexedDataMap<TDF_Label,
                                   AssemblyItemIdList,
                                   TDF_LabelMapHasher> LabelsToInstancesMap;

} // xde
} // asiAsm

#endif
