//-----------------------------------------------------------------------------
// Created on: April 2014
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

#ifndef ActAux_ArrayUtils_HeaderFile
#define ActAux_ArrayUtils_HeaderFile

// ACT algorithmic collection includes
#include <ActAux_Common.h>

// OCCT includes
#include <TCollection_AsciiString.hxx>

//! \ingroup AD_ALGO
//!
//! Auxiliary functions for working with arrays & matrices.
class ActAux_ArrayUtils
{
public:

  //! Internal class containing functionality to compare two scalar values.
  template<typename ScalarType>
  class ScalarComparator
  {
  public:

    //! Checks whether two passed scalar values are equal.
    //! \param theScalar1 [in] first scalar value.
    //! \param theScalar2 [in] second scalar value.
    //! \return true/false.
    Standard_Boolean IsEqual(const ScalarType theScalar1,
                             const ScalarType theScalar2) const
    {
      return theScalar1 == theScalar2;
    }

    //! Checks whether the first passed scalar value is less than the
    //! second passed scalar value.
    //! \param theScalar1 [in] first scalar value.
    //! \param theScalar2 [in] second scalar value.
    //! \return true/false.
    Standard_Boolean IsLess(const ScalarType theScalar1,
                            const ScalarType theScalar2) const
    {
      return theScalar1 < theScalar2;
    }

    //! Checks whether the first passed scalar value is greater than the
    //! second passed scalar value.
    //! \param theScalar1 [in] first scalar value.
    //! \param theScalar2 [in] second scalar value.
    //! \return true/false.
    Standard_Boolean IsGreater(const ScalarType theScalar1,
                               const ScalarType theScalar2) const
    {
      return theScalar1 > theScalar2;
    }
  };

public:

  ActData_EXPORT static Handle(HRealArray)
    ToRealArray(const Standard_Real*   theArray,
                const Standard_Integer theNumElems);

  ActData_EXPORT static void
    ToRealArray(const Standard_Real*   theArray,
                const Standard_Integer theNumElems,
                Handle(HRealArray)&    theResult);

  ActData_EXPORT static void
    ToRealArray(const std::vector<Standard_Real>& theVector,
                Handle(HRealArray)&               theResult);

  ActData_EXPORT static void
    FromRealArray(const Handle(HRealArray)&   theArray,
                  std::vector<Standard_Real>& theResult);

  //-------------------------------------------------------------------------//

  template <typename T>
  static void ToCoords3d(const std::vector<T>& elems,
                         Handle(HRealArray)&   coords)
  {
    if ( elems.size() == 0 )
      return;

    const Standard_Integer numElems = (Standard_Integer) elems.size();
    //
    coords = new HRealArray(0, numElems*3 - 1);
    //
    Standard_Integer idx = 0;
    for ( size_t k = 0; k < elems.size(); ++k )
    {
      const Standard_Real x = elems[k].X();
      const Standard_Real y = elems[k].Y();
      const Standard_Real z = elems[k].Z();
      //
      coords->ChangeValue(idx++) = x;
      coords->ChangeValue(idx++) = y;
      coords->ChangeValue(idx++) = z;
    }
  }

  //-------------------------------------------------------------------------//

  template <typename T>
  static void FromCoords3d(const Handle(HRealArray)& coords,
                           std::vector<T>&           elems)
  {
    if ( coords.IsNull() )
      return;

    for ( Standard_Integer idx = 0; idx < coords->Length(); idx += 3 )
    {
      T elem;
      elem.SetX( coords->Value(idx + 0) );
      elem.SetY( coords->Value(idx + 1) );
      elem.SetZ( coords->Value(idx + 2) );
      //
      elems.push_back(elem);
    }
  }

  //-------------------------------------------------------------------------//

  //! Prepares copy of the passed array.
  //! \param theArr [in] array to copy.
  //! \return new array.
  template<typename ColType, typename HColType, typename ElemType>
  static HColType Copy(const HColType& theArr)
  {
    if ( theArr.IsNull() )
      return NULL;

    // Create new collection
    Standard_Integer aLower = theArr->Lower();
    Standard_Integer anUpper = theArr->Upper();
    HColType aCopyCol = new ColType(aLower, anUpper);

    // Transfer old data to the new collection
    for ( Standard_Integer i = aLower; i <= anUpper; ++i )
      aCopyCol->SetValue( i, theArr->Value(i) );

    return aCopyCol;
  }

  //-------------------------------------------------------------------------//

  //! Appends new element to the given array. New grown array will be actually
  //! created instead.
  //! \param theArr [in] array to affect.
  //! \param theElem [in] element to add.
  //! \return new array.
  template<typename ColType, typename HColType, typename ElemType>
  static HColType Append(const HColType& theArr, const ElemType& theElem)
  {
    // Create new collection
    Standard_Integer aLower = theArr.IsNull() ? 0 : theArr->Lower();
    Standard_Integer anUpper = theArr.IsNull() ? 0 : (theArr->Upper() + 1);
    HColType aResizedCol = new ColType(aLower, anUpper);

    // Transfer old data to the new collection
    if ( !theArr.IsNull() )
      for ( Standard_Integer i = theArr->Lower(); i <= theArr->Upper(); ++i )
        aResizedCol->SetValue( i, theArr->Value(i) );

    // Add the target value to the end
    aResizedCol->SetValue(aResizedCol->Upper(), theElem);

    return aResizedCol;
  }

  //-------------------------------------------------------------------------//

  //! Appends new triple of elements to the given array. New grown array will
  //! be actually created instead.
  //! \param theArr [in] array to affect.
  //! \param theElem [in] element to add.
  //! \return new array.
  template<typename ColType, typename HColType, typename ElemType>
  static HColType Append3d(const HColType& theArr, const ElemType& theElem)
  {
    // Create new collection
    Standard_Integer aLower = theArr.IsNull() ? 0 : theArr->Lower();
    Standard_Integer anUpper = theArr.IsNull() ? 2 : (theArr->Upper() + 3);
    HColType aResizedCol = new ColType(aLower, anUpper);

    // Transfer old data to the new collection
    if ( !theArr.IsNull() )
      for ( Standard_Integer i = theArr->Lower(); i <= theArr->Upper(); ++i )
        aResizedCol->SetValue( i, theArr->Value(i) );

    // Add the target value to the end
    aResizedCol->SetValue( aResizedCol->Upper() - 2, theElem.X() );
    aResizedCol->SetValue( aResizedCol->Upper() - 1, theElem.Y() );
    aResizedCol->SetValue( aResizedCol->Upper() - 0, theElem.Z() );

    return aResizedCol;
  }

  //-------------------------------------------------------------------------//

  //! Prepend new element to the given array. New grown array will be actually
  //! created instead.
  //! \param theArr [in] array to affect.
  //! \param theElem [in] element to add.
  //! \return new array.
  template<typename ColType, typename HColType, typename ElemType>
  static HColType Prepend(const HColType& theArr, const ElemType& theElem)
  {
    // Create new collection
    Standard_Integer aLower = theArr.IsNull() ? 0 : theArr->Lower();
    Standard_Integer anUpper = theArr.IsNull() ? 0 : (theArr->Upper() + 1);
    HColType aResizedCol = new ColType(aLower, anUpper);

    // Add the target value to the end
    aResizedCol->SetValue(aResizedCol->Lower(), theElem);

    // Transfer old data to the new collection
    if ( !theArr.IsNull() )
      for ( Standard_Integer i = theArr->Lower(); i <= theArr->Upper(); ++i )
        aResizedCol->SetValue( i + 1, theArr->Value(i) );

    return aResizedCol;
  }

  //-------------------------------------------------------------------------//

  //! Removes the given element from the passed array. New array will be
  //! actually created instead.
  //! \param theArr [in] array to affect.
  //! \param theIdx [in] index of the element to remove.
  //! \return new array.
  template<typename ColType, typename HColType>
  static HColType Remove(const HColType& theArr, const Standard_Integer theIdx)
  {
    if ( theArr->Upper() == theArr->Lower() ) // Single-element array
      return NULL;

    // Create new collection
    Standard_Integer aLower = theArr.IsNull() ? 0 : theArr->Lower();
    Standard_Integer anUpper = theArr.IsNull() ? 0 : (theArr->Upper() - 1);
    HColType aResizedCol = new ColType(aLower, anUpper);

    // Transfer old data to the new collection
    if ( !theArr.IsNull() )
    {
      Standard_Integer IDX = aResizedCol->Lower();
      for ( Standard_Integer i = theArr->Lower(); i <= theArr->Upper(); ++i )
      {
        if ( i != theIdx )
          aResizedCol->SetValue( IDX++, theArr->Value(i) );
      }
    }

    return aResizedCol;
  }

  //-------------------------------------------------------------------------//

  //! Resizes the passed array to the new size.
  //! New array will be actually created instead if the
  //! size changes.
  //! \param theArr [in] array to affect.
  //! \param theFillValue [in] value to fill new elements.
  //! \param theLength [in] the new size of array.
  //! \return new array if size is changed.
  template<typename ColType, typename HColType, typename ElemType>
  static HColType Resize(const HColType& theArr, const ElemType theFillValue, const Standard_Integer theLength)
  {
    // nothing to resize actually
    if ( theArr->Length() == theLength )
      return theArr;

    Standard_Integer aLower = theArr->Lower();
    Standard_Integer anUpper = theArr->Upper();
    Standard_Integer aResUpper = aLower + theLength;

    // Nullified array for zero length case
    if ( aLower <= aResUpper )
      return NULL;

    // Create new collection
    HColType aResizedCol = new ColType(aLower, aResUpper);

    for ( Standard_Integer it = aLower; it < aResUpper; ++it )
    {
      ElemType aVal = ( it <= anUpper ) ? theArr->Value(it) : theFillValue;
      aResizedCol->SetValue(it, aVal);
    }

    return aResizedCol;
  }

  //-------------------------------------------------------------------------//

  //! Resizes the passed matrix to the new size.
  //! New matrix will be actually created instead if the
  //! size changes.
  //! \param theArr [in] array to affect.
  //! \param theFillValue [in] value to fill new elements.
  //! \param theRows [in] the number of rows.
  //! \param theCols [in] the number of columns.
  //! \return new matrix if size is changed.
  template<typename MxType, typename HMxType, typename ElemType>
  static HMxType Resize(const HMxType& theMx,
                        const ElemType theFillValue,
                        const Standard_Integer theRows,
                        const Standard_Integer theCols)
  {
    // nothing to resize actually
    if ( theMx->ColLength() == theRows
      && theMx->RowLength() == theCols )
      return theMx;

    Standard_Integer aLowerCol = theMx->LowerCol();
    Standard_Integer aUpperCol = theMx->UpperCol();

    Standard_Integer aLowerRow = theMx->LowerRow();
    Standard_Integer aUpperRow = theMx->UpperRow();
 
    Standard_Integer aResUpperRow = aLowerRow + theRows;
    Standard_Integer aResUpperCol = aLowerCol + theCols;

    // Nullified array for zero length case
    if ( aLowerCol <= aResUpperCol
      || aLowerRow <= aResUpperRow )
      return NULL;

    // Create new collection
    HMxType aResizedMx = new MxType(aLowerRow, aResUpperRow, aLowerCol, aResUpperCol);

    for ( Standard_Integer row = aLowerRow; row < aResUpperRow; ++row )
    {
      for ( Standard_Integer col = aLowerCol; col < aResUpperCol; ++col )
      {
        ElemType aVal = ( row <= aUpperRow && col <= aUpperCol ) ?
          theMx->Value(row, col) : theFillValue;

        aResizedMx->SetValue(row, col, aVal);
      }
    }

    return aResizedMx;
  }

  //-------------------------------------------------------------------------//

  //! Inserts new element after the given one in the passed array. New grown
  //! array will be actually created instead.
  //! \param theArr [in] array to affect.
  //! \param theElem [in] element to insert.
  //! \param theIdx [in] index of the existing element to insert a new one
  //!        after.
  //! \return new array.
  template<typename ColType, typename HColType, typename ElemType>
  static HColType InsertAfter(const HColType& theArr, const ElemType& theElem,
                              const Standard_Integer theIdx)
  {
    // Create new collection
    Standard_Integer aLower = theArr.IsNull() ? 0 : theArr->Lower();
    Standard_Integer anUpper = theArr.IsNull() ? 0 : (theArr->Upper() + 1);
    HColType aResizedCol = new ColType(aLower, anUpper);

    // Transfer old data to the new collection
    if ( !theArr.IsNull() )
    {
      Standard_Integer IDX = aResizedCol->Lower();
      for ( Standard_Integer i = theArr->Lower(); i <= theArr->Upper(); ++i )
      {
        aResizedCol->SetValue( IDX, theArr->Value(i) );
        if ( IDX == theIdx )
          aResizedCol->SetValue( ++IDX, theElem);
        IDX++;
      }
    }

    return aResizedCol;
  }

  //-------------------------------------------------------------------------//

  //! Performs binary search in the given array.
  //! \param theArr [in] array to search in.
  //! \param theElem [in] element to search for.
  //! \param theComparator [in] abstract comparator for array elements.
  //! \return found position or -1 if nothing was found.
  template<typename HColType, typename ElemType, typename ComparatorType>
  static Standard_Integer BinarySearch(const HColType& theArr,
                                       const ElemType& theElem,
                                       const ComparatorType& theComparator)
  {
    if ( theArr.IsNull() )
      return -1;

    Standard_Integer aRangeFirst = theArr->Lower(),
                     aRangeLast = theArr->Upper();

    return binarySearch<HColType, ElemType, ComparatorType>(aRangeFirst, aRangeLast, theArr, theElem,
                                                            theComparator);
  }

  //-------------------------------------------------------------------------//

  //! Searches for position of the given value in the passed array.
  //! \param theArr [in] source array.
  //! \param theElem [in] element to find possible position for.
  //! \param theComparator [in] abstract comparator for array elements.
  //! \return found position.
  template<typename HColType, typename ElemType, typename ComparatorType>
  static Standard_Integer BinarySearchPos(const HColType& theArr,
                                          const ElemType& theElem,
                                          const ComparatorType& theComparator)
  {
    if ( theArr.IsNull() || !theArr->Length() )
      return -1;

    Standard_Integer aRangeFirst = theArr->Lower(),
                     aRangeLast = theArr->Upper();

    return binarySearchPos<HColType, ElemType, ComparatorType>(aRangeFirst, aRangeLast,
                                                               theArr, theElem,
                                                               theComparator);
  }

  //-------------------------------------------------------------------------//

  //! Searches for position of the given value in the passed array.
  //! \param theArr [in] source array.
  //! \param theNumElems [in] number of elements in the array.
  //! \param theElem [in] element to find possible position for.
  //! \return found position.
  static Standard_Integer BinarySearchPos_Real(const Standard_Real* theArr,
                                               const Standard_Integer theNumElems,
                                               const Standard_Real theElem)
  {
    return binarySearchPos_Real(0, theNumElems-1, theArr, theElem);
  }

  //-------------------------------------------------------------------------//

  //! Populates the given array with the passed value, incrementing this
  //! value as index monotonically grows.
  //! \param theArr [in] array to populate.
  //! \param theStart [in] initial value.
  //! \param theStep [in] increment step.
  template<typename HColType, typename ElemType>
  static void ValuefyArrIncremental(const HColType& theArr,
                                    const ElemType& theStart,
                                    const ElemType& theStep)
  {
    if ( theArr.IsNull() )
      return;

    ElemType aNextVal = theStart;
    for ( Standard_Integer i = theArr->Lower(); i <= theArr->Upper(); ++i )
    {
      theArr->SetValue(i, aNextVal);
      aNextVal += theStep;
    }
  }

  //-------------------------------------------------------------------------//

  //! Populates the given array with random values lying in the specified
  //! range.
  //! \param theArr [in] array to populate.
  //! \param theMin [in] minimal value.
  //! \param theMax [in] maximal step.
  template<typename HColType, typename ElemType>
  static void ValuefyArrRandom(const HColType& theArr,
                               const ElemType& theMin,
                               const ElemType& theMax)
  {
    if ( theArr.IsNull() )
      return;
    
    srand (unsigned(time(NULL)));

    for ( Standard_Integer i = theArr->Lower(); i <= theArr->Upper(); ++i )
    {
      ElemType aRand = (ElemType) rand() / RAND_MAX;
      ElemType aVal = theMin + aRand * (theMax - theMin);
      theArr->SetValue(i, aVal);
    }
  }

  //-------------------------------------------------------------------------//

  //! Populates the given array with the passed value.
  //! \param theArr [in] array to populate.
  //! \param theVal [in] value to set in each element.
  template<typename HColType, typename ElemType>
  static void ValuefyArr(const HColType& theArr,
                         const ElemType& theVal)
  {
    if ( theArr.IsNull() )
      return;

    for ( Standard_Integer i = theArr->Lower(); i <= theArr->Upper(); ++i )
      theArr->SetValue(i, theVal);
  }

  //-------------------------------------------------------------------------//

  //! Populates the given matrix with the passed value.
  //! \param theMx [in] matrix to populate.
  //! \param theVal [in] value to set in each element.
  template<typename HColType, typename ElemType>
  static void ValuefyMx(const HColType& theMx,
                        const ElemType& theVal)
  {
    if ( theMx.IsNull() )
      return;

    for ( Standard_Integer r = theMx->LowerRow(); r <= theMx->UpperRow(); ++r )
      for ( Standard_Integer c = theMx->LowerCol(); c <= theMx->UpperCol(); ++c )
        theMx->SetValue(r, c, theVal);
  }

  //-------------------------------------------------------------------------//

  //! Populates the given matrix with random values lying in the specified
  //! range.
  //! \param theMx [in] matrix to populate.
  //! \param theMin [in] minimal value.
  //! \param theMax [in] maximal step.
  template<typename HColType, typename ElemType>
  static void ValuefyMxRandom(const HColType& theMx,
                              const ElemType& theMin,
                              const ElemType& theMax)
  {
    if ( theMx.IsNull() )
      return;

    for ( Standard_Integer r = theMx->LowerRow(); r <= theMx->UpperRow(); ++r )
    {
      for ( Standard_Integer c = theMx->LowerCol(); c <= theMx->UpperCol(); ++c )
      {
        ElemType aRand = (ElemType) rand() / RAND_MAX;
        ElemType aVal = theMin + aRand * (theMax - theMin);
        theMx->SetValue(r, c, aVal);
      }
    }
  }

  //-------------------------------------------------------------------------//

  //! Compares two arrays with exact comparison via operator==.
  //! \param theArr1 [in] first array.
  //! \param theArr2 [in] second array.
  //! \return true/false.
  template<typename HColType>
  static Standard_Boolean CompareArrays(const HColType& theArr1,
                                        const HColType& theArr2)
  {
    Standard_Integer Lower1 = theArr1->Lower(),
                     Upper1 = theArr1->Upper(),
                     Lower2 = theArr2->Lower(),
                     Upper2 = theArr2->Upper();

    if ( Lower1 != Lower2 || Upper1 != Upper2 )
      return Standard_False;

    for ( Standard_Integer i = Lower1; i <= Upper1; ++i )
    {
      if ( theArr1->Value(i) != theArr2->Value(i) )
        return Standard_False;
    }

    return Standard_True;
  }

  //-------------------------------------------------------------------------//

  //! Compares two arrays with tolerant comparison.
  //! \param theArr1 [in] first array.
  //! \param theArr2 [in] second array.
  //! \param theTol [in] tolerance to compare with.
  //! \return true/false.
  template<typename HColType, typename ElemType>
  static Standard_Boolean CompareArrays(const HColType& theArr1,
                                        const HColType& theArr2,
                                        const ElemType theTol)
  {
    Standard_Integer Lower1 = theArr1->Lower(),
                     Upper1 = theArr1->Upper(),
                     Lower2 = theArr2->Lower(),
                     Upper2 = theArr2->Upper();

    if ( Lower1 != Lower2 || Upper1 != Upper2 )
      return Standard_False;

    for ( Standard_Integer i = Lower1; i <= Upper1; ++i )
    {
      const ElemType val1 = theArr1->Value(i);
      const ElemType val2 = theArr2->Value(i);
      if ( Abs(val1 - val2) > theTol )
        return Standard_False;
    }

    return Standard_True;
  }

  //-------------------------------------------------------------------------//

  //! Removes negative values from the given array. Initial array is not
  //! actually modified.
  //! \param theArr [in] source array.
  //! \return new array.
  template<typename HColType, typename ColType, typename ElemType>
  static HColType RemoveNegativeElems(const HColType& theArr)
  {
    if ( theArr.IsNull() )
      return NULL;

    // Get number of negative elements in the passed array
    Standard_Integer aNbNegative = 0;
    for ( Standard_Integer i = theArr->Lower(); i <= theArr->Upper(); ++i )
    {
      if ( theArr->Value(i) < 0 )
        ++aNbNegative;
    }

    // Create new collection
    HColType aResult = new ColType(theArr->Lower(), theArr->Upper() - aNbNegative);

    // Populate new collection
    Standard_Integer idx = theArr->Lower();
    for ( Standard_Integer i = theArr->Lower(); i <= theArr->Upper(); ++i )
    {
      ElemType aValue = theArr->Value(i);
      if ( aValue >= 0 )
        aResult->SetValue(idx++, aValue);
    }

    return aResult;
  }

  //-------------------------------------------------------------------------//

  //! Makes array symmetrical against zero. This function will only
  //! work for positive- and negative-definite arrays.
  //! \param theArr [in] source array.
  //! \return new symmetrical array.
  static Handle(HRealArray) SymmetrizeElems(const Handle(HRealArray)& theArr)
  {
    if ( theArr.IsNull() )
      return NULL;

    // Get number of negative elements in the passed array
    Standard_Integer sign = 0;
    for ( Standard_Integer i = theArr->Lower(); i <= theArr->Upper(); ++i )
    {
      if ( sign < 0 && theArr->Value(i) > 0 )
        return NULL;

      if ( sign > 0 && theArr->Value(i) < 0 )
        return NULL;

      if ( !sign && theArr->Value(i) < 0 )
        sign = -1;

      if ( !sign && theArr->Value(i) > 0 )
        sign = 1;
    }

    Standard_Boolean hasZero = ( (sign > 0) ? (theArr->Value(0) == 0)
                                            : (theArr->Value( theArr->Upper() ) == 0) );

    // Create new collection
    Handle(HRealArray)
      aResult = hasZero ? new HRealArray(0, 2*theArr->Length() - 2)
                        : new HRealArray(0, 2*theArr->Length() - 1);

    // Middle of the resulting array
    const Standard_Integer mid = aResult->Length() / 2;

    // Populate new collection
    for ( Standard_Integer i = theArr->Lower(), idx = 0; i <= theArr->Upper(); ++i, ++idx )
    {
      if ( hasZero && i == theArr->Lower() )
      {
        aResult->ChangeValue(mid) = 0.0;
        continue;
      }

      const Standard_Real aValue = theArr->Value(i);
      aResult->ChangeValue( mid + idx )                     =  aValue;
      aResult->ChangeValue( mid - idx - (hasZero ? 0 : 1) ) = -aValue;
    }

    return aResult;
  }

  //-------------------------------------------------------------------------//

  ActData_EXPORT static void
    NullifyArr(const Handle(HRealArray)& theArr);

  ActData_EXPORT static void
    NullifyMx(const Handle(HRealMatrix)& theMx);

  ActData_EXPORT static Handle(HRealArray)
    Convert(const Standard_Real* theArr,
            const Standard_Integer theNum);

  ActData_EXPORT static Handle(HComplexArray)
    Convert(const ComplexNumber* theArr,
            const Standard_Integer theNum);

  ActData_EXPORT static Handle(HRealArray)
    Amplitudes(const Handle(HComplexArray)& theArr);

private:

  //! Kernel routine for binary search.
  //! \param theRangeF [in] first index of the actual working range.
  //! \param theRangeL [in] last index of the actual working range.
  //! \param theArr [in] source array.
  //! \param theElem [in] element to find.
  //! \param theComparator [in] abstract comparator for array elements.
  //! \return found index.
  template<typename HColType, typename ElemType, typename ComparatorType>
  static Standard_Integer binarySearch(const Standard_Integer theRangeF,
                                       const Standard_Integer theRangeL,
                                       const HColType&        theArr,
                                       const ElemType&        theElem,
                                       const ComparatorType&  theComparator)
  {
    if ( theRangeF > theRangeL )
      return -1;

    Standard_Integer aMidIdx = (theRangeF + theRangeL) * 0.5;
    ElemType aMidVal = theArr->Value(aMidIdx);

    if ( theComparator.IsEqual(aMidVal, theElem) )
      return aMidIdx;

    if ( theComparator.IsLess(aMidVal, theElem) )
      return binarySearch<HColType, ElemType, ComparatorType>(aMidIdx + 1, theRangeL,
                                                              theArr, theElem,
                                                              theComparator);

    return binarySearch<HColType, ElemType, ComparatorType>(theRangeF, aMidIdx - 1,
                                                            theArr, theElem,
                                                            theComparator);
  }

  //! Kernel routine for binary search of element's position.
  //! \param theRangeF [in] first index of the actual working range.
  //! \param theRangeL [in] last index of the actual working range.
  //! \param theArr [in] source array.
  //! \param theElem [in] element to find.
  //! \param theComparator [in] abstract comparator for array elements.
  //! \return found position index.
  template<typename HColType, typename ElemType, typename ComparatorType>
  static Standard_Integer binarySearchPos(const Standard_Integer theRangeF,
                                          const Standard_Integer theRangeL,
                                          const HColType&        theArr,
                                          const ElemType&        theElem,
                                          const ComparatorType&  theComparator)
  {
    Standard_Integer range_first = theRangeF;
    Standard_Integer range_last = theRangeL;
    Standard_Integer res = -1;

    // Divide and conquer
    while ( range_first < range_last )
    {
      Standard_Integer mid_idx = (Standard_Integer) (range_first + range_last) / 2;
      const ElemType& mid_val = theArr->Value(mid_idx);

      // Set up new bounds
      if ( theComparator.IsLess(mid_val, theElem) )
        range_first = mid_idx + 1;
      else
        range_last = mid_idx;
    }

    const ElemType& val = theArr->Value(range_first);

    if ( theComparator.IsLess(val, theElem) ) // The searched element is greater than left boundary
      res = range_first;
    else if ( theComparator.IsGreater(val, theElem) ) // The searched element is less than left boundary
      res = range_first - 1;
    else // Catched
      res = range_first;

    return res;
  }

  //! Kernel routine for binary search of element's position.
  //! \param theRangeF [in] first index of the actual working range.
  //! \param theRangeL [in] last index of the actual working range.
  //! \param theArr [in] source array.
  //! \param theElem [in] element to find.
  //! \return found position index.
  static Standard_Integer binarySearchPos_Real(const Standard_Integer theRangeF,
                                               const Standard_Integer theRangeL,
                                               const Standard_Real*   theArr,
                                               const Standard_Real    theElem)
  {
    Standard_Integer range_first = theRangeF;
    Standard_Integer range_last = theRangeL;
    Standard_Integer res = -1;

    // Divide and conquer
    while ( range_first < range_last )
    {
      Standard_Integer mid_idx = (Standard_Integer) (range_first + range_last) / 2;
      Standard_Real mid_val = theArr[mid_idx];

      // Set up new bounds
      if ( mid_val < theElem )
        range_first = mid_idx + 1;
      else
        range_last = mid_idx;
    }

    Standard_Real val = theArr[range_first];

    if ( val < theElem ) // The searched element is greater than left boundary
      res = range_first;
    else if ( val > theElem ) // The searched element is less than left boundary
      res = range_first - 1;
    else // Catched
      res = range_first;

    return res;
  }

};

#endif
