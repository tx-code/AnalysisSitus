//-----------------------------------------------------------------------------
// Created on: 2012-2015
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

// ACT algorithmic collection includes
#include <ActAux_ArrayUtils.h>

//! Converts raw array to a handled array.
//! \param theArray [in] raw array to convert.
//! \param theNumElems [in] number of elements in the raw array.
//! \return converted array.
Handle(HRealArray) ActAux_ArrayUtils::ToRealArray(const Standard_Real*   theArray,
                                                  const Standard_Integer theNumElems)
{
  Handle(HRealArray) aResult = new HRealArray(0, theNumElems - 1);
  for ( Standard_Integer i = 0; i < theNumElems; ++i )
    aResult->SetValue(i, theArray[i]);

  return aResult;
}

//! Converts raw array to a handled array.
//! \param theArray [in] raw array to convert.
//! \param theNumElems [in] number of elements in the raw array.
//! \param theResult [out] result array.
void ActAux_ArrayUtils::ToRealArray(const Standard_Real*   theArray,
                                    const Standard_Integer theNumElems,
                                    Handle(HRealArray)&    theResult)
{
  for ( Standard_Integer i = 0; i < theNumElems; ++i )
    theResult->SetValue(i, theArray[i]);
}

//! Converts the passed vector of real values to persistent array.
//! \param[in]  theVector vector to convert.
//! \param[out] theResult resulting array.
void ActAux_ArrayUtils::ToRealArray(const std::vector<Standard_Real>& theVector,
                                    Handle(HRealArray)&               theResult)
{
  theResult = new HRealArray(0, Standard_Integer ( theVector.size() ) - 1);
  //
  for ( int k = 0; k < theResult->Length(); ++k )
    theResult->ChangeValue(k) = theVector[k];
}

//! Converts the passed persistent array of real values to a vector.
//! \param[in]  theArray  array to convert.
//! \param[out] theResult resulting vector.
void ActAux_ArrayUtils::FromRealArray(const Handle(HRealArray)&   theArray,
                                      std::vector<Standard_Real>& theResult)
{
  for ( int k = theArray->Lower(); k <= theArray->Upper(); ++k )
    theResult.push_back( theArray->Value(k) );
}

//! Initializes the passed array with zeros.
//! \param theArr [in] array to nullify.
void ActAux_ArrayUtils::NullifyArr(const Handle(HRealArray)& theArr)
{
  ValuefyArr<Handle(HRealArray), Standard_Real>(theArr, 0.0);
}

//! Initializes the passed matrix with zeros.
//! \param theMx [in] matrix to nullify.
void ActAux_ArrayUtils::NullifyMx(const Handle(HRealMatrix)& theMx)
{
  for ( Standard_Integer i = theMx->LowerRow(); i <= theMx->UpperRow(); ++i )
    for ( Standard_Integer j = theMx->LowerCol(); j <= theMx->UpperCol(); ++j )
      theMx->SetValue(i, j, 0.0);
}

//! Converts the passed raw array to handled array.
//! \param theArr [in] plain array to convert.
//! \param theNum [in] number of elements in the plain array.
//! \return converted array.
Handle(HRealArray) ActAux_ArrayUtils::Convert(const Standard_Real* theArr,
                                              const Standard_Integer theNum)
{
  Handle(HRealArray) res = new HRealArray(0, theNum - 1);
  for ( Standard_Integer i = 0; i < theNum; ++i )
    res->SetValue(i, theArr[i]);

  return res;
}

//! Converts the passed raw array to handled array.
//! \param theArr [in] plain array to convert.
//! \param theNum [in] number of elements in the plain array.
//! \return converted array.
Handle(HComplexArray) ActAux_ArrayUtils::Convert(const ComplexNumber* theArr,
                                                 const Standard_Integer theNum)
{
  Handle(HComplexArray) res = new HComplexArray(0, theNum - 1);
  for ( Standard_Integer i = 0; i < theNum; ++i )
    res->SetValue(i, theArr[i]);

  return res;
}

//! Extracts amplitudes from the given complex array.
//! \param theArr [in] array of complex values to extract amplitudes for.
//! \return array of amplitudes.
Handle(HRealArray) ActAux_ArrayUtils::Amplitudes(const Handle(HComplexArray)& theArr)
{
  Handle(HRealArray) result = new HRealArray( theArr->Lower(), theArr->Upper() );
  for ( Standard_Integer i = theArr->Lower(); i <= theArr->Upper(); ++i )
  {
    result->ChangeValue(i) = theArr->Value(i).Amp();
  }
  return result;
}
