//-----------------------------------------------------------------------------
// Created on: 21 September 2016
//-----------------------------------------------------------------------------
// Copyright (c) 2017, Sergey Slyadnev
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

#ifndef asiAlgo_BVHFacets_h
#define asiAlgo_BVHFacets_h

// Analysis Situs includes
#include <asiAlgo.h>

// OCCT includes
#include <BVH_Types.hxx>
#include <BVH_PrimitiveSet.hxx>
#include <NCollection_Vector.hxx>
#include <TopoDS_Face.hxx>
#include <TopTools_IndexedMapOfShape.hxx>

// STL includes
#include <vector>

// Active Data includes
#include <ActAPI_IPlotter.h>
#include <ActAPI_IProgressNotifier.h>

#if defined USE_MOBIUS
// Mobius includes
#include <mobius/poly_Mesh.h>
#endif

//-----------------------------------------------------------------------------

//! BVH-based accelerating structure representing CAD model's
//! facets in computations.
class asiAlgo_BVHFacets : public BVH_PrimitiveSet<double, 3>
{
public:

  //! Structure representing a single facet.
  struct t_facet
  {
    t_facet()               : FaceIndex(-1) {}
    t_facet(const int fidx) : FaceIndex(fidx) {}

    BVH_Vec3d P0, P1, P2; //!< Triangle nodes.
    gp_Vec    N;          //!< Cached normal calculated by nodes.
    int       FaceIndex;  //!< Index of the host face.
  };

  //! Type of BVH builder to use.
  enum BuilderType
  {
    Builder_Binned,
    Builder_Linear
  };

public:

  //! Creates the accelerating structure with immediate initialization.
  //! \param[in] model       CAD model to create the accelerating structure for.
  //! \param[in] builderType type of builder to use.
  //! \param[in] progress    progress notifier.
  //! \param[in] plotter     imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_BVHFacets(const TopoDS_Shape&  model,
                      const BuilderType    builderType = Builder_Binned,
                      ActAPI_ProgressEntry progress    = nullptr,
                      ActAPI_PlotterEntry  plotter     = nullptr);

  //! Creates the accelerating structure with immediate initialization.
  //! \param[in] mesh        triangulation to create the accelerating structure for.
  //! \param[in] builderType type of builder to use.
  //! \param[in] progress    progress notifier.
  //! \param[in] plotter     imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_BVHFacets(const Handle(Poly_Triangulation)& mesh,
                      const BuilderType                 builderType = Builder_Binned,
                      ActAPI_ProgressEntry              progress    = nullptr,
                      ActAPI_PlotterEntry               plotter     = nullptr);

#if defined USE_MOBIUS
  //! Creates the accelerating structure with immediate initialization.
  //! \param[in] mesh        triangulation to create the accelerating structure for.
  //! \param[in] builderType type of builder to use.
  //! \param[in] progress    progress notifier.
  //! \param[in] plotter     imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_BVHFacets(const mobius::t_ptr<mobius::poly_Mesh>& mesh,
                      const BuilderType                       builderType = Builder_Binned,
                      ActAPI_ProgressEntry                    progress    = nullptr,
                      ActAPI_PlotterEntry                     plotter     = nullptr);
#endif

public:

  //! \return number of stored facets.
  asiAlgo_EXPORT virtual int
    Size() const override;

  //! Builds an elementary box for a facet with the given index.
  //! \param[in] index index of the facet of interest.
  //! \return AABB for the facet of interest.
  asiAlgo_EXPORT virtual BVH_Box<double, 3>
    Box(const int index) const override;

  //! Calculates center point of a facet with respect to the axis of interest.
  //! \param[in] index index of the facet of interest.
  //! \param[in] axis  axis of interest.
  //! \return center parameter along the straight line.
  asiAlgo_EXPORT virtual double
    Center(const int index,
           const int axis) const override;

  //! Swaps two elements for BVH building.
  //! \param[in] index1 first index.
  //! \param[in] index2 second index.
  asiAlgo_EXPORT virtual void
    Swap(const int index1,
         const int index2) override;

public:

  //! Returns vertices for a facet with the given 0-based index.
  asiAlgo_EXPORT void
    GetVertices(const int  index,
                BVH_Vec3d& vertex1,
                BVH_Vec3d& vertex2,
                BVH_Vec3d& vertex3) const;

  //! \return characteristic diagonal of the full model.
  asiAlgo_EXPORT double
    GetBoundingDiag() const;

public:

  //! Dumps the BVH primitive set to the passed plotter.
  //! \param[in] IV imperative plotter to dump to.
  asiAlgo_EXPORT void
    Dump(ActAPI_PlotterEntry IV);

public:

  //! Sets the map of faces to use.
  //! \param[in] faces the map of faces to set.
  void SetMapOfFaces(const TopTools_IndexedMapOfShape& faces)
  {
    m_faces = faces;
  }

  //! \return the constructed map of faces.
  const TopTools_IndexedMapOfShape& GetMapOfFaces() const
  {
    return m_faces;
  }

  //! Returns a facet by its 0-based index.
  //! \param[in] index index of the facet of interest.
  //! \return requested facet.
  const t_facet& GetFacet(const int index)
  {
    return m_facets[index];
  }

  //! \return AABB of the entire set of objects.
  virtual BVH_Box<double, 3> Box() const
  {
    BVH_Box<double, 3> aabb;
    const int size = this->Size();

    for ( int i = 0; i < size; ++i )
    {
      aabb.Combine( this->Box(i) );
    }
    return aabb;
  }

protected:

  //! Initializes the accelerating structure with the given CAD model.
  //! \param[in] model       CAD model to prepare the accelerating structure for.
  //! \param[in] builderType type of builder to use.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    init(const TopoDS_Shape& model,
         const BuilderType   builderType);

  //! Initializes the accelerating structure with the given triangulation.
  //! \param[in] model       triangulation to prepare the accelerating structure for.
  //! \param[in] builderType type of builder to use.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    init(const Handle(Poly_Triangulation)& mesh,
         const BuilderType                 builderType);

#if defined USE_MOBIUS
  //! Initializes the accelerating structure with the given mesh.
  //! \param[in] model       mesh model to prepare the accelerating structure for.
  //! \param[in] builderType type of builder to use.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    init(const mobius::t_ptr<mobius::poly_Mesh>& mesh,
         const BuilderType                       builderType);
#endif

  //! Adds face to the accelerating structure.
  //! \param[in] face     face to add.
  //! \param[in] face_idx index of the face being added.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    addFace(const TopoDS_Face& face,
            const int          face_idx);

  //! Adds triangulation to the accelerating structure.
  //! \param[in] triangulation triangulation to add.
  //! \param[in] loc           location to apply.
  //! \param[in] face_idx      index of the corresponding face being.
  //! \param[in] isReversed    true if the original B-rep face is reversed.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    addTriangulation(const Handle(Poly_Triangulation)& triangulation,
                     const TopLoc_Location&            loc,
                     const int                         face_idx,
                     const bool                        isReversed);

protected:

  //! Map of faces constructed by the BVH builder.
  TopTools_IndexedMapOfShape m_faces;

  //! Array of facets.
  std::vector<t_facet> m_facets;

  //! Characteristic size of the model.
  double m_fBoundingDiag;

  //! Progress Entry.
  ActAPI_ProgressEntry m_progress;

  //! Imperative Plotter.
  ActAPI_PlotterEntry m_plotter;

};

#endif
