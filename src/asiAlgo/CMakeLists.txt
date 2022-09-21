project(asiAlgo)

#------------------------------------------------------------------------------
# Common
#------------------------------------------------------------------------------

set (H_FILES
  asiAlgo.h
  asiAlgo_Outcome.h
  asiAlgo_Version.h
)
set (CPP_FILES
  asiAlgo_Outcome.cpp
  asiAlgo_Version.cpp
)

#------------------------------------------------------------------------------
# Auxiliary
#------------------------------------------------------------------------------

set (aux_H_FILES
  auxiliary/asiAlgo_AnalyzeWire.h
  auxiliary/asiAlgo_AttrFaceColor.h
  auxiliary/asiAlgo_BuildConvexHull.h
  auxiliary/asiAlgo_BuildHLR.h
  auxiliary/asiAlgo_BullardRNG.h
  auxiliary/asiAlgo_BVHAlgo.h
  auxiliary/asiAlgo_BVHFacets.h
  auxiliary/asiAlgo_BVHIterator.h
  auxiliary/asiAlgo_CheckClearance.h
  auxiliary/asiAlgo_CheckContour.h
  auxiliary/asiAlgo_CheckThickness.h
  auxiliary/asiAlgo_CheckToler.h
  auxiliary/asiAlgo_CheckValidity.h
  auxiliary/asiAlgo_Classifier.h
  auxiliary/asiAlgo_ClassifyPointFace.h
  auxiliary/asiAlgo_ClassifyPointSolid.h
  auxiliary/asiAlgo_Collections.h
  auxiliary/asiAlgo_CompleteEdgeLoop.h
  auxiliary/asiAlgo_ConvertCurve.h
  auxiliary/asiAlgo_CSG.h
  auxiliary/asiAlgo_DesignLaw.h
  auxiliary/asiAlgo_DivideByContinuity.h
  auxiliary/asiAlgo_FindVisibleFaces.h
  auxiliary/asiAlgo_FuncUnivariate.h
  auxiliary/asiAlgo_GeomSummary.h
  auxiliary/asiAlgo_HitFacet.h
  auxiliary/asiAlgo_IntersectBoxMesh.h
  auxiliary/asiAlgo_IntersectCC.h
  auxiliary/asiAlgo_IntersectCS.h
  auxiliary/asiAlgo_IntersectionCurveSS.h
  auxiliary/asiAlgo_IntersectionPointCC.h
  auxiliary/asiAlgo_IntersectionPointCS.h
  auxiliary/asiAlgo_IntersectSS.h
  auxiliary/asiAlgo_Membership.h
  auxiliary/asiAlgo_Optional.h
  auxiliary/asiAlgo_PointInPoly.h
  auxiliary/asiAlgo_ProjectPointOnMesh.h
  auxiliary/asiAlgo_QuickHull.h
  auxiliary/asiAlgo_ReapproxContour.h
  auxiliary/asiAlgo_ResampleADF.h
  auxiliary/asiAlgo_RTCD.h
  auxiliary/asiAlgo_SampleFace.h
  auxiliary/asiAlgo_ShapeSerializer.h
  auxiliary/asiAlgo_TopoSummary.h
  auxiliary/asiAlgo_UniformGrid.h
  auxiliary/asiAlgo_Utils.h
  auxiliary/asiAlgo_VisualSettings.h
)
set (aux_CPP_FILES
  auxiliary/asiAlgo_AnalyzeWire.cpp
  auxiliary/asiAlgo_BuildConvexHull.cpp
  auxiliary/asiAlgo_BuildHLR.cpp
  auxiliary/asiAlgo_BVHAlgo.cpp
  auxiliary/asiAlgo_BVHFacets.cpp
  auxiliary/asiAlgo_BVHIterator.cpp
  auxiliary/asiAlgo_CheckClearance.cpp
  auxiliary/asiAlgo_CheckContour.cpp
  auxiliary/asiAlgo_CheckThickness.cpp
  auxiliary/asiAlgo_CheckToler.cpp
  auxiliary/asiAlgo_CheckValidity.cpp
  auxiliary/asiAlgo_Classifier.cpp
  auxiliary/asiAlgo_ClassifyPointFace.cpp
  auxiliary/asiAlgo_CompleteEdgeLoop.cpp
  auxiliary/asiAlgo_ConvertCurve.cpp
  auxiliary/asiAlgo_DesignLaw.cpp
  auxiliary/asiAlgo_DivideByContinuity.cpp
  auxiliary/asiAlgo_FindVisibleFaces.cpp
  auxiliary/asiAlgo_FuncUnivariate.cpp
  auxiliary/asiAlgo_HitFacet.cpp
  auxiliary/asiAlgo_IntersectBoxMesh.cpp
  auxiliary/asiAlgo_IntersectCC.cpp
  auxiliary/asiAlgo_IntersectCS.cpp
  auxiliary/asiAlgo_IntersectSS.cpp
  auxiliary/asiAlgo_PointInPoly.cpp
  auxiliary/asiAlgo_ProjectPointOnMesh.cpp
  auxiliary/asiAlgo_QuickHull.cpp
  auxiliary/asiAlgo_ReapproxContour.cpp
  auxiliary/asiAlgo_ResampleADF.cpp
  auxiliary/asiAlgo_RTCD.cpp
  auxiliary/asiAlgo_SampleFace.cpp
  auxiliary/asiAlgo_ShapeSerializer.cpp
  auxiliary/asiAlgo_Utils.cpp
)

if (USE_MOBIUS)
  list(APPEND aux_H_FILES   auxiliary/asiAlgo_BoundaryDistanceField.h)
  list(APPEND aux_CPP_FILES auxiliary/asiAlgo_BoundaryDistanceField.cpp)
endif()

#------------------------------------------------------------------------------
# Blends
#------------------------------------------------------------------------------

set (blends_H_FILES
  blends/asiAlgo_AttrBlendCandidate.h
  blends/asiAlgo_AttrBlendSupport.h
  blends/asiAlgo_BlendChain.h
  blends/asiAlgo_BlendChainProps.h
  blends/asiAlgo_BlendTopoCondition.h
  blends/asiAlgo_BlendTopoConditionFF.h
  blends/asiAlgo_BlendTopoConditionFFIsolated.h
  blends/asiAlgo_BlendTopoConditionFFOrdinaryEBF.h
  blends/asiAlgo_BlendTopoConditionFFOrdinaryVBF.h
  blends/asiAlgo_BlendType.h
  blends/asiAlgo_BlendVexity.h
  blends/asiAlgo_FindCrossEdges.h
  blends/asiAlgo_FindSmoothEdges.h
  blends/asiAlgo_FindSpringEdges.h
  blends/asiAlgo_FindTermEdges.h
  blends/asiAlgo_RecognizeBlends.h
  blends/asiAlgo_RecognizeEBF.h
  blends/asiAlgo_RecognizeVBF.h
  blends/asiAlgo_SuppressBlendChain.h
  blends/asiAlgo_SuppressBlendsInc.h
)
set (blends_CPP_FILES
  blends/asiAlgo_AttrBlendCandidate.cpp
  blends/asiAlgo_BlendTopoCondition.cpp
  blends/asiAlgo_FindCrossEdges.cpp
  blends/asiAlgo_FindSmoothEdges.cpp
  blends/asiAlgo_FindSpringEdges.cpp
  blends/asiAlgo_FindTermEdges.cpp
  blends/asiAlgo_RecognizeBlends.cpp
  blends/asiAlgo_RecognizeEBF.cpp
  blends/asiAlgo_RecognizeVBF.cpp
  blends/asiAlgo_SuppressBlendChain.cpp
  blends/asiAlgo_SuppressBlendsInc.cpp
)

#------------------------------------------------------------------------------
# Canonical recognition
#------------------------------------------------------------------------------

set (canrec_H_FILES
  canrec/asiAlgo_ConvertCanonical.h
  canrec/asiAlgo_ConvertCanonicalCurve.h
  canrec/asiAlgo_ConvertCanonicalMod.h
  canrec/asiAlgo_ConvertCanonicalSummary.h
  canrec/asiAlgo_ConvertCanonicalSurface.h
  canrec/asiAlgo_RecognizeCanonical.h
)
set (canrec_CPP_FILES
  canrec/asiAlgo_ConvertCanonical.cpp
  canrec/asiAlgo_ConvertCanonicalCurve.cpp
  canrec/asiAlgo_ConvertCanonicalMod.cpp
  canrec/asiAlgo_ConvertCanonicalSummary.cpp
  canrec/asiAlgo_ConvertCanonicalSurface.cpp
  canrec/asiAlgo_RecognizeCanonical.cpp
)

#------------------------------------------------------------------------------
# Dictionary
#------------------------------------------------------------------------------

set (dict_H_FILES
  dict/asiAlgo_Dictionary.h
  dict/asiAlgo_DictionaryDimension.h
  dict/asiAlgo_DictionaryGroup.h
  dict/asiAlgo_DictionaryItem.h
  dict/asiAlgo_DictionaryUnit.h
  dict/asiAlgo_DictionaryUnitSystem.h
)
set (dict_CPP_FILES
  dict/asiAlgo_Dictionary.cpp
  dict/asiAlgo_DictionaryDimension.cpp
  dict/asiAlgo_DictionaryGroup.cpp
  dict/asiAlgo_DictionaryItem.cpp
  dict/asiAlgo_DictionaryUnit.cpp
  dict/asiAlgo_DictionaryUnitSystem.cpp
)

set (dict_salome_H_FILES
  dict/salome/DDS.h
  dict/salome/DDS_DicGroup.h
  dict/salome/DDS_DicItem.h
  dict/salome/DDS_Dictionary.h
  dict/salome/DDS_KeyWords.h
)
set (dict_salome_CPP_FILES
  dict/salome/DDS_DicGroup.cxx
  dict/salome/DDS_DicItem.cxx
  dict/salome/DDS_Dictionary.cxx
  dict/salome/DDS_KeyWords.cxx
)

#------------------------------------------------------------------------------
# Editing
#------------------------------------------------------------------------------

set (editing_H_FILES
  editing/asiAlgo_BRepNormalization.h
  editing/asiAlgo_BRepNormalizer.h
  editing/asiAlgo_DeleteEdges.h
  editing/asiAlgo_DetachFaces.h
  editing/asiAlgo_Edge2Rebuild.h
  editing/asiAlgo_Euler.h
  editing/asiAlgo_EulerKEF.h
  editing/asiAlgo_EulerKEV.h
  editing/asiAlgo_EulerKFMV.h
  editing/asiAlgo_EulerPoincare.h
  editing/asiAlgo_FindFree.h
  editing/asiAlgo_FindNonmanifold.h
  editing/asiAlgo_FindSameHosts.h
  editing/asiAlgo_FrozenVertices.h
  editing/asiAlgo_History.h
  editing/asiAlgo_InvertFaces.h
  editing/asiAlgo_InvertShells.h
  editing/asiAlgo_JoinEdges.h
  editing/asiAlgo_ModBase.h
  editing/asiAlgo_ModConstructEdge.h
  editing/asiAlgo_ModEdgeInfo.h
  editing/asiAlgo_Naming.h
  editing/asiAlgo_RebuildEdge.h
  editing/asiAlgo_RehostFaces.h
  editing/asiAlgo_RepatchFaces.h
  editing/asiAlgo_SmallEdges.h
  editing/asiAlgo_SuppressFaces.h
  editing/asiAlgo_SuppressFeatures.h
  editing/asiAlgo_SuppressHard.h
  editing/asiAlgo_SuppressSoft.h
  editing/asiAlgo_TopoKill.h
)
set (editing_CPP_FILES
  editing/asiAlgo_BRepNormalization.cpp
  editing/asiAlgo_BRepNormalizer.cpp
  editing/asiAlgo_DeleteEdges.cpp
  editing/asiAlgo_DetachFaces.cpp
  editing/asiAlgo_Euler.cpp
  editing/asiAlgo_EulerKEF.cpp
  editing/asiAlgo_EulerKEV.cpp
  editing/asiAlgo_EulerKFMV.cpp
  editing/asiAlgo_EulerPoincare.cpp
  editing/asiAlgo_FindFree.cpp
  editing/asiAlgo_FindNonmanifold.cpp
  editing/asiAlgo_FindSameHosts.cpp
  editing/asiAlgo_History.cpp
  editing/asiAlgo_InvertFaces.cpp
  editing/asiAlgo_InvertShells.cpp
  editing/asiAlgo_JoinEdges.cpp
  editing/asiAlgo_ModBase.cpp
  editing/asiAlgo_ModConstructEdge.cpp
  editing/asiAlgo_Naming.cpp
  editing/asiAlgo_RebuildEdge.cpp
  editing/asiAlgo_RepatchFaces.cpp
  editing/asiAlgo_RehostFaces.cpp
  editing/asiAlgo_SmallEdges.cpp
  editing/asiAlgo_SuppressFaces.cpp
  editing/asiAlgo_SuppressFeatures.cpp
  editing/asiAlgo_SuppressHard.cpp
  editing/asiAlgo_SuppressSoft.cpp
  editing/asiAlgo_TopoKill.cpp
)

#------------------------------------------------------------------------------
# Features
#------------------------------------------------------------------------------

set (features_H_FILES
  features/asiAlgo_AAG.h
  features/asiAlgo_AAGIterator.h
  features/asiAlgo_AdjacencyMx.h
  features/asiAlgo_BorderTrihedron.h
  features/asiAlgo_CheckDihedralAngle.h
  features/asiAlgo_CheckInsertion.h
  features/asiAlgo_CheckVertexVexity.h
  features/asiAlgo_ExtractFeatures.h
  features/asiAlgo_ExtractFeaturesResult.h
  features/asiAlgo_FeatureAngleType.h
  features/asiAlgo_FeatureAttrArea.h
  features/asiAlgo_FeatureAttrBaseFace.h
  features/asiAlgo_FeatureAttrConvexHull.h
  features/asiAlgo_FeatureAttr.h
  features/asiAlgo_FeatureAttrAdjacency.h
  features/asiAlgo_FeatureAttrAngle.h
  features/asiAlgo_FeatureAttrFace.h
  features/asiAlgo_FeatureAttrUVBounds.h
  features/asiAlgo_FeatureFaces.h
  features/asiAlgo_FeatureType.h
  features/asiAlgo_FindFeatureHints.h
  features/asiAlgo_InsertionType.h
  features/asiAlgo_Isomorphism.h
  features/asiAlgo_RecognitionRule.h
  features/asiAlgo_RecognizeCavities.h
  features/asiAlgo_RecognizeCavitiesRule.h
  features/asiAlgo_RecognizeConvexHull.h
  features/asiAlgo_RecognizeDrillHoles.h
  features/asiAlgo_RecognizeDrillHolesRule.h
  features/asiAlgo_RecognizeIsolated.h
  features/asiAlgo_Recognizer.h
  features/asiAlgo_ShapePartnerHasher.h
  features/asiAlgo_TopoAttr.h
  features/asiAlgo_TopoAttrLocation.h
  features/asiAlgo_TopoAttrName.h
  features/asiAlgo_TopoAttrOrientation.h
  features/asiAlgo_TopoGraph.h
)
set (features_CPP_FILES
  features/asiAlgo_AAG.cpp
  features/asiAlgo_AAGIterator.cpp
  features/asiAlgo_AdjacencyMx.cpp
  features/asiAlgo_CheckDihedralAngle.cpp
  features/asiAlgo_CheckInsertion.cpp
  features/asiAlgo_CheckVertexVexity.cpp
  features/asiAlgo_ExtractFeatures.cpp
  features/asiAlgo_ExtractFeaturesResult.cpp
  features/asiAlgo_FeatureFaces.cpp
  features/asiAlgo_FindFeatureHints.cpp
  features/asiAlgo_Isomorphism.cpp
  features/asiAlgo_RecognizeCavities.cpp
  features/asiAlgo_RecognizeCavitiesRule.cpp
  features/asiAlgo_RecognizeConvexHull.cpp
  features/asiAlgo_RecognizeDrillHoles.cpp
  features/asiAlgo_RecognizeDrillHolesRule.cpp
  features/asiAlgo_RecognizeIsolated.cpp
  features/asiAlgo_Recognizer.cpp
  features/asiAlgo_TopoGraph.cpp
)

#------------------------------------------------------------------------------
# Interoperability
#------------------------------------------------------------------------------

set (interop_H_FILES
  interop/asiAlgo_BaseSTEP.h
  interop/asiAlgo_FileFormat.h
  interop/asiAlgo_IGES.h
  interop/asiAlgo_InteropVars.h
  interop/asiAlgo_JsonDict.h
  interop/asiAlgo_OBJ.h
  interop/asiAlgo_PLY.h
  interop/asiAlgo_ReadSTEPWithMeta.h
  interop/asiAlgo_ReadSTEPWithMetaOutput.h
  interop/asiAlgo_STEP.h
  interop/asiAlgo_STEPReduce.h
  interop/asiAlgo_WriteDXF.h
  interop/asiAlgo_WriteREK.h
  interop/asiAlgo_WriteSTEPWithMeta.h
  interop/asiAlgo_WriteSTEPWithMetaInput.h
  interop/asiAlgo_WriteSVG.h
  interop/dxf.h
)
set (interop_CPP_FILES
  interop/asiAlgo_BaseSTEP.cpp
  interop/asiAlgo_FileFormat.cpp
  interop/asiAlgo_IGES.cpp
  interop/asiAlgo_OBJ.cpp
  interop/asiAlgo_PLY.cpp
  interop/asiAlgo_ReadSTEPWithMeta.cpp
  interop/asiAlgo_STEP.cpp
  interop/asiAlgo_STEPReduce.cpp
  interop/asiAlgo_WriteDXF.cpp
  interop/asiAlgo_WriteREK.cpp
  interop/asiAlgo_WriteSTEPWithMeta.cpp
  interop/asiAlgo_WriteSVG.cpp
  interop/dxf.cpp
  interop/dxf_plate.cpp
)

#------------------------------------------------------------------------------
# Mesh
#------------------------------------------------------------------------------

set (mesh_H_FILES
  mesh/asiAlgo_FacetQuality.h
  mesh/asiAlgo_MeshCheckTopology.h
  mesh/asiAlgo_MeshComputeNorms.h
  mesh/asiAlgo_MeshComputeShapeNorms.h
  mesh/asiAlgo_MeshConvert.h
  mesh/asiAlgo_MeshField.h
  mesh/asiAlgo_MeshGen.h
  mesh/asiAlgo_MeshInfo.h
  mesh/asiAlgo_MeshInterPlane.h
  mesh/asiAlgo_MeshLink.h
  mesh/asiAlgo_MeshMerge.h
  mesh/asiAlgo_MeshMergeNodes.h
  mesh/asiAlgo_MeshOBB.h
  mesh/asiAlgo_MeshOffset.h
  mesh/asiAlgo_MeshParams.h
  mesh/asiAlgo_MeshProjectLine.h
  mesh/asiAlgo_MeshSmooth.h
  mesh/asiAlgo_MeshWithFields.h
)
set (mesh_CPP_FILES
  mesh/asiAlgo_MeshCheckTopology.cpp
  mesh/asiAlgo_MeshComputeNorms.cpp
  mesh/asiAlgo_MeshComputeShapeNorms.cpp
  mesh/asiAlgo_MeshConvert.cpp
  mesh/asiAlgo_MeshGen.cpp
  mesh/asiAlgo_MeshInfo.cpp
  mesh/asiAlgo_MeshInterPlane.cpp
  mesh/asiAlgo_MeshMerge.cpp
  mesh/asiAlgo_MeshMergeNodes.cpp
  mesh/asiAlgo_MeshOBB.cpp
  mesh/asiAlgo_MeshOffset.cpp
  mesh/asiAlgo_MeshParams.cpp
  mesh/asiAlgo_MeshProjectLine.cpp
  mesh/asiAlgo_MeshSmooth.cpp
)

if (USE_MOBIUS)
  list(APPEND mesh_H_FILES   mesh/asiAlgo_MeshDistanceFunc.h)
  list(APPEND mesh_CPP_FILES mesh/asiAlgo_MeshDistanceFunc.cpp)
endif()

#------------------------------------------------------------------------------
# Optimization
#------------------------------------------------------------------------------

set (opt_H_FILES
  opt/asiAlgo_ArmijoRule.h
  opt/asiAlgo_Function.h
  opt/asiAlgo_FunctionWithGradient.h
  opt/asiAlgo_GradientDescent.h
  opt/asiAlgo_IneqOpt.h
  opt/asiAlgo_IneqOptParams.h
  opt/asiAlgo_IneqSystem.h
  opt/asiAlgo_PSO.h
)
set (opt_CPP_FILES
  opt/asiAlgo_ArmijoRule.cpp
  opt/asiAlgo_Function.cpp
  opt/asiAlgo_FunctionWithGradient.cpp
  opt/asiAlgo_GradientDescent.cpp
  opt/asiAlgo_IneqOpt.cpp
  opt/asiAlgo_IneqOptParams.cpp
  opt/asiAlgo_IneqSystem.cpp
  opt/asiAlgo_PSO.cpp
)

#------------------------------------------------------------------------------
# Points
#------------------------------------------------------------------------------

set (points_H_FILES
  points/asiAlgo_BaseCloud.h
  points/asiAlgo_Cloudify.h
  points/asiAlgo_CloudRegion.h
  points/asiAlgo_KHull2d.h
  points/asiAlgo_PlaneOnPoints.h
  points/asiAlgo_PlateOnPoints.h
  points/asiAlgo_PointCloudUtils.h
  points/asiAlgo_PointWithAttr.h
  points/asiAlgo_PurifyCloud.h
  points/asiAlgo_QuickHull2d.h
  points/asiAlgo_RelievePointCloud.h
  points/asiAlgo_ReorientNorms.h
)
set (points_CPP_FILES
  points/asiAlgo_BaseCloud.cpp
  points/asiAlgo_Cloudify.cpp
  points/asiAlgo_KHull2d.cpp
  points/asiAlgo_PlaneOnPoints.cpp
  points/asiAlgo_PlateOnPoints.cpp
  points/asiAlgo_PointCloudUtils.cpp
  points/asiAlgo_PointWithAttr.cpp
  points/asiAlgo_QuickHull2d.cpp
  points/asiAlgo_RelievePointCloud.cpp
  points/asiAlgo_ReorientNorms.cpp
)

#------------------------------------------------------------------------------
# Reverse engineering
#------------------------------------------------------------------------------

set (re_H_FILES
  re/asiAlgo_BuildCoonsSurf.h
  re/asiAlgo_CheckDeviations.h
  re/asiAlgo_InterpolateSurfMesh.h
  re/asiAlgo_PatchJointAdaptor.h
  re/asiAlgo_PlateOnEdges.h
)
set (re_CPP_FILES
  re/asiAlgo_BuildCoonsSurf.cpp
  re/asiAlgo_CheckDeviations.cpp
  re/asiAlgo_InterpolateSurfMesh.cpp
  re/asiAlgo_PatchJointAdaptor.cpp
  re/asiAlgo_PlateOnEdges.cpp
)

#------------------------------------------------------------------------------
# Utilities
#------------------------------------------------------------------------------

set (utils_H_FILES
  utils/asiAlgo_FileDumper.h
  utils/asiAlgo_JSON.h
  utils/asiAlgo_Logger.h
  utils/asiAlgo_MemChecker.h
  utils/asiAlgo_Timer.h
  utils/asiAlgo_ProgressNotifier.h
  utils/asiAlgo_TimeStamp.h
  utils/asiAlgo_Variable.h
  utils/MemTracker.h
)
set (utils_CPP_FILES
  utils/asiAlgo_FileDumper.cpp
  utils/asiAlgo_JSON.cpp
  utils/asiAlgo_Logger.cpp
  utils/asiAlgo_ProgressNotifier.cpp
  utils/asiAlgo_TimeStamp.cpp
  utils/MemTracker.cpp
)

#------------------------------------------------------------------------------
# Discrete B-rep
#------------------------------------------------------------------------------

set (discr_H_FILES
  discr/asiAlgo_DiscrBgrMesh.h
  discr/asiAlgo_DiscrBgrNode.h
  discr/asiAlgo_DiscrBgrTriangle.h
  discr/asiAlgo_DiscrBuilder.h
  discr/asiAlgo_DiscrClassifier2d.h
  discr/asiAlgo_DiscrCurve.h
  discr/asiAlgo_DiscrCurve2d.h
  discr/asiAlgo_DiscrCurveAdaptor.h
  discr/asiAlgo_DiscrEdge.h
  discr/asiAlgo_DiscrEdgeSeg2d.h
  discr/asiAlgo_DiscrFace.h
  discr/asiAlgo_DiscrFaceNode.h
  discr/asiAlgo_DiscrLocation.h
  discr/asiAlgo_DiscrMetrics.h
  discr/asiAlgo_DiscrModel.h
  discr/asiAlgo_DiscrParams.h
  discr/asiAlgo_DiscrPolParameter.h
  discr/asiAlgo_DiscrPolyEdgeSeg2d.h
  discr/asiAlgo_DiscrPolySeg2d.h
  discr/asiAlgo_DiscrSeg.h
  discr/asiAlgo_DiscrSeg2d.h
  discr/asiAlgo_DiscrSegAddress.h
  discr/asiAlgo_DiscrSegOnFace.h
  discr/asiAlgo_DiscrSequenceOfPointer.h
  discr/asiAlgo_DiscrTessellateCurve.h
  discr/asiAlgo_DiscrVectorOfBgrNode.h
  discr/asiAlgo_DiscrVertex.h
  discr/asiAlgo_DiscrWire.h
)
set (discr_CPP_FILES
  discr/asiAlgo_DiscrBuilder.cpp
  discr/asiAlgo_DiscrClassifier2d.cpp
  discr/asiAlgo_DiscrCurve.cpp
  discr/asiAlgo_DiscrCurve2d.cpp
  discr/asiAlgo_DiscrCurveAdaptor.cpp
  discr/asiAlgo_DiscrEdge.cpp
  discr/asiAlgo_DiscrFace.cpp
  discr/asiAlgo_DiscrFaceNode.cpp
  discr/asiAlgo_DiscrModel.cpp
  discr/asiAlgo_DiscrParams.cpp
  discr/asiAlgo_DiscrPolyEdgeSeg2d.cpp
  discr/asiAlgo_DiscrPolySeg2d.cpp
  discr/asiAlgo_DiscrTessellateCurve.cpp
  discr/asiAlgo_DiscrWire.cpp
)

#------------------------------------------------------------------------------
set (OCCT_LIB_FILES
  TKernel
  TKMath
  TKBRep
  TKOffset
  TKTopAlgo
  TKG2d
  TKG3d
  TKGeomBase
  TKGeomAlgo
  TKMesh
  TKShHealing
  TKFeat
  TKBool
  TKBO
  TKPrim
  TKBin
  TKBinL
  TKBinXCAF
  TKLCAF
  TKCDF
  TKCAF
  TKXCAF
  TKService
  TKXSBase
  TKSTEP
  TKSTEPBase
  TKSTEPAttr
  TKIGES
  TKXDESTEP
  TKXDEIGES
  TKHLR
  TKFillet
  TKSTL
  TKV3d
  TKOpenGl
)
#------------------------------------------------------------------------------
if (USE_MOBIUS)
  set (MOBIUS_LIB_FILES
    mobiusCore
    mobiusPoly
    mobiusBSpl
    mobiusCascade
    mobiusGeom
  )
endif()
#------------------------------------------------------------------------------
if (USE_NETGEN)
  set (NETGEN_LIB_FILES
    ngcore
    nglib
  )
endif()
#------------------------------------------------------------------------------
if (NOT BUILD_ALGO_ONLY)
  set (VTK_LIB_FILES
    vtkCommonCore-8.2
    vtkCommonDataModel-8.2
    vtkCommonExecutionModel-8.2
    vtkCommonMath-8.2
    vtkCommonTransforms-8.2
    vtkCommonMisc-8.2
    vtkFiltersCore-8.2
    vtkFiltersGeneral-8.2
    vtkFiltersSources-8.2
    vtkFiltersGeometry-8.2
    vtkFiltersParallel-8.2
    vtkFiltersExtraction-8.2
    vtkFiltersModeling-8.2
    vtkIOCore-8.2
    vtkIOImage-8.2
    vtkIOExport-8.2
    vtkIOXML-8.2
    vtkImagingCore-8.2
    vtkInteractionStyle-8.2
  )
endif()

#------------------------------------------------------------------------------
# Add sources
#------------------------------------------------------------------------------

foreach (FILE ${H_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Header Files" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${CPP_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Source Files" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${aux_H_FILES})
  source_group ("Header Files\\Auxiliary" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${aux_CPP_FILES})
  source_group ("Source Files\\Auxiliary" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${blends_H_FILES})
  source_group ("Header Files\\Blends" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${blends_CPP_FILES})
  source_group ("Source Files\\Blends" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${canrec_H_FILES})
  source_group ("Header Files\\Canonical" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${canrec_CPP_FILES})
  source_group ("Source Files\\Canonical" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${dict_H_FILES})
  source_group ("Header Files\\Dictionary" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${dict_CPP_FILES})
  source_group ("Source Files\\Dictionary" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${dict_salome_H_FILES})
  source_group ("Header Files\\Dictionary\\Salome" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${dict_salome_CPP_FILES})
  source_group ("Source Files\\Dictionary\\Salome" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${editing_H_FILES})
  source_group ("Header Files\\Editing" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${editing_CPP_FILES})
  source_group ("Source Files\\Editing" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${features_H_FILES})
  source_group ("Header Files\\Features" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${features_CPP_FILES})
  source_group ("Source Files\\Features" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${interop_H_FILES})
  source_group ("Header Files\\Interoperability" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${interop_CPP_FILES})
  source_group ("Source Files\\Interoperability" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${mesh_H_FILES})
  source_group ("Header Files\\Mesh" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${mesh_CPP_FILES})
  source_group ("Source Files\\Mesh" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${opt_H_FILES})
  source_group ("Header Files\\Optimization" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${opt_CPP_FILES})
  source_group ("Source Files\\Optimization" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${points_H_FILES})
  source_group ("Header Files\\Points" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${points_CPP_FILES})
  source_group ("Source Files\\Points" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${re_H_FILES})
  source_group ("Header Files\\RE" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${re_CPP_FILES})
  source_group ("Source Files\\RE" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${utils_H_FILES})
  source_group ("Header Files\\Utilities" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${utils_CPP_FILES})
  source_group ("Source Files\\Utilities" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${discr_H_FILES})
  source_group ("Header Files\\Discrete" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${discr_CPP_FILES})
  source_group ("Source Files\\Discrete" FILES "${FILE}")
endforeach (FILE)

#------------------------------------------------------------------------------
# Configure includes
#------------------------------------------------------------------------------

# Create include variable
set (asiAlgo_include_dir_loc "${CMAKE_CURRENT_SOURCE_DIR};\
  ${CMAKE_CURRENT_SOURCE_DIR}/auxiliary;\
  ${CMAKE_CURRENT_SOURCE_DIR}/blends;\
  ${CMAKE_CURRENT_SOURCE_DIR}/canrec;\
  ${CMAKE_CURRENT_SOURCE_DIR}/discr;\
  ${CMAKE_CURRENT_SOURCE_DIR}/dict;\
  ${CMAKE_CURRENT_SOURCE_DIR}/dict/salome;\
  ${CMAKE_CURRENT_SOURCE_DIR}/editing;\
  ${CMAKE_CURRENT_SOURCE_DIR}/features;\
  ${CMAKE_CURRENT_SOURCE_DIR}/interop;\
  ${CMAKE_CURRENT_SOURCE_DIR}/mesh;\
  ${CMAKE_CURRENT_SOURCE_DIR}/opt;\
  ${CMAKE_CURRENT_SOURCE_DIR}/points;\
  ${CMAKE_CURRENT_SOURCE_DIR}/re;\
  ${CMAKE_CURRENT_SOURCE_DIR}/utils;")
#
set (asiAlgo_include_dir ${asiAlgo_include_dir_loc} PARENT_SCOPE)

include_directories ( SYSTEM
                      ${asiAlgo_include_dir_loc}
                      ${asiActiveData_include_dir}
                      ${3RDPARTY_OCCT_INCLUDE_DIR}
                      ${3RDPARTY_EIGEN_DIR}
                      ${3RDPARTY_rapidjson_DIR}
                      ${3RDPARTY_vtk_INCLUDE_DIR}
                      ${3RDPARTY_tbb_INCLUDE_DIR} )

if (USE_MOBIUS)
  include_directories(SYSTEM ${3RDPARTY_mobius_INCLUDE_DIR})
endif()

if (USE_NETGEN)
  include_directories(SYSTEM ${3RDPARTY_zlib_INCLUDE_DIR})
  include_directories(SYSTEM ${3RDPARTY_netgen_INCLUDE_DIR})
  include_directories(SYSTEM ${3RDPARTY_netgen_INCLUDE_DIR}/core)
  include_directories(SYSTEM ${3RDPARTY_netgen_INCLUDE_DIR}/csg)
  include_directories(SYSTEM ${3RDPARTY_netgen_INCLUDE_DIR}/general)
  include_directories(SYSTEM ${3RDPARTY_netgen_INCLUDE_DIR}/geom2d)
  include_directories(SYSTEM ${3RDPARTY_netgen_INCLUDE_DIR}/gprim)
  include_directories(SYSTEM ${3RDPARTY_netgen_INCLUDE_DIR}/include)
  include_directories(SYSTEM ${3RDPARTY_netgen_INCLUDE_DIR}/interface)
  include_directories(SYSTEM ${3RDPARTY_netgen_INCLUDE_DIR}/linalg)
  include_directories(SYSTEM ${3RDPARTY_netgen_INCLUDE_DIR}/meshing)
  include_directories(SYSTEM ${3RDPARTY_netgen_INCLUDE_DIR}/occ)
  include_directories(SYSTEM ${3RDPARTY_netgen_INCLUDE_DIR}/stlgeom)
  include_directories(SYSTEM ${3RDPARTY_netgen_INCLUDE_DIR}/visualization)
endif()

#------------------------------------------------------------------------------
# Add target
#------------------------------------------------------------------------------

# Link with TBB (should be done BEFORE the target is added)
if (3RDPARTY_tbb_LIBRARY_DIR_DEBUG)
  link_directories(${3RDPARTY_tbb_LIBRARY_DIR_DEBUG})
else()
  link_directories(${3RDPARTY_tbb_LIBRARY_DIR})
endif()

# Add target
add_library (asiAlgo SHARED
  ${H_FILES}             ${CPP_FILES}
  ${aux_H_FILES}         ${aux_CPP_FILES}
  ${blends_H_FILES}      ${blends_CPP_FILES}
  ${canrec_H_FILES}      ${canrec_CPP_FILES}
  ${dict_H_FILES}        ${dict_CPP_FILES}
  ${dict_salome_H_FILES} ${dict_salome_CPP_FILES}
  ${editing_H_FILES}     ${editing_CPP_FILES}
  ${features_H_FILES}    ${features_CPP_FILES}
  ${interop_H_FILES}     ${interop_CPP_FILES}
  ${mesh_H_FILES}        ${mesh_CPP_FILES}
  ${opt_H_FILES}         ${opt_CPP_FILES}
  ${points_H_FILES}      ${points_CPP_FILES}
  ${re_H_FILES}          ${re_CPP_FILES}
  ${utils_H_FILES}       ${utils_CPP_FILES}
  ${discr_H_FILES}       ${discr_CPP_FILES}
)

target_link_libraries(asiAlgo asiActiveData)

#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------

foreach (LIB_FILE ${OCCT_LIB_FILES})
  if (WIN32)
    set (LIB_FILENAME "${LIB_FILE}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  else()
    set (LIB_FILENAME "lib${LIB_FILE}${CMAKE_SHARED_LIBRARY_SUFFIX}")
  endif()

  if (3RDPARTY_OCCT_LIBRARY_DIR_DEBUG AND EXISTS "${3RDPARTY_OCCT_LIBRARY_DIR_DEBUG}/${LIB_FILENAME}")
    target_link_libraries (asiAlgo debug ${3RDPARTY_OCCT_LIBRARY_DIR_DEBUG}/${LIB_FILENAME})
    target_link_libraries (asiAlgo optimized ${3RDPARTY_OCCT_LIBRARY_DIR}/${LIB_FILENAME})
  else()
    target_link_libraries (asiAlgo ${3RDPARTY_OCCT_LIBRARY_DIR}/${LIB_FILENAME})
  endif()
endforeach()

if (USE_MOBIUS)
  foreach (LIB_FILE ${MOBIUS_LIB_FILES})
    if (WIN32)
      set (LIB_FILENAME "${LIB_FILE}${CMAKE_STATIC_LIBRARY_SUFFIX}")
    else()
      set (LIB_FILENAME "lib${LIB_FILE}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    endif()

    if (3RDPARTY_mobius_LIBRARY_DIR_DEBUG AND EXISTS "${3RDPARTY_MOBIUS_LIBRARY_DIR_DEBUG}/${LIB_FILENAME}")
      target_link_libraries (asiAlgo debug ${3RDPARTY_mobius_LIBRARY_DIR_DEBUG}/${LIB_FILENAME})
      target_link_libraries (asiAlgo optimized ${3RDPARTY_mobius_LIBRARY_DIR}/${LIB_FILENAME})
    else()
      target_link_libraries (asiAlgo ${3RDPARTY_mobius_LIBRARY_DIR}/${LIB_FILENAME})
    endif()
  endforeach()
endif()

if (USE_NETGEN)
  foreach (LIB_FILE ${NETGEN_LIB_FILES})
    if (WIN32)
      set (LIB_FILENAME "${LIB_FILE}${CMAKE_STATIC_LIBRARY_SUFFIX}")
    else()
      set (LIB_FILENAME "lib${LIB_FILE}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    endif()

    if (3RDPARTY_netgen_LIBRARY_DIR_DEBUG AND EXISTS "${3RDPARTY_netgen_LIBRARY_DIR_DEBUG}/${LIB_FILENAME}")
      target_link_libraries (asiAlgo debug ${3RDPARTY_netgen_LIBRARY_DIR_DEBUG}/${LIB_FILENAME})
      target_link_libraries (asiAlgo optimized ${3RDPARTY_netgen_LIBRARY_DIR}/${LIB_FILENAME})
    else()
      target_link_libraries (asiAlgo ${3RDPARTY_netgen_LIBRARY_DIR}/${LIB_FILENAME})
    endif()
  endforeach()
endif()

if (NOT BUILD_ALGO_ONLY)
  foreach (LIB_FILE ${VTK_LIB_FILES})
    if (WIN32)
      set (LIB_FILENAME "${LIB_FILE}${CMAKE_STATIC_LIBRARY_SUFFIX}")
    else()
      set (LIB_FILENAME "lib${LIB_FILE}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    endif()

    if (3RDPARTY_vtk_LIBRARY_DIR_DEBUG AND EXISTS "${3RDPARTY_vtk_LIBRARY_DIR_DEBUG}/${LIB_FILENAME}")
      target_link_libraries (asiAlgo debug ${3RDPARTY_vtk_LIBRARY_DIR_DEBUG}/${LIB_FILENAME})
      target_link_libraries (asiAlgo optimized ${3RDPARTY_vtk_LIBRARY_DIR}/${LIB_FILENAME})
    else()
      target_link_libraries (asiAlgo ${3RDPARTY_vtk_LIBRARY_DIR}/${LIB_FILENAME})
    endif()
  endforeach()
endif()

#------------------------------------------------------------------------------
# Installation of Analysis Situs as a software
#------------------------------------------------------------------------------

if (NOT BUILD_ALGO_ONLY)
  install (TARGETS asiAlgo CONFIGURATIONS Release        RUNTIME DESTINATION bin  LIBRARY DESTINATION bin  COMPONENT Runtime)
  install (TARGETS asiAlgo CONFIGURATIONS RelWithDebInfo RUNTIME DESTINATION bini LIBRARY DESTINATION bini COMPONENT Runtime)
  install (TARGETS asiAlgo CONFIGURATIONS Debug          RUNTIME DESTINATION bind LIBRARY DESTINATION bind COMPONENT Runtime)
endif()

#------------------------------------------------------------------------------
# Installation of Analysis Situs as a framework
#------------------------------------------------------------------------------

install (TARGETS asiAlgo
         CONFIGURATIONS Release
         RUNTIME DESTINATION ${SDK_INSTALL_SUBDIR}bin COMPONENT Runtime
         ARCHIVE DESTINATION ${SDK_INSTALL_SUBDIR}lib COMPONENT Library
         LIBRARY DESTINATION ${SDK_INSTALL_SUBDIR}lib COMPONENT Library)

install (TARGETS asiAlgo
         CONFIGURATIONS RelWithDebInfo
         RUNTIME DESTINATION ${SDK_INSTALL_SUBDIR}bini COMPONENT Runtime
         ARCHIVE DESTINATION ${SDK_INSTALL_SUBDIR}libi COMPONENT Library
         LIBRARY DESTINATION ${SDK_INSTALL_SUBDIR}libi COMPONENT Library)

install (TARGETS asiAlgo
         CONFIGURATIONS Debug
         RUNTIME DESTINATION ${SDK_INSTALL_SUBDIR}bind COMPONENT Runtime
         ARCHIVE DESTINATION ${SDK_INSTALL_SUBDIR}libd COMPONENT Library
         LIBRARY DESTINATION ${SDK_INSTALL_SUBDIR}libd COMPONENT Library)

if (MSVC)
  install (FILES ${PROJECT_BINARY_DIR}/../../${PLATFORM}${COMPILER_BITNESS}/${COMPILER}/bind/asiAlgo.pdb DESTINATION ${SDK_INSTALL_SUBDIR}bind CONFIGURATIONS Debug)
endif()

install (FILES ${H_FILES}             DESTINATION ${SDK_INSTALL_SUBDIR}include)
install (FILES ${aux_H_FILES}         DESTINATION ${SDK_INSTALL_SUBDIR}include)
install (FILES ${blends_H_FILES}      DESTINATION ${SDK_INSTALL_SUBDIR}include)
install (FILES ${canrec_H_FILES}      DESTINATION ${SDK_INSTALL_SUBDIR}include)
install (FILES ${dict_H_FILES}        DESTINATION ${SDK_INSTALL_SUBDIR}include)
install (FILES ${dict_salome_H_FILES} DESTINATION ${SDK_INSTALL_SUBDIR}include)
install (FILES ${editing_H_FILES}     DESTINATION ${SDK_INSTALL_SUBDIR}include)
install (FILES ${features_H_FILES}    DESTINATION ${SDK_INSTALL_SUBDIR}include)
install (FILES ${interop_H_FILES}     DESTINATION ${SDK_INSTALL_SUBDIR}include)
install (FILES ${mesh_H_FILES}        DESTINATION ${SDK_INSTALL_SUBDIR}include)
install (FILES ${opt_H_FILES}         DESTINATION ${SDK_INSTALL_SUBDIR}include)
install (FILES ${points_H_FILES}      DESTINATION ${SDK_INSTALL_SUBDIR}include)
install (FILES ${re_H_FILES}          DESTINATION ${SDK_INSTALL_SUBDIR}include)
install (FILES ${utils_H_FILES}       DESTINATION ${SDK_INSTALL_SUBDIR}include)
install (FILES ${discr_H_FILES}       DESTINATION ${SDK_INSTALL_SUBDIR}include)
