//-----------------------------------------------------------------------------
// Created on: 07 September 2022
// Author: Andrey Voevodin
//-----------------------------------------------------------------------------

#ifndef asiAlgo_ComputeNegativeVolume_h
#define asiAlgo_ComputeNegativeVolume_h

// asiAlgo includes
#include <asiAlgo.h>
#include <asiAlgo_AAG.h>
#include <asiAlgo_FeatureFaces.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

//-----------------------------------------------------------------------------

//! \ingroup ASI_MODELING
//!
//! The class for computing negative volumes of features.
class ComputeNegativeVolumeAlgo : public ActAPI_IAlgorithm
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ComputeNegativeVolumeAlgo, ActAPI_IAlgorithm)

public:

  //! Constructor.
  //! \param[in] aag      AAG.
  //! \param[in] faceIds  IDs of faces.
  //! \param[in] progress progress entry.
  //! \param[in] plotter  plotter entry.
  asiAlgo_EXPORT
    ComputeNegativeVolumeAlgo(const Handle(asiAlgo_AAG)& aag,
                              const asiAlgo_Feature&     faceIds,
                              ActAPI_ProgressEntry       progress,
                              ActAPI_PlotterEntry        plotter,
                              const bool                 isOneSolid = true)
    : ActAPI_IAlgorithm(progress, plotter),
      m_faceIDs(faceIds),
      m_aag(aag),
      m_isOneSolid(isOneSolid)
  {}

  //! Destructor.
  ~ComputeNegativeVolumeAlgo() {}

  //! Copy constructor.
  //! \param[in] algo Algo.
  asiAlgo_EXPORT
    ComputeNegativeVolumeAlgo(const ComputeNegativeVolumeAlgo& algo)
    : ActAPI_IAlgorithm(algo.m_progress, algo.m_plotter)
  {
    this->m_faceIDs         = algo.m_faceIDs;
    this->m_aag             = algo.m_aag;
    this->m_isOneSolid      = algo.m_isOneSolid;
    this->m_negativeVolumes = algo.m_negativeVolumes;
  }

  //! Assignment operator.
  //! \param[in] algo Algo.
  //! \return Algo.
  asiAlgo_EXPORT ComputeNegativeVolumeAlgo& operator=(const ComputeNegativeVolumeAlgo& algo)
  {
    ActAPI_IAlgorithm::operator=(algo);
    this->m_faceIDs         = algo.m_faceIDs;
    this->m_aag             = algo.m_aag;
    this->m_isOneSolid      = algo.m_isOneSolid;
    this->m_negativeVolumes = algo.m_negativeVolumes;
    return *this;
  }

  //! Perform.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT
    bool Perform();

  //! Gets AAG.
  //! \return AAG.
  asiAlgo_EXPORT const Handle(asiAlgo_AAG)& GetAAG() const { return m_aag; }

  //! Gets negative volumes.
  //! \return negative volumes.
  asiAlgo_EXPORT
    const std::vector<std::tuple<asiAlgo_Feature,
                                 TopoDS_Shape,
                                 double>>& GetNegativeVolumes() const { return m_negativeVolumes; }

protected:

  //! Internal perform.
  //! \return true in case of success, false -- otherwise.
  bool perform();

protected:

  asiAlgo_Feature                         m_faceIDs;         //!< FaceIds of features.
  Handle(asiAlgo_AAG)                     m_aag;             //!< AAG.
  bool                                    m_isOneSolid;      //!< Indicator showing whether there
                                                             //!  should be a "one solid - one feature"
                                                             //!  configuration.
  std::vector<std::tuple<asiAlgo_Feature,
                         TopoDS_Shape,
                         double>>         m_negativeVolumes; //!< Feature, shape of negative volume, volume.

};

#endif
