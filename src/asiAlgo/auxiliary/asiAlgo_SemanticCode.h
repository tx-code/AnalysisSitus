//-----------------------------------------------------------------------------
// Created on: 19 December 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Sergey Slyadnev
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

#ifndef asiAlgo_SemanticCode_HeaderFile
#define asiAlgo_SemanticCode_HeaderFile

// asiAlgo includes
#include <asiAlgo.h>

// asiAlgo includes
#include <asiAlgo_FeatureFaces.h>

// OpenCascade includes
#include <NCollection_IndexedMap.hxx>

// Standard includes
#include <vector>

//-----------------------------------------------------------------------------

//! Type of semantic code.
enum class asiAlgo_SemanticCodeType
{
  Undefined,
  Warning
};

//-----------------------------------------------------------------------------

//! Diagnostic code for reporting design issues and geometry
//! processing problems. This is normally a part body-level warning
//! code enriched with geometric semantics.
//!
//! Each code is a tuple having an ID corresponding to a group
//! of faces to report together. This way we get diagnostics
//! resolution by features and not by the codes themselves.
struct asiAlgo_SemanticCode
{
  int                      featureId; //!< Feature ID.
  int                      code;      //!< Diagnostic code.
  asiAlgo_Feature          fids;      //!< Feature faces.
  asiAlgo_SemanticCodeType type;      //!< Code type.

  //! Default ctor.
  asiAlgo_SemanticCode()
  : featureId (0),
    code      (0),
    type      (asiAlgo_SemanticCodeType::Undefined)
  {}

  //! Ctor with a feature ID, code and one face ID.
  asiAlgo_SemanticCode(const int                      _featId,
                       const int                      _code,
                       const int                      _fid,
                       const asiAlgo_SemanticCodeType _type)
  : featureId (_featId),
    code      (_code),
    type      (_type)
  {
    fids.Add(_fid);
  }

  //! Complete ctor.
  asiAlgo_SemanticCode(const int                      _featId,
                       const int                      _code,
                       const asiAlgo_Feature&         _fids,
                       const asiAlgo_SemanticCodeType _type)
  : featureId (_featId),
    code      (_code),
    fids      (_fids),
    type      (_type)
  {}

  //! Checks if the passed diagnostic code is equal to this one.
  //! This logic here is different from `Hasher::IsEqual()` as
  //! here we also take into account the stored face indices.
  //! The `Hasher::IsEqual()` rather checks the associated
  //! `featureId` properties and does not check `fids` to allow
  //! for merging the semantic codes.
  //!
  //! \param[in] other the code to check with.
  //! \return true in case of equality, false -- otherwise.
  bool Equals(const asiAlgo_SemanticCode& other) const
  {
    if ( this->code != other.code )
      return false;

    if ( !this->fids.IsEqual(other.fids) )
      return false;

    if ( this->type != other.type )
      return false;

    return true;
  }

  //! Hasher for maps.
  struct Hasher
  {
    //! Computes hash code for the passed diagnostic code.
    //! \param[in] c     the diagnostic code to compute a hash code for.
    //! \param[in] upper the upper bound to cap a hash code with.
    //! \return the computed hash code.
    static int HashCode(const asiAlgo_SemanticCode& c, const int upper)
    {
      int key = c.featureId + c.code;
      key += (key << 10);
      key ^= (key >> 6);
      key += (key << 3);
      key ^= (key >> 11);
      return (key & 0x7fffffff) % upper;
    }

    //! Checks the passed diagnostic codes for equality to be used
    //! in hash tables. This function does not compare the stored
    //! face IDs: it only compares the "primary key", which is
    //! the tuple `<featureId, code>`.
    //! \param[in] c1 the first code.
    //! \param[in] c2 the second code.
    //! \return true in case of equality, false -- otherwise.
    static bool IsEqual(const asiAlgo_SemanticCode& c1,
                        const asiAlgo_SemanticCode& c2)
    {
      if ( c1.featureId != c2.featureId )
        return false;

      if ( c1.code != c2.code )
        return false;

      if ( c1.type != c2.type )
        return false;

      return true;
    }
  };
};

//-----------------------------------------------------------------------------

//! A collection of codes with their associated feature faces. The standard
//! OpenCascade's map is subclassed to provide merge logic for codes.
class asiAlgo_SemanticCodes : public NCollection_IndexedMap<asiAlgo_SemanticCode,
                                                            asiAlgo_SemanticCode::Hasher>
{
public:

  //! Collects all warnings of the given type.
  //! \param[in]  type  the type of interest.
  //! \param[out] codes the collected codes.
  void CollectByType(const asiAlgo_SemanticCodeType     type,
                     std::vector<asiAlgo_SemanticCode>& codes) const
  {
    for ( Iterator it(*this); it.More(); it.Next() )
    {
      if ( it.Value().type == type )
        codes.push_back( it.Value() );
    }
  }

  //! Takes care to merge the collection of face IDs whenever the same code
  //! is being added several times under the same feature ID. Such merging
  //! would not happen if we used the base `NCollection_IndexedMap` collection
  //! as it is.
  //! \param[in] code the semantic code to add.
  //! \return the 1-based index of the code in the map. If such a code already
  //!         exists, it will be substituted with an extended collection of
  //!         face IDs under the same index.
  int Add(const asiAlgo_SemanticCode& code)
  {
    const int idx = this->FindIndex(code);

    if ( idx == 0 ) // Does not exist.
    {
      // Delegate to the base class.
      return NCollection_IndexedMap<asiAlgo_SemanticCode,
                                    asiAlgo_SemanticCode::Hasher>::Add(code);
    }

    // Merge face IDs.
    asiAlgo_SemanticCode existingCode = this->FindKey(idx);
    existingCode.fids.Unite(code.fids);

    // Change the existing element and return its index.
    this->Substitute(idx, existingCode);
    return idx;
  }

  //! Checks whether this collection of diagnostic codes is equal to the
  //! passed one.
  //! \param[in] other the collection to check with.
  //! \return true/false.
  bool IsEqual(const asiAlgo_SemanticCodes& other) const
  {
    if ( this->Extent() != other.Extent() )
    {
      return false;
    }

    // Compare elements.
    for ( int ii = 1; ii <= this->Extent(); ++ii )
    {
      const asiAlgo_SemanticCode& code = this->FindKey(ii);

      // We cannot use `FindIndex()` here as we should not rely on
      // the feature ID, which is not persistent. The feature IDs
      // are not stored in the reference files and are generated
      // on reading the files as serial numbers of the corresponding
      // JSON blocks.
      bool isFound = false;
      for ( int jj = 1; jj <= other.Extent(); ++jj )
      {
        if ( other.FindKey(jj).Equals(code) )
        {
          isFound = true;
          break;
        }
      }
      //
      if ( !isFound )
        return false;
    }

    return true;
  }

};

#endif
