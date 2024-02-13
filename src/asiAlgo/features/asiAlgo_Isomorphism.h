//-----------------------------------------------------------------------------
// Created on: 28 April 2020
//-----------------------------------------------------------------------------
// Copyright (c) 2016-present, Sergey Slyadnev
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

#ifndef asiAlgo_Isomorphism_h
#define asiAlgo_Isomorphism_h

// asiAlgo includes
#include <asiAlgo_AAG.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

//-----------------------------------------------------------------------------

//! \ingroup ASI_AFR
//!
//! \brief Solves subgraph isomorphism problem.
class asiAlgo_Isomorphism : public ActAPI_IAlgorithm
{
public:

  //! Modes of matching.
  enum Mode
  {
    Mode_None            = 0x00,
    Mode_MatchDimensions = 0x01, //!< `dim(P) == dim(G)` is required.
    Mode_MatchGeometry   = 0x02, //!< Features' geometric props should match.
    Mode_MatchCardinals  = 0x04  //!< Numbers of edges and verices should match.
  };

public:

  //! Default ctor.
  //! \param[in] progress progress notifier.
  //! \param[in] plotter  imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_Isomorphism(ActAPI_ProgressEntry progress = nullptr,
                        ActAPI_PlotterEntry  plotter  = nullptr);

public:

  //! Makes the algorithm to match the geometric props of the
  //! pattern faces with the sought-for features in the problem
  //! graph.
  //! \param[in] on the matching mode to set (true/false).
  asiAlgo_EXPORT void
    SetMatchGeomProps(const bool on);

  //! \return true if the geometries are requested to match.
  asiAlgo_EXPORT bool
    GetMatchGeomProps() const;

  //! Enables/disables the graphs' dimensions matching mode. Setting
  //! this option actually turns subgraph isomorphism to graph
  //! isomorphism.
  //! \param[in] on the mode to set (true/false).
  asiAlgo_EXPORT void
    SetMatchDimensions(const bool on);

  //! \return true if the graphs' dimensions are requested to match.
  asiAlgo_EXPORT bool
    GetMatchDimensions() const;

  //! Enables/disables the cardinality numbers matching mode.
  //! \param[in] on the mode to set (true/false).
  asiAlgo_EXPORT void
    SetMatchCardinals(const bool on);

  //! \return true if the cardinality numbers are requested to match.
  asiAlgo_EXPORT bool
    GetMatchCardinals() const;

public:

  //! Initializes the algorithm with the problem graph `G`.
  //! \param[in] G_aag attributed adjacency graph to set.
  asiAlgo_EXPORT void
    InitGraph(const Handle(asiAlgo_AAG)& G_aag);

  //! Solves isomorphism problem for the pattern graph `P`.
  //! \param[in] P_aag subgraph to check.
  //! \return true in case of success, false -- otherwise.
  //! \sa GetIsomorphisms() method for accessing the results.
  asiAlgo_EXPORT bool
    Perform(const Handle(asiAlgo_AAG)& P_aag);

  //! \return found isomorphisms.
  asiAlgo_EXPORT const std::vector<Eigen::MatrixXd>&
    GetIsomorphisms() const;

  //! \return found features.
  asiAlgo_EXPORT const std::vector<TColStd_PackedMapOfInteger>&
    GetFeatures() const;

  //! \return all found features in one map.
  asiAlgo_EXPORT asiAlgo_Feature
    GetAllFeatures() const;

  //! Returns the domain image (1-based face ID) of the passed `V_P`
  //! vertex of the pattern graph for the given solution matrix.
  //!
  //! \param[in] V_P the 1-based index of a vertex in the pattern graph.
  //! \param[in] M   the solution matrix.
  //! \return the image ID in the problem graoh.
  asiAlgo_EXPORT int
    GetDomainImage(const int              V_P,
                   const Eigen::MatrixXd& M) const;

  //! Returns the domain images (1-based face IDs) of the passed `V_P`
  //! vertex of the pattern graph.
  //!
  //! \param[in]  V_P    the 1-based index of a vertex in the pattern graph.
  //! \param[out] images the collected images. The passed set is not cleared.
  asiAlgo_EXPORT void
    GetDomainImages(const int                   V_P,
                    TColStd_PackedMapOfInteger& images) const;

protected:

  struct t_faceInfo
  {
    Handle(Geom_Surface) surf;
    int                  nVerts;
    int                  nEdges;
    int                  nWires;

    //! Default ctor.
    t_faceInfo() : nVerts(0), nEdges(0), nWires(0)
    {}
  };

protected:

  asiAlgo_EXPORT void
    fillFacesInfo(const Handle(asiAlgo_AAG)&                 aag,
                  NCollection_DataMap<t_topoId, t_faceInfo>& map);

  //! Initializes bijection matrix `M0`.
  //! \return the initialized matrix.
  asiAlgo_EXPORT Eigen::MatrixXd
    init_M0() const;

  //! Checks if the given vertices of graphs `P` and `G` are matching, i.e.,
  //! if we can put 1 in the corresponding element of the bijection matrix `M`.
  //!
  //! \param[in] V_P_eigenIdx zero-based index of the node in graph `P`.
  //! \param[in] V_G_eigenIdx zero-based index of the node in graph `G`.
  //! \return true/false.
  asiAlgo_EXPORT bool
    areMatching(const int V_P_eigenIdx,
                const int V_G_eigenIdx) const;

  //! Checks if the passed matrix `M` encodes some solution.
  //! Each row in the matrix `M` should contain at least one
  //! element equal to 1.
  //! \param[in] M bijection matrix to check.
  //! \return true/false.
  asiAlgo_EXPORT bool
    solutionExists(const Eigen::MatrixXd& M) const;

  //! Checks if the passed matrix `M` encodes isomorphism
  //! of the graph `P` w.r.t. any subgraph of the problem
  //! graph `G`.
  //! \param[in] M bijection matrix to check.
  //! \return true/false.
  asiAlgo_EXPORT bool
    isIsomorphism(const Eigen::MatrixXd& M);

  //! Prunes the passed bijection candidate matrix.
  //! \param[in,out] M the matrix to prune.
  asiAlgo_EXPORT void
    prune(Eigen::MatrixXd& M);

  //! Recursive routine to find isomorphisms.
  //! \param[in]     curRow   currently processed row.
  //! \param[in]     M        current state of the `M` bijection matrix.
  //! \param[in,out] usedCols used columns.
  asiAlgo_EXPORT void
    recurse(const int                   curRow,
            const Eigen::MatrixXd&      M,
            TColStd_PackedMapOfInteger& usedCols);

  //! Returns the domain image (1-based face ID) of the passed `V_P` vertex of the pattern
  //! graph w.r.t. to the given bijection specified by the `M` matrix.
  //!
  //! \param[in] V_P_eigenIdx zero-based index of a vertex in the pattern graph.
  //! \param[in] M            bijection matrix (isomorphism).
  //! \return index of the `V_P` vertex in the problem graph `G`.
  asiAlgo_EXPORT t_topoId
    getDomainImage(const int              V_P_eigenIdx,
                   const Eigen::MatrixXd& M) const;

  //! Filters out all collected isomorphisms to reduce them
  //! to the real features. This method checks the arc attributes
  //! on the found subgraphs w.r.t. the attributes of the passed
  //! pattern.
  asiAlgo_EXPORT void
    collectFeatures();

protected:

  //! Graphs in question.
  Handle(asiAlgo_AAG) m_G_aag, m_P_aag;

  //! Adjacency matrices driven by standard C++ collections.
  asiAlgo_AdjacencyMx::t_std_mx m_G_std, m_P_std;

  //! Eigen versions of the adjacency matrices.
  Eigen::MatrixXd m_G, m_P;

  //! Mappings between the domain and the Eigen versions of adjacency matrices.
  asiAlgo_AdjacencyMx::t_indexMap m_G_eigenMapping, m_P_eigenMapping;

  //! Mappings between the domain and the C++ standard versions of adjacency matrices.
  asiAlgo_AdjacencyMx::t_indexMap m_G_stdMapping, m_P_stdMapping;

  //! Face info.
  NCollection_DataMap<t_topoId, t_faceInfo> m_faceInfo_G, m_faceInfo_P;

  //! Found isomorphisms.
  std::vector<Eigen::MatrixXd> m_Ms;

  //! Found features.
  std::vector<TColStd_PackedMapOfInteger> m_features;

  //! The number of tests on isomorphism.
  int m_iNumTests;

  //! Matching modes.
  int m_iModes;

};

#endif
