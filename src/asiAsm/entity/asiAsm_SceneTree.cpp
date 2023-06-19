//-----------------------------------------------------------------------------
// Created on: 19 June 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Sergey Slyadnev
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
//-----------------------------------------------------------------------------s

// Own include
#include <asiAsm_SceneTree.h>

// asiAlgo includes
#include <asiAlgo_JsonDict.h>
#include <asiAlgo_Utils.h>

// Standard includes
#include <algorithm>

// Rapidjson includes
#include <rapidjson/document.h>

//-----------------------------------------------------------------------------

typedef rapidjson::Document::Array     t_jsonArray;
typedef rapidjson::Document::ValueType t_jsonValue;

//-----------------------------------------------------------------------------

//! Base assembly tree child item.
class asiAsm_SceneTree_Child : public Standard_Transient
{
  // OpenCascade RTTI
  DEFINE_STANDARD_RTTI_INLINE( asiAsm_SceneTree_Child, Standard_Transient )

  public:

    //! Ctor.
    asiAsm_SceneTree_Child()
      : id(-1)
    {}

  public:

    int         id;
    std::string name;

  public:

    //! Checks if this scene tree's child is equal to the passed one.
    //! \param[in] other the item to test.
    //! \return true/false.
    bool IsEqual(const Handle(asiAsm_SceneTree_Child)& other) const
    {
      if ( this->id != other->id
        || this->name != other->name )
      {
        return false;
      }

      return isEqual( other );
    }

    //! Constructs the scene tree's child data structure from a JSON object.
    //! \param[in]  pJsonBlock the JSON object to construct the data structure from.
    //! \param[out] value      the outcome data structure.
    static void FromJSON(void*                                 pJsonBlock,
                         const Handle(asiAsm_SceneTree_Child)& value)
    {
      t_jsonValue*
        pJsonObj = reinterpret_cast< t_jsonValue* >( pJsonBlock );

      // Iterate members of the object.
      for ( t_jsonValue::MemberIterator mit = pJsonObj->MemberBegin();
            mit != pJsonObj->MemberEnd(); mit++ )
      {
        if ( mit->value.IsNull() )
        {
          continue;
        }

        std::string prop( mit->name.GetString() );

        // Id.
        if ( prop == asiPropName_SceneChildId )
        {
          value->id = mit->value.GetInt();
        }

        // Name.
        else if ( prop == asiPropName_SceneChildName )
        {
          value->name = mit->value.GetString();
        }

        // Handle other props.
        else
        {
          value->fromJSON( prop, &mit->value );
        }
      }
    }

  //! Converts the passed data structure to JSON (the passed `out` stream).
  //! \param[in]     value  the data structure to serialize.
  //! \param[in]     indent the pretty indentation shift.
  //! \param[in,out] out    the output JSON string stream.
  static void ToJSON(const Handle(asiAsm_SceneTree_Child)& value,
                     const int                              indent,
                     std::ostream&                          out)
  {
    std::string ws(indent, ' ');
    std::string nl = "\n" + ws;

    out        << nl << std::quoted( asiPropName_SceneChildId )   << ": " << value->id;
    out << "," << nl << std::quoted( asiPropName_SceneChildName ) << ": " << std::quoted( asiAlgo_Utils::Json::EscapeJson( value->name ) );

    // Dump delivered classes data.
    value->toJSON( indent, out );
  }

  protected:

    //! To handle properties of derived classes.
    virtual void fromJSON(const std::string&, void*)
    {}

    //! To dump properties of derived classes.
    virtual void toJSON(const int,
                        std::ostream&)
    {}

    //! To check equality of derived classes.
    virtual bool isEqual(const Handle(asiAsm_SceneTree_Child)&) const
    {
      return true;
    }

};

//-----------------------------------------------------------------------------

//! Provides properties of parts in assembly tree.
class asiAsm_SceneTree_Part : public asiAsm_SceneTree_Child
{
  // OpenCascade RTTI
  DEFINE_STANDARD_RTTI_INLINE( asiAsm_SceneTree_Part, asiAsm_SceneTree_Child )

  public:

    //! Ctor.
    asiAsm_SceneTree_Part()
      : asiAsm_SceneTree_Child()
    {}

  public:

    std::string persistentId;

  protected:

    //! Fills this part data structure from a JSON object.
    //! \param[in] prop       the JSON property name.
    //! \param[in] pJsonBlock the JSON property value.
    virtual void fromJSON(const std::string& prop,
                          void*              pJsonBlock) override
    {
      t_jsonValue*
        pJsonObj = reinterpret_cast< t_jsonValue* >( pJsonBlock );

      // Persistent id.
      if ( prop == asiPropName_ScenePartsPersistentId )
      {
        this->persistentId = pJsonObj->GetString();
      }
    }

    //! Dumps this part data structure to JSON (the passed `out` stream).
    //! \param[in]     indent the pretty indentation shift.
    //! \param[in,out] out    the output JSON string stream.
    virtual void toJSON(const int     indent,
                        std::ostream& out) override
    {
      std::string ws(indent, ' ');
      std::string nl = "\n" + ws;

      out << "," << nl << std::quoted( asiPropName_ScenePartsPersistentId ) << ": " << std::quoted( persistentId );
    }

    //! Checks is this part is equal to the passed one.
    //! \param[in] other the part to test.
    //! \return true/false.
    virtual bool isEqual(const Handle(asiAsm_SceneTree_Child)& other) const
    {
      Handle(asiAsm_SceneTree_Part) otherCasted = Handle(asiAsm_SceneTree_Part)::DownCast( other );

      return this->persistentId == otherCasted->persistentId;
    }
};

//-----------------------------------------------------------------------------

//! Provides properties of assemblies in assembly tree.
class asiAsm_SceneTree_Assembly : public asiAsm_SceneTree_Child
{
  // OpenCascade RTTI
  DEFINE_STANDARD_RTTI_INLINE( asiAsm_SceneTree_Assembly, asiAsm_SceneTree_Child )

  public:

    //! Ctor.
    asiAsm_SceneTree_Assembly()
      : asiAsm_SceneTree_Child()
    {}

  public:

    std::vector<int> children;

  protected:

    //! Fills this assembly data structure from a JSON object.
    //! \param[in] prop       the JSON property name.
    //! \param[in] pJsonBlock the JSON property value.
    virtual void fromJSON(const std::string& prop,
                          void*              pJsonBlock) override
    {
      t_jsonValue*
        pJsonObj = reinterpret_cast< t_jsonValue* >( pJsonBlock );

      // Children instances.
      if ( prop == asiPropName_SceneAssembliesAssemblyChildInstances )
      {
        t_jsonArray arr = pJsonObj->GetArray();

        asiAlgo_Utils::Json::ReadVector( &arr, children );
      }
    }

    //! Dumps this assembly data structure to JSON (the passed `out` stream).
    //! \param[in]     indent the pretty indentation shift.
    //! \param[in,out] out    the output JSON string stream.
    virtual void toJSON(const int     indent,
                        std::ostream& out) override
    {
      std::string ws(indent, ' ');
      std::string nl = "\n" + ws;

      out << "," << nl << std::quoted( asiPropName_SceneAssembliesAssemblyChildInstances ) << ": ";

      out << asiAlgo_Utils::Json::FromVector( children );
    }

    //! Checks is this assembly is equal to the passed one.
    //! \param[in] other the assembly to test.
    //! \return true/false.
    virtual bool isEqual(const Handle(asiAsm_SceneTree_Child)& other) const
    {
      Handle(asiAsm_SceneTree_Assembly) otherCasted = Handle(asiAsm_SceneTree_Assembly)::DownCast( other );

      if ( children.size() != otherCasted->children.size() )
      {
        return false;
      }

      for ( const auto& i : children )
      {
        if ( std::find( children.begin(), children.end(), i ) == children.end() )
        {
          return false;
        }
      }

      return true;
    }
};

//-----------------------------------------------------------------------------

class asiAsm_SceneTree_Instance : public asiAsm_SceneTree_Child
{
  // OpenCascade RTTI
  DEFINE_STANDARD_RTTI_INLINE( asiAsm_SceneTree_Instance, asiAsm_SceneTree_Child )

  public:

    //! Ctor.
    asiAsm_SceneTree_Instance()
      : asiAsm_SceneTree_Child(),
        prototype( -1 ),
        angle( -1. )
    {}

  public:

    int         prototype;
    std::string assemblyItemId;
    gp_XYZ      xyz;
    double      angle;
    gp_XYZ      translation;

  protected:

    //! Fills this instance data structure from a JSON object.
    //! \param[in] prop       the JSON property name.
    //! \param[in] pJsonBlock the JSON property value.
    virtual void fromJSON(const std::string& prop,
                          void*              pJsonBlock) override
    {
      t_jsonValue*
        pJsonObj = reinterpret_cast< t_jsonValue* >( pJsonBlock );

      // Prototype.
      if ( prop == asiPropName_SceneInstancesInstancePrototype )
      {
        this->prototype = pJsonObj->GetInt();
      }
    }

    //! Dumps this assembly data structure to JSON (the passed `out` stream).
    //! \param[in]     indent the pretty indentation shift.
    //! \param[in,out] out    the output JSON string stream.
    virtual void toJSON(const int     indent,
                        std::ostream& out) override
    {
      std::string ws(indent, ' ');
      std::string nl = "\n" + ws;

      out << "," << nl << std::quoted( asiPropName_SceneInstancesInstancePrototype ) << ": " << prototype;
/*
      out << "," << nl << std::quoted( asiScene_InstancesAssemblyItemId ) << ": " << std::quoted( assemblyItemId );

      out << "," << nl << std::quoted( asiScene_InstancesRotation ) << ": [ "
         << xyz.X() << ", "
         << xyz.Y() << ", "
         << xyz.Z() << ", "
         << angle   << " ]";

      out << "," << nl << std::quoted( asiScene_InstancesTranslation ) << ": [ "
         << translation.X() << ", "
         << translation.Y() << ", "
         << translation.Z() << " ]";
*/
    }

    //! Checks is this instance is equal to the passed one.
    //! \param[in] other the instance to test.
    //! \return true/false.
    virtual bool isEqual(const Handle(asiAsm_SceneTree_Child)& other) const
    {
      Handle(asiAsm_SceneTree_Instance) otherCasted = Handle(asiAsm_SceneTree_Instance)::DownCast( other );

      return prototype == otherCasted->prototype
         /* && assemblyItemId == otherCasted->assemblyItemId
            && xyz.IsEqual(otherCasted->xyz, 0.001)
            && translation.IsEqual(otherCasted->translation, 0.001)
               Abs( angle - otherCasted->angle ) < 0.001*/;
    }
};

//-----------------------------------------------------------------------------

namespace
{
  template<typename T>
  void readChildren(void*                       pJsonBlock,
                    std::vector< Handle( T ) >& v)
  {
    t_jsonArray*
      jsonBlock = reinterpret_cast< t_jsonArray* >( pJsonBlock );

    for ( t_jsonValue::ValueIterator vit = jsonBlock->Begin();
          vit != jsonBlock->End(); vit++ )
    {
      Handle( T ) value = new T;

      t_jsonValue pJsonObj = vit->GetObject();

      asiAsm_SceneTree_Child::FromJSON( &pJsonObj, value );

      v.push_back( value );
    }
  }

  //---------------------------------------------------------------------------

  template<typename T>
  void dumpChildren(const int                         indent,
                    const std::vector< Handle( T ) >& v,
                    std::ostream&                     out)
  {
    std::string comma = "";
    std::string ws(indent, ' ');
    std::string nl = "\n" + ws;
    //
    for ( auto& value : v )
    {
      out << comma << nl << "{";

      asiAsm_SceneTree_Child::ToJSON( value, indent + 2, out );

      out << nl << "}";

      if ( comma.empty() )
        comma = ",";
    }
  }
}

//-----------------------------------------------------------------------------

asiAsm_SceneTree::asiAsm_SceneTree()
{}

//-----------------------------------------------------------------------------

bool asiAsm_SceneTree::Match(const asiAsm_SceneTree& R1,
                              const asiAsm_SceneTree& R2,
                              ActAPI_ProgressEntry     progress)
{
  // Check roots.
  auto resRoots = R1.GetRoots();
  auto refRoots = R2.GetRoots();

  if ( resRoots.size() != refRoots.size() )
  {
    progress.SendLogMessage( LogErr(Normal)  << "Wrong number of roots." );
    progress.SendLogMessage( LogInfo(Normal) << "Actual size of roots: %1. Referenced size of roots: %2"
                                             << static_cast< int >( resRoots.size() )
                                             << static_cast< int >( refRoots.size() ) );

    return false;
  }

  for ( const auto& resIter : resRoots )
  {
    if ( std::find( refRoots.begin(), refRoots.end(), resIter ) == refRoots.end() )
    {
      progress.SendLogMessage( LogErr(Normal)  << "Wrong index of root - %1." << resIter );

      return false;
    }
  }

  // Check assemblies.
  auto& resAssemblies = R1.GetAssemblies();
  auto& refAssemblies = R2.GetAssemblies();

  if ( resAssemblies.size() != refAssemblies.size() )
  {
    progress.SendLogMessage( LogErr(Normal)  << "Wrong number of assemblies." );
    progress.SendLogMessage( LogInfo(Normal) << "Actual size of assemblies: %1. Referenced size of assemblies: %2"
                                             << static_cast< int >( resAssemblies.size() )
                                             << static_cast< int >( refAssemblies.size() ) );

    return false;
  }

  for ( const auto& resIter : resAssemblies )
  {
    auto comparator = [ &resIter ] ( Handle( asiAsm_SceneTree_Assembly ) assembly )
    {
      return resIter->IsEqual( assembly );
    };

    if ( std::find_if( refAssemblies.begin(), refAssemblies.end(), comparator ) == refAssemblies.end() )
    {
      progress.SendLogMessage( LogErr(Normal)  << "Wrong assembly with id -  %1." << resIter->id );

      return false;
    }
  }

  // Check instances.
  auto& resInstances = R1.GetInstances();
  auto& refInstances = R2.GetInstances();

  if ( resInstances.size() != refInstances.size() )
  {
    progress.SendLogMessage( LogErr(Normal)  << "Wrong number of instances." );
    progress.SendLogMessage( LogInfo(Normal) << "Actual size of instances: %1. Referenced size of instances: %2"
                                             << static_cast< int >( resInstances.size() )
                                             << static_cast< int >( refInstances.size() ) );

    return false;
  }

  for ( const auto& resIter : resInstances )
  {
    auto comparator = [ &resIter ] ( Handle( asiAsm_SceneTree_Instance ) instance )
    {
      return resIter->IsEqual( instance );
    };

    if ( std::find_if( refInstances.begin(), refInstances.end(), comparator ) == refInstances.end() )
    {
      progress.SendLogMessage( LogErr(Normal)  << "Wrong instance with id -  %1." << resIter->id );

      return false;
    }
  }

  // Check parts.
  auto& resParts = R1.GetParts();
  auto& refParts = R2.GetParts();

  if ( resParts.size() != refParts.size() )
  {
    progress.SendLogMessage( LogErr(Normal)  << "Wrong number of parts." );
    progress.SendLogMessage( LogInfo(Normal) << "Actual size of parts: %1. Referenced size of parts: %2"
                                             << static_cast< int >( resParts.size() )
                                             << static_cast< int >( refParts.size() ) );

    return false;
  }

  for ( const auto& resIter : resParts )
  {
    auto comparator = [ &resIter ] ( Handle( asiAsm_SceneTree_Part ) part )
    {
      return resIter->IsEqual( part );
    };

    if ( std::find_if( refParts.begin(), refParts.end(), comparator ) == refParts.end() )
    {
      progress.SendLogMessage( LogErr(Normal)  << "Wrong part with id -  %1." << resIter->id );

      return false;
    }
  }

  return true;
}

//-----------------------------------------------------------------------------

void asiAsm_SceneTree::cleanUpData()
{
  m_assemblies.clear();
  m_instances.clear();
  m_parts.clear();
  m_roots.clear();
}

//-----------------------------------------------------------------------------

void asiAsm_SceneTree::getChildInfo(const Handle(asiAsm::xde::Doc)&       doc,
                                    const Handle(asiAsm::xde::Graph)&     graph,
                                    const Handle(asiAsm_SceneTree_Child)& child,
                                    const int                             childId,
                                    const std::string&                    path)
{
  // Get id.
  child->id = childId;

  // Get name.
  TCollection_ExtendedString extName;
  doc->GetObjectName( graph->GetPersistentId( childId ), extName );

  TCollection_AsciiString name( extName );
  child->name = name.ToCString();

  // Get persistent id.
  if ( child->DynamicType() == STANDARD_TYPE( asiAsm_SceneTree_Part ) )
  {
    Handle(asiAsm_SceneTree_Part)::DownCast( child )->persistentId =
      graph->GetPersistentId( childId ).ToCString();
  }
  else if ( child->DynamicType() == STANDARD_TYPE( asiAsm_SceneTree_Instance ) )
  {
    Handle(asiAsm_SceneTree_Instance)::DownCast( child )->assemblyItemId = path;
  }
}

//-----------------------------------------------------------------------------

void asiAsm_SceneTree::Perform(const Handle(asiAsm::xde::Doc)& doc)
{
  // Clean up previously saved data.
  this->cleanUpData();

  // Extract the new one.
  Handle(asiAsm::xde::Graph) graph = new asiAsm::xde::Graph( doc );

  const TColStd_PackedMapOfInteger& rootsFromGraph = graph->GetRoots();

  for ( TColStd_MapIteratorOfPackedMapOfInteger rit( rootsFromGraph ); rit.More(); rit.Next() )
  {
    const int rootId = rit.Key();

    m_roots.push_back( rootId );

    Handle(asiAsm_SceneTree_Child) child;

    populate( doc, graph, rootId, child, graph->GetPersistentId( rootId ).ToCString() );
  }
}

//-----------------------------------------------------------------------------

void asiAsm_SceneTree::populate(const Handle(asiAsm::xde::Doc)&   doc,
                                const Handle(asiAsm::xde::Graph)& graph,
                                const int                         parentId,
                                Handle(asiAsm_SceneTree_Child)&   parent,
                                const std::string&                path)
{
  // Check parent. If it is a root, need to add it to the collection.
  // Otherwise, it had collected yet.
  if ( graph->GetRoots().Contains( parentId ) )
  {
    asiAsm::xde::Graph::NodeType nodeType = graph->GetNodeType( parentId );

    auto name = graph->GetPersistentId( parentId ).ToCString();
    //
    if ( nodeType == asiAsm::xde::Graph::NodeType_Subassembly )
    {
      Handle(asiAsm_SceneTree_Assembly) assembly = new asiAsm_SceneTree_Assembly;

      getChildInfo( doc, graph, assembly, parentId, path + "/" + name );

      m_assemblies.push_back( assembly );

      parent = assembly;
    }
    //
    if ( ( nodeType == asiAsm::xde::Graph::NodeType_PartOccurrence ) ||
         ( nodeType == asiAsm::xde::Graph::NodeType_SubassemblyOccurrence ) )
    {
      Handle(asiAsm_SceneTree_Instance) instance = new asiAsm_SceneTree_Instance;

      getChildInfo( doc, graph, instance, parentId, path + "/" + name );

      m_instances.push_back( instance );

      parent = instance;
    }
    //
    if ( nodeType == asiAsm::xde::Graph::NodeType_Part )
    {
      Handle(asiAsm_SceneTree_Part) part = new asiAsm_SceneTree_Part;

      getChildInfo( doc, graph, part, parentId, path + "/" + name );

      m_parts.push_back( part );

      parent = part;
    }
  }
  //
  if ( graph->HasChildren( parentId ) )
  {
    const TColStd_PackedMapOfInteger& children = graph->GetChildren( parentId );

    // Check children and collect them.
    for ( TColStd_MapIteratorOfPackedMapOfInteger cit( children ); cit.More(); cit.Next() )
    {
      const int childId = cit.Key();
      //
      auto comparator = [ childId ] (const Handle( asiAsm_SceneTree_Child )& c)
      {
        return c->id == childId;
      };

      if ( std::find_if( m_assemblies.begin(), m_assemblies.end(), comparator ) == m_assemblies.end() &&
           std::find_if( m_parts.begin(),      m_parts.end(),      comparator ) == m_parts.end() )
      {
        asiAsm::xde::Graph::NodeType nodeType = graph->GetNodeType( childId );

        auto name = graph->GetPersistentId( childId ).ToCString();

        // Check node type -- assembly, instance or part.
        if ( nodeType == asiAsm::xde::Graph::NodeType_Subassembly )
        {
          Handle(asiAsm_SceneTree_Assembly) assembly = new asiAsm_SceneTree_Assembly;

          getChildInfo( doc, graph, assembly, childId, path + "/" + name );

          m_assemblies.push_back( assembly );

          populate( doc, graph, childId, assembly, path + "/" + name );
        }
        //
        if ( ( nodeType == asiAsm::xde::Graph::NodeType_PartOccurrence ) ||
             ( nodeType == asiAsm::xde::Graph::NodeType_SubassemblyOccurrence ) )
        {
          Handle(asiAsm_SceneTree_Instance) instance = new asiAsm_SceneTree_Instance;

          getChildInfo( doc, graph, instance, childId, path + "/" + name );

          m_instances.push_back( instance );

          populate( doc, graph, childId, instance, path + "/" + name );
        }
        //
        if ( nodeType == asiAsm::xde::Graph::NodeType_Part )
        {
          Handle(asiAsm_SceneTree_Part) part = new asiAsm_SceneTree_Part;

          getChildInfo( doc, graph, part, childId, path + "/" + name );

          m_parts.push_back( part );

          populate( doc, graph, childId, part, path + "/" + name );
        }
      }
      // If parent is an assembly, collect its child id.
      if ( parent->DynamicType() == STANDARD_TYPE( asiAsm_SceneTree_Assembly ) )
      {
        Handle(asiAsm_SceneTree_Assembly)::DownCast( parent )->children.push_back( childId );
      }
      else if ( parent->DynamicType() == STANDARD_TYPE( asiAsm_SceneTree_Instance ) )
      {
        // If parent is an instance, collect its transformation info.
        Handle(asiAsm_SceneTree_Instance) instance = Handle(asiAsm_SceneTree_Instance)::DownCast( parent );

        instance->prototype = childId;

        Handle(asiAsm::xde::HAssemblyItemIdsMap)
          leaves = new asiAsm::xde::HAssemblyItemIdsMap;

        asiAsm::xde::AssemblyItemId id( instance->assemblyItemId.c_str() );
        doc->GetLeafAssemblyItems( id, leaves );

        for ( asiAsm::xde::HAssemblyItemIdsMap::Iterator it( *leaves ); it.More(); it.Next() )
        {
          const asiAsm::xde::AssemblyItemId& aiid = it.Value();

          TopLoc_Location T;
          TopoDS_Shape shape = doc->GetShape( aiid );
          T = T * shape.Location();

          gp_Trsf trsf = T.Transformation();

          gp_XYZ axis;
          double angle;
          trsf.GetRotation( axis, angle );

          instance->xyz = axis;
          instance->angle = angle;
          instance->translation = trsf.TranslationPart();
        }
      }
    }
  }
}

//-----------------------------------------------------------------------------

const std::vector< Handle( asiAsm_SceneTree_Assembly ) >&
  asiAsm_SceneTree::GetAssemblies() const
{
  return m_assemblies;
}

//-----------------------------------------------------------------------------

const std::vector< Handle( asiAsm_SceneTree_Instance ) >&
  asiAsm_SceneTree::GetInstances() const
{
  return m_instances;
}

//-----------------------------------------------------------------------------

const std::vector< Handle( asiAsm_SceneTree_Part ) >&
  asiAsm_SceneTree::GetParts() const
{
  return m_parts;
}

//-----------------------------------------------------------------------------

const std::vector<int>&
  asiAsm_SceneTree::GetRoots() const
{
  return m_roots;
}

//-----------------------------------------------------------------------------

void asiAsm_SceneTree::FromJSON(void*              pJsonGenericObj,
                                 asiAsm_SceneTree& info)
{
  t_jsonValue*
    pJsonObj = reinterpret_cast< t_jsonValue* >( pJsonGenericObj );

  // Iterate members of the object.
  for ( t_jsonValue::MemberIterator mit = pJsonObj->MemberBegin();
        mit != pJsonObj->MemberEnd(); mit++ )
  {
    if ( mit->value.IsNull() )
    {
      continue;
    }

    std::string prop( mit->name.GetString() );

    // Roots.
    if ( prop == asiPropName_SceneRootsIds )
    {
      t_jsonArray arr = mit->value.GetArray();

      asiAlgo_Utils::Json::ReadVector( &arr, info.m_roots );
    }

    // Prototypes.
    else if ( prop == asiPropName_ScenePrototypes )
    {
      asiAsm_SceneTree::FromJSON( &mit->value, info );
    }

    // Parts.
    else if ( prop == asiPropName_ScenePartsName )
    {
      t_jsonArray arr = mit->value.GetArray();

      readChildren< asiAsm_SceneTree_Part >( &arr, info.m_parts );
    }

    // Assemblies.
    else if ( prop == asiPropName_SceneAssembliesName )
    {
      t_jsonArray arr = mit->value.GetArray();

      readChildren< asiAsm_SceneTree_Assembly >( &arr, info.m_assemblies );
    }

    // Instances.
    else if ( prop == asiPropName_SceneInstancesName )
    {
      t_jsonArray arr = mit->value.GetArray();

      readChildren< asiAsm_SceneTree_Instance >( &arr, info.m_instances );
    }
  }
}

//-----------------------------------------------------------------------------

void asiAsm_SceneTree::ToJSON(const asiAsm_SceneTree& info,
                               const int                indent,
                               std::ostream&            out)
{
  std::string ws(indent, ' ');
  std::string nl = "\n" + ws;

  out << nl << std::quoted( asiPropName_SceneTree ) << ": {";

  // Roots.
  out << nl << "  " << std::quoted( asiPropName_SceneRootsIds ) << ": ";

  out << asiAlgo_Utils::Json::FromVector( info.GetRoots() );

  // Prototypes.
  out << "," << nl << "  " << std::quoted( asiPropName_ScenePrototypes ) << ": {";

  {
    // Parts.
    out << nl << "    " << std::quoted( asiPropName_ScenePartsName ) << ": [";
    //
    dumpChildren< asiAsm_SceneTree_Part >( indent + 6, info.GetParts(), out );

    out << nl << "    ]"; // End parts.

    // Assemblies.
    out << "," << nl << "    " << std::quoted( asiPropName_SceneAssembliesName ) << ": [";
    //
    dumpChildren< asiAsm_SceneTree_Assembly >( indent + 6, info.GetAssemblies(), out );

    out << nl << "    ]"; // End assemblies.
  }

  out << nl << "  }"; // End prototypes.

  // Instances.
  out << "," << nl << "  " << std::quoted( asiPropName_SceneInstancesName ) << ": [";

  dumpChildren< asiAsm_SceneTree_Instance >( indent + 4, info.GetInstances(), out );

  out << nl << "  ]"; // End instances.

  out << nl << "}"; // End scene tree.
}
