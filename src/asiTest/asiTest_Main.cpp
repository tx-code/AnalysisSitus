//-----------------------------------------------------------------------------
// Created on: 22 June 2018
//-----------------------------------------------------------------------------
// Copyright (c) 2018-present, Sergey Slyadnev
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

// asiTest includes
#include <asiTest_AAG.h>
#include <asiTest_BuildQuickHull.h>
#include <asiTest_ChangeColor.h>
#include <asiTest_CommonFacilities.h>
#include <asiTest_ComputeNegativeVolume.h>
#include <asiTest_ConvertCanonical.h>
#include <asiTest_EdgeVexity.h>
#include <asiTest_Exchange.h>
#include <asiTest_ExchangeAstra.h>
#include <asiTest_ExchangeMesh.h>
#include <asiTest_ExchangeShape.h>
#include <asiTest_FaceGrid.h>
#include <asiTest_InvertShells.h>
#include <asiTest_IsContourClosed.h>
#include <asiTest_GenerateFacets.h>
#include <asiTest_KEV.h>
#include <asiTest_RebuildEdge.h>
#include <asiTest_RecognizeBlends.h>
#include <asiTest_RecognizeCavities.h>
#include <asiTest_RecognizeConvexHull.h>
#include <asiTest_SuppressBlends.h>
#include <asiTest_Utils.h>
#include <asiTest_VertexVexity.h>
#include <asiTest_XdeDoc.h>

// Active Data unit tests
#include <ActTest_AsciiStringParameter.h>
#include <ActTest_BaseModel.h>
#include <ActTest_BoolArrayParameter.h>
#include <ActTest_BoolParameter.h>
#include <ActTest_CAFConversionCtx.h>
#include <ActTest_ComplexArrayParameter.h>
#include <ActTest_CopyPasteEngine.h>
#include <ActTest_ExtTransactionEngine.h>
#include <ActTest_GroupParameter.h>
#include <ActTest_IntArrayParameter.h>
#include <ActTest_IntParameter.h>
#include <ActTest_MeshAttr.h>
#include <ActTest_MeshParameter.h>
#include <ActTest_NameParameter.h>
#include <ActTest_RealArrayParameter.h>
#include <ActTest_RealParameter.h>
#include <ActTest_ReferenceListParameter.h>
#include <ActTest_ReferenceParameter.h>
#include <ActTest_SelectionParameter.h>
#include <ActTest_ShapeParameter.h>
#include <ActTest_StringArrayParameter.h>
#include <ActTest_TimeStamp.h>
#include <ActTest_TimeStampParameter.h>
#include <ActTest_TreeFunctionParameter.h>
#include <ActTest_TreeNodeParameter.h>
#include <ActTest_TriangulationParameter.h>

// asiTestEngine includes
#include <asiTestEngine_Launcher.h>

// asiTcl includes
#include <asiTcl_Plugin.h>

#define PAUSE \
  system("pause");

#define RET_OK \
  PAUSE \
  return 0;

#define RET_FAILURE \
  PAUSE \
  return 1;

#undef TEST_DEBUG
#if defined TEST_DEBUG
  #pragma message("===== warning: TEST_DEBUG is enabled")
#endif

//-----------------------------------------------------------------------------

DEFINE_TEST_VARIABLES

//-----------------------------------------------------------------------------

#define TEST_LOAD_MODULE(name) \
{ \
  Handle(asiTest_CommonFacilities) cf = asiTest_CommonFacilities::Instance();\
  \
  if ( asiTcl_Plugin::Load(cf->Interp, cf, name) != asiTcl_Plugin::Status_OK ) \
    cf->Progress.SendLogMessage(LogErr(Normal) << "Cannot load %1 commands." << name); \
  else \
    cf->Progress.SendLogMessage(LogInfo(Normal) << "Loaded %1 commands." << name); \
}

//-----------------------------------------------------------------------------

int main(int argc, char* argv[])
{
  asiTest_NotUsed(argc);
  asiTest_NotUsed(argv);

  // Load commands.
  TEST_LOAD_MODULE("cmdMisc")
  TEST_LOAD_MODULE("cmdEngine")
  TEST_LOAD_MODULE("cmdAsm")
  TEST_LOAD_MODULE("cmdTest")

  // Populate launchers.
  std::cout << "asiTest : main()" << std::endl;
  std::vector< Handle(asiTestEngine_CaseLauncherAPI) > CaseLaunchers;

  // Main tests.
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<asiTest_InvertShells>          );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<asiTest_KEV>                   );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<asiTest_RebuildEdge>           );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<asiTest_RecognizeBlends>       );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<asiTest_RecognizeCavities>     );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<asiTest_RecognizeConvexHull>   );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<asiTest_SuppressBlends>        );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<asiTest_AAG>                   );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<asiTest_EdgeVexity>            );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<asiTest_VertexVexity>          );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<asiTest_FaceGrid>              );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<asiTest_IsContourClosed>       );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<asiTest_Utils>                 );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<asiTest_XdeDoc>                );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<asiTest_ConvertCanonical>      );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<asiTest_GenerateFacets>        );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<asiTest_ChangeColor>           );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<asiTest_ComputeNegativeVolume> );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<asiTest_BuildQuickHull>        );

  // Data exchange tests.
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<asiTest_Exchange>      );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<asiTest_ExchangeAstra> );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<asiTest_ExchangeMesh>  );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<asiTest_ExchangeShape> );

  // Active Data tests.
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_AsciiStringParameter>   );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_BaseModelPersistence>   );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_BaseModelStructure>     );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_BaseModelEvaluation>    );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_BoolArrayParameter>     );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_BoolParameter>          );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_ComplexArrayParameter>  );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_GroupParameter>         );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_IntArrayParameter>      );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_IntParameter>           );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_MeshParameter>          );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_NameParameter>          );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_RealArrayParameter>     );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_RealParameter>          );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_ReferenceListParameter> );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_ReferenceParameter>     );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_SelectionParameter>     );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_ShapeParameter>         );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_StringArrayParameter>   );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_TimeStampParameter>     );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_TreeFunctionParameter>  );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_TreeNodeParameter>      );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_TimeStamp>              );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_TriangulationParameter> );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_CAFConversionCtx>       );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_ExtTransactionEngine>   );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_MeshAttrTransactional>  );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_MeshAttrBean>           );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_MeshAttrPersistent>     );
  CaseLaunchers.push_back( new asiTestEngine_CaseLauncher<ActTest_CopyPasteEngine>        );

  // Launcher of entire test suite
  asiTestEngine_Launcher Launcher;
  for ( int c = 0; c < (int) CaseLaunchers.size(); ++c )
    Launcher << CaseLaunchers[c];

  PRINT_DECOR
  if ( !Launcher.Launch(&std::cout) ) // Launch test cases.
  {
    std::cout << "\t***\n\tTests FAILED" << std::endl;
    PRINT_DECOR
    return 1;
  }

  std::cout << "\t***\n\tTests SUCCEEDED" << std::endl;
  PRINT_DECOR
  return 0;
}
