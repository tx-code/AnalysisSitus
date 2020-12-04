// dxf.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
// modified 2018 wandererfan
// modified 2020 Quaoar

// Own include
#include <dxf.h>

// asiAlgo includes
#include <asiAlgo_Version.h>

// Standard includes
#include <unordered_map>

// required by windows for M_PI definition
#define _USE_MATH_DEFINES
#include <cmath>
#include <iomanip>

using namespace std;

//-----------------------------------------------------------------------------

/*
  Quaoar: Some "plate" files were found in FreeCAD's Mod/Import/DxfPlate directory.
          To remove dependency from files, I just include their contents right
          here as string.
*/
#include "dxf_plate.cpp"

//-----------------------------------------------------------------------------

gp_Vec toVector3d(const double* a)
{
  gp_Vec result;
  result.SetX(a[0]);
  result.SetY(a[1]);
  result.SetZ(a[2]);
  return result;
}

CDxfWrite::CDxfWrite(const char*          filepath,
                     ActAPI_ProgressEntry progress,
                     ActAPI_PlotterEntry  plotter)
//
: ActAPI_IAlgorithm(progress, plotter),
//TODO: these should probably be parameters in config file
//handles:
//boilerplate 0 - A00
//used by dxf.cpp A01 - FFFE
//ACAD HANDSEED  FFFF

m_handle(0xA00),                       //room for 2560 handles in boilerplate files
//m_entityHandle(0x300),               //don't need special ranges for handles
//m_layerHandle(0x30),
//m_blockHandle(0x210),
//m_blkRecordHandle(0x110),
m_polyOverride(false),
m_layerName("none")
{
    // start the file
    m_fail        = false;
    m_version     = 12;
    m_ofs         = new ofstream(filepath, ios::out);
    m_ssBlock     = new std::ostringstream();
    m_ssBlkRecord = new std::ostringstream();
    m_ssEntity    = new std::ostringstream();
    m_ssLayer     = new std::ostringstream();

    if(!(*m_ofs)){
        m_fail = true;
        return;
    }
    m_ofs->imbue(std::locale("C"));
}

CDxfWrite::~CDxfWrite()
{
    delete m_ofs;
    delete m_ssBlock;
    delete m_ssBlkRecord;
    delete m_ssEntity;
    delete m_ssLayer;
}

void CDxfWrite::init(void)
{
    writeHeaderSection();
    makeBlockRecordTableHead();
    makeBlockSectionHead();
}

//! assemble pieces into output file
void CDxfWrite::endRun(void)
{
    makeLayerTable();
    makeBlockRecordTableBody();
    
    writeClassesSection();
    writeTablesSection();
    writeBlocksSection();
    writeEntitiesSection();
    writeObjectsSection();

    (*m_ofs) << "  0"         << endl;
    (*m_ofs) << "EOF";
}

//***************************
//writeHeaderSection
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::writeHeaderSection(void)
{
    std::stringstream ss;
    ss << ASITUS_APP_NAME << " "
       << ASITUS_VERSION_STRING ;

    //header & version
    (*m_ofs) << "999"      << endl;
    (*m_ofs) << ss.str()   << endl;

    //static header content
    ss.str("");
    ss.clear();
    ss << "header" << m_version << ".rub";
    std::string fileSpec = m_dataDir + ss.str();
    (*m_ofs) << getPlateFile(fileSpec);
}

//***************************
//writeClassesSection
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::writeClassesSection(void)
{
    if (m_version < 14) {
        return;
    }

    //static classes section content
    std::stringstream ss;
    ss << "classes" << m_version << ".rub";
    std::string fileSpec = m_dataDir + ss.str();
    (*m_ofs) << getPlateFile(fileSpec);
}

//***************************
//writeTablesSection
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::writeTablesSection(void)
{
    //static tables section head end content
    std::stringstream ss;
    ss << "tables1" << m_version << ".rub";
    std::string fileSpec = m_dataDir + ss.str();
    (*m_ofs) << getPlateFile(fileSpec);

    (*m_ofs) << (*m_ssLayer).str();

    //static tables section tail end content
    ss.str("");
    ss.clear();
    ss << "tables2" << m_version << ".rub";
    fileSpec = m_dataDir + ss.str();
    (*m_ofs) << getPlateFile(fileSpec);

    if (m_version > 12) {
        (*m_ofs) << (*m_ssBlkRecord).str();
        (*m_ofs) << "  0"      << endl;
        (*m_ofs) << "ENDTAB"   << endl;
    }
    (*m_ofs) << "  0"      << endl;
    (*m_ofs) << "ENDSEC"   << endl;
}

//***************************
//makeLayerTable
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::makeLayerTable(void)
{
    std::string tablehash = getLayerHandle();
    (*m_ssLayer) << "  0"      << endl;
    (*m_ssLayer) << "TABLE"    << endl;
    (*m_ssLayer) << "  2"      << endl;
    (*m_ssLayer) << "LAYER"    << endl;
    (*m_ssLayer) << "  5"      << endl;
    (*m_ssLayer) << tablehash  << endl;
    if (m_version > 12) {
        (*m_ssLayer) << "330"      << endl;
        (*m_ssLayer) << 0          << endl;
        (*m_ssLayer) << "100"      << endl;
        (*m_ssLayer) << "AcDbSymbolTable"   << endl;
    }
    (*m_ssLayer) << " 70"      << endl;
    (*m_ssLayer) << m_layerList.size() + 1 << endl;

    (*m_ssLayer) << "  0"      << endl;
    (*m_ssLayer) << "LAYER"    << endl;
    (*m_ssLayer) << "  5"      << endl;
    (*m_ssLayer) << getLayerHandle()  << endl;
    if (m_version > 12) {
        (*m_ssLayer) << "330"      << endl;
        (*m_ssLayer) << tablehash  << endl;
        (*m_ssLayer) << "100"      << endl;
        (*m_ssLayer) << "AcDbSymbolTableRecord"      << endl;
        (*m_ssLayer) << "100"      << endl;
        (*m_ssLayer) << "AcDbLayerTableRecord"      << endl;
    }
    (*m_ssLayer) << "  2"      << endl;
    (*m_ssLayer) << "0"        << endl;
    (*m_ssLayer) << " 70"      << endl;
    (*m_ssLayer) << "   0"     << endl;
    (*m_ssLayer) << " 62"      << endl;
    (*m_ssLayer) << "   7"     << endl;
    (*m_ssLayer) << "  6"      << endl;
    (*m_ssLayer) << "CONTINUOUS" << endl;

    for (auto& l: m_layerList) {
        (*m_ssLayer) << "  0"      << endl;
        (*m_ssLayer) << "LAYER"      << endl;
        (*m_ssLayer) << "  5"      << endl;
        (*m_ssLayer) << getLayerHandle() << endl;
        if (m_version > 12) {
            (*m_ssLayer) << "330"      << endl;
            (*m_ssLayer) << tablehash  << endl;
            (*m_ssLayer) << "100"      << endl;
            (*m_ssLayer) << "AcDbSymbolTableRecord"      << endl;
            (*m_ssLayer) << "100"      << endl;
            (*m_ssLayer) << "AcDbLayerTableRecord"      << endl;
        }
        (*m_ssLayer) << "  2"      << endl;
        (*m_ssLayer) << l << endl;
        (*m_ssLayer) << " 70"      << endl;
        (*m_ssLayer) << "    0"      << endl;
        (*m_ssLayer) << " 62"      << endl;
        (*m_ssLayer) << "    7"      << endl;
        (*m_ssLayer) << "  6"      << endl;
        (*m_ssLayer) << "CONTINUOUS"      << endl;
    }
    (*m_ssLayer) << "  0"      << endl;
    (*m_ssLayer) << "ENDTAB"   << endl;
}

//***************************
//makeBlockRecordTableHead
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::makeBlockRecordTableHead(void)
{
    if (m_version < 14) {
        return;
    }
        std::string tablehash = getBlkRecordHandle();
        m_saveBlockRecordTableHandle = tablehash;
        (*m_ssBlkRecord) << "  0"      << endl;
        (*m_ssBlkRecord) << "TABLE"      << endl;
        (*m_ssBlkRecord) << "  2"      << endl;
        (*m_ssBlkRecord) << "BLOCK_RECORD"      << endl;
        (*m_ssBlkRecord) << "  5"      << endl;
        (*m_ssBlkRecord) << tablehash  << endl;
        (*m_ssBlkRecord) << "330"      << endl;
        (*m_ssBlkRecord) << "0"        << endl;
        (*m_ssBlkRecord) << "100"      << endl;
        (*m_ssBlkRecord) << "AcDbSymbolTable"      << endl;
        (*m_ssBlkRecord) << "  70"      << endl;
        (*m_ssBlkRecord) << (m_blockList.size() + 5)   << endl;
        
        m_saveModelSpaceHandle = getBlkRecordHandle();
        (*m_ssBlkRecord) << "  0"      << endl;
        (*m_ssBlkRecord) << "BLOCK_RECORD"      << endl;
        (*m_ssBlkRecord) << "  5"      << endl;
        (*m_ssBlkRecord) << m_saveModelSpaceHandle  << endl;
        (*m_ssBlkRecord) << "330"      << endl;
        (*m_ssBlkRecord) << tablehash  << endl;
        (*m_ssBlkRecord) << "100"      << endl;
        (*m_ssBlkRecord) << "AcDbSymbolTableRecord"      << endl;
        (*m_ssBlkRecord) << "100"      << endl;
        (*m_ssBlkRecord) << "AcDbBlockTableRecord"      << endl;
        (*m_ssBlkRecord) << "  2"      << endl;
        (*m_ssBlkRecord) << "*MODEL_SPACE"   << endl;
//        (*m_ssBlkRecord) << "  1"      << endl;
//        (*m_ssBlkRecord) << " "        << endl;

        m_savePaperSpaceHandle = getBlkRecordHandle();
        (*m_ssBlkRecord) << "  0"      << endl;
        (*m_ssBlkRecord) << "BLOCK_RECORD"  << endl;
        (*m_ssBlkRecord) << "  5"      << endl;
        (*m_ssBlkRecord) << m_savePaperSpaceHandle  << endl;
        (*m_ssBlkRecord) << "330"      << endl;
        (*m_ssBlkRecord) << tablehash  << endl;
        (*m_ssBlkRecord) << "100"      << endl;
        (*m_ssBlkRecord) << "AcDbSymbolTableRecord"      << endl;
        (*m_ssBlkRecord) << "100"      << endl;
        (*m_ssBlkRecord) << "AcDbBlockTableRecord"      << endl;
        (*m_ssBlkRecord) << "  2"      << endl;
        (*m_ssBlkRecord) << "*PAPER_SPACE"   << endl;
//        (*m_ssBlkRecord) << "  1"      << endl;
//        (*m_ssBlkRecord) << " "        << endl;
}
 
//***************************
//makeBlockRecordTableBody
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::makeBlockRecordTableBody(void)
{
    if (m_version < 14) {
        return;
    }
    
    int iBlkRecord = 0;
    for (auto& b: m_blockList) {
        (*m_ssBlkRecord) << "  0"      << endl;
        (*m_ssBlkRecord) << "BLOCK_RECORD"      << endl;
        (*m_ssBlkRecord) << "  5"      << endl;
        (*m_ssBlkRecord) << m_blkRecordList.at(iBlkRecord)      << endl;
        (*m_ssBlkRecord) << "330"      << endl;
        (*m_ssBlkRecord) << m_saveBlockRecordTableHandle  << endl;
        (*m_ssBlkRecord) << "100"      << endl;
        (*m_ssBlkRecord) << "AcDbSymbolTableRecord"      << endl;
        (*m_ssBlkRecord) << "100"      << endl;
        (*m_ssBlkRecord) << "AcDbBlockTableRecord"      << endl;
        (*m_ssBlkRecord) << "  2"      << endl;
        (*m_ssBlkRecord) << b          << endl;
//        (*m_ssBlkRecord) << " 70"      << endl;
//        (*m_ssBlkRecord) << "    0"      << endl;
        iBlkRecord++;
    }
}

//***************************
//makeBlockSectionHead
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::makeBlockSectionHead(void)
{
    (*m_ssBlock) << "  0"          << endl;
    (*m_ssBlock) << "SECTION"      << endl;
    (*m_ssBlock) << "  2"          << endl;
    (*m_ssBlock) << "BLOCKS"       << endl;
    (*m_ssBlock) << "  0"          << endl;
    (*m_ssBlock) << "BLOCK"        << endl;
    (*m_ssBlock) << "  5"          << endl;
    m_currentBlock = getBlockHandle();
    (*m_ssBlock) << m_currentBlock << endl;
    if (m_version > 12) {
        (*m_ssBlock) << "330"      << endl;
        (*m_ssBlock) << m_saveModelSpaceHandle << endl;
        (*m_ssBlock) << "100"      << endl;
        (*m_ssBlock) << "AcDbEntity"      << endl;
    }
    (*m_ssBlock) << "  8"          << endl;
    (*m_ssBlock) << "0"            << endl;
    if (m_version > 12) {
        (*m_ssBlock) << "100"      << endl;
        (*m_ssBlock) << "AcDbBlockBegin"  << endl;
    }
    (*m_ssBlock) << "  2"          << endl;
    (*m_ssBlock) << "*MODEL_SPACE" << endl;
    (*m_ssBlock) << " 70"          << endl;
    (*m_ssBlock) << "   0"         << endl;
    (*m_ssBlock) << " 10"          << endl;
    (*m_ssBlock) << 0.0            << endl;
    (*m_ssBlock) << " 20"          << endl; 
    (*m_ssBlock) << 0.0            << endl;
    (*m_ssBlock) << " 30"          << endl;
    (*m_ssBlock) << 0.0            << endl;
    (*m_ssBlock) << "  3"          << endl;
    (*m_ssBlock) << "*MODEL_SPACE" << endl;
    (*m_ssBlock) << "  1"          << endl;
    (*m_ssBlock) << " "            << endl;
    (*m_ssBlock) << "  0"          << endl;
    (*m_ssBlock) << "ENDBLK"       << endl;
    (*m_ssBlock) << "  5"          << endl;
    (*m_ssBlock) << getBlockHandle()   << endl;
    if (m_version > 12) {
        (*m_ssBlock) << "330"      << endl;
        (*m_ssBlock) << m_saveModelSpaceHandle << endl;
        (*m_ssBlock) << "100"      << endl;
        (*m_ssBlock) << "AcDbEntity"  << endl;
    }
    (*m_ssBlock) << "  8"          << endl;
    (*m_ssBlock) << "0"            << endl;
    if (m_version > 12) {
        (*m_ssBlock) << "100"      << endl;
        (*m_ssBlock) << "AcDbBlockEnd"      << endl;
    }

    (*m_ssBlock) << "  0"          << endl;
    (*m_ssBlock) << "BLOCK"        << endl;
    (*m_ssBlock) << "  5"          << endl;
    m_currentBlock = getBlockHandle();
    (*m_ssBlock) << m_currentBlock << endl;
    if (m_version > 12) {
        (*m_ssBlock) << "330"      << endl;
        (*m_ssBlock) << m_savePaperSpaceHandle << endl;
        (*m_ssBlock) << "100"      << endl;
        (*m_ssBlock) << "AcDbEntity"      << endl;
        (*m_ssBlock) << " 67"          << endl;
        (*m_ssBlock) << "1"            << endl;
    }
    (*m_ssBlock) << "  8"          << endl;
    (*m_ssBlock) << "0"            << endl;
    if (m_version > 12) {
        (*m_ssBlock) << "100"      << endl;
        (*m_ssBlock) << "AcDbBlockBegin"  << endl;
    }
    (*m_ssBlock) << "  2"          << endl;
    (*m_ssBlock) << "*PAPER_SPACE" << endl;
    (*m_ssBlock) << " 70"          << endl;
    (*m_ssBlock) << "   0"         << endl;
    (*m_ssBlock) << " 10"          << endl;
    (*m_ssBlock) << 0.0            << endl;
    (*m_ssBlock) << " 20"          << endl; 
    (*m_ssBlock) << 0.0            << endl;
    (*m_ssBlock) << " 30"          << endl;
    (*m_ssBlock) << 0.0            << endl;
    (*m_ssBlock) << "  3"          << endl;
    (*m_ssBlock) << "*PAPER_SPACE" << endl;
    (*m_ssBlock) << "  1"          << endl;
    (*m_ssBlock) << " "            << endl;
    (*m_ssBlock) << "  0"          << endl;
    (*m_ssBlock) << "ENDBLK"       << endl;
    (*m_ssBlock) << "  5"          << endl;
    (*m_ssBlock) << getBlockHandle()   << endl;
    if (m_version > 12) {
        (*m_ssBlock) << "330"      << endl;
        (*m_ssBlock) << m_savePaperSpaceHandle << endl;
        (*m_ssBlock) << "100"      << endl;
        (*m_ssBlock) << "AcDbEntity"      << endl;
        (*m_ssBlock) << " 67"      << endl;      //paper_space flag
        (*m_ssBlock) << "    1"    << endl;
    }
    (*m_ssBlock) << "  8"          << endl;
    (*m_ssBlock) << "0"            << endl;
    if (m_version > 12) {
        (*m_ssBlock) << "100"      << endl;
        (*m_ssBlock) << "AcDbBlockEnd" << endl;
    }
}

std::string CDxfWrite::getPlateFile(std::string fileSpec)
{
  return __dxfBlocks[fileSpec];
}

std::string CDxfWrite::getHandle(void)
{
    m_handle++;
    std::stringstream ss;
    ss << std::uppercase << std::hex << std::setfill('0') << std::setw(2);
    ss << m_handle;
    return ss.str();
}

std::string CDxfWrite::getEntityHandle(void)
{
    return getHandle();
//    m_entityHandle++;
//    std::stringstream ss;
//    ss << std::uppercase << std::hex << std::setfill('0') << std::setw(2);
//    ss << m_entityHandle;
//    return ss.str();
}

std::string CDxfWrite::getLayerHandle(void)
{
    return getHandle();
//    m_layerHandle++;
//    std::stringstream ss;
//    ss << std::uppercase << std::hex << std::setfill('0') << std::setw(2);
//    ss << m_layerHandle;
//    return ss.str();
}

std::string CDxfWrite::getBlockHandle(void)
{
    return getHandle();
//    m_blockHandle++;
//    std::stringstream ss;
//    ss << std::uppercase << std::hex << std::setfill('0') << std::setw(2);
//    ss << m_blockHandle;
//    return ss.str();
}

std::string CDxfWrite::getBlkRecordHandle(void)
{
    return getHandle();
//    m_blkRecordHandle++;
//    std::stringstream ss;
//    ss << std::uppercase << std::hex << std::setfill('0') << std::setw(2);
//    ss << m_blkRecordHandle;
//    return ss.str();
}

void CDxfWrite::addBlockName(std::string b, std::string h) 
{
    m_blockList.push_back(b);
    m_blkRecordList.push_back(h);
}

void CDxfWrite::setLayerName(std::string s)
{
   m_layerName = s;
   m_layerList.push_back(s);
}

void CDxfWrite::writeLine(const double* s, const double* e)
{
    putLine(toVector3d(s),toVector3d(e),m_ssEntity, getEntityHandle(), m_saveModelSpaceHandle);
}

void CDxfWrite::putLine(const gp_Vec& s, const gp_Vec& e,
                        std::ostringstream* outStream, const std::string handle,
                        const std::string ownerHandle)
{
    (*outStream) << "  0"       << endl;
    (*outStream) << "LINE"      << endl;
    (*outStream) << "  5"       << endl;
    (*outStream) << handle      << endl;
    if (m_version > 12) {
        (*outStream) << "330"      << endl;
        (*outStream) << ownerHandle  << endl;
        (*outStream) << "100"      << endl;
        (*outStream) << "AcDbEntity"      << endl;
    }
    (*outStream) << "  8"       << endl;    // Group code for layer name
    (*outStream) << getLayerName()  << endl;    // Layer number
    if (m_version > 12) {
        (*outStream) << "100"      << endl;
        (*outStream) << "AcDbLine" << endl;
    }
    (*outStream) << " 10"       << endl;    // Start point of line
    (*outStream) << s.X()       << endl;    // X in WCS coordinates
    (*outStream) << " 20"       << endl;
    (*outStream) << s.Y()       << endl;    // Y in WCS coordinates
    (*outStream) << " 30"       << endl;
    (*outStream) << s.Z()       << endl;    // Z in WCS coordinates
    (*outStream) << " 11"       << endl;    // End point of line
    (*outStream) << e.X()       << endl;    // X in WCS coordinates
    (*outStream) << " 21"       << endl;
    (*outStream) << e.Y()       << endl;    // Y in WCS coordinates
    (*outStream) << " 31"       << endl;
    (*outStream) << e.Z()       << endl;    // Z in WCS coordinates
}


//***************************
//writeLWPolyLine  (Note: LWPolyline might not be supported in R12
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::writeLWPolyLine(const LWPolyDataOut &pd)
{
    (*m_ssEntity) << "  0"               << endl;
    (*m_ssEntity) << "LWPOLYLINE"     << endl;
    (*m_ssEntity) << "  5"      << endl;
    (*m_ssEntity) << getEntityHandle() << endl;
    if (m_version > 12) {
        (*m_ssEntity) << "330"      << endl;
        (*m_ssEntity) << m_saveModelSpaceHandle  << endl;
        (*m_ssEntity) << "100"      << endl;
        (*m_ssEntity) << "AcDbEntity"      << endl;
    }
    if (m_version > 12) {
        (*m_ssEntity) << "100"            << endl;    //100 groups are not part of R12
        (*m_ssEntity) << "AcDbPolyline"   << endl;
    }
    (*m_ssEntity) << "  8"            << endl;    // Group code for layer name
    (*m_ssEntity) << getLayerName()   << endl;    // Layer name
    (*m_ssEntity) << " 90"            << endl;
    (*m_ssEntity) << pd.nVert         << endl;    // number of vertices
    (*m_ssEntity) << " 70"            << endl;
    (*m_ssEntity) << pd.Flag          << endl;
    (*m_ssEntity) << " 43"            << endl;
    (*m_ssEntity) << "0"              << endl;    //Constant width opt
//    (*m_ssEntity) << pd.Width         << endl;    //Constant width opt
//    (*m_ssEntity) << " 38"            << endl;
//    (*m_ssEntity) << pd.Elev          << endl;    // Elevation
//    (*m_ssEntity) << " 39"            << endl;
//    (*m_ssEntity) << pd.Thick         << endl;    // Thickness
    for (auto& p: pd.Verts) {
        (*m_ssEntity) << " 10"        << endl;    // Vertices
        (*m_ssEntity) << p.x          << endl;
        (*m_ssEntity) << " 20"        << endl;
        (*m_ssEntity) << p.y          << endl;
    } 
    for (auto& s: pd.StartWidth) {
        (*m_ssEntity) << " 40"        << endl;
        (*m_ssEntity) << s            << endl;    // Start Width
    }
    for (auto& e: pd.EndWidth) {
        (*m_ssEntity) << " 41"        << endl;
        (*m_ssEntity) << e            << endl;    // End Width
    }
    for (auto& b: pd.Bulge) {                // Bulge
        (*m_ssEntity) << " 42"        << endl;
        (*m_ssEntity) << b            << endl;
    }
//    (*m_ssEntity) << "210"            << endl;    //Extrusion dir
//    (*m_ssEntity) << pd.Extr.x        << endl;
//    (*m_ssEntity) << "220"            << endl;
//    (*m_ssEntity) << pd.Extr.y        << endl;
//    (*m_ssEntity) << "230"            << endl;
//    (*m_ssEntity) << pd.Extr.z        << endl;
}

//***************************
//writePolyline
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::writePolyline(const LWPolyDataOut &pd)
{
    (*m_ssEntity) << "  0"            << endl;
    (*m_ssEntity) << "POLYLINE"       << endl;
    (*m_ssEntity) << "  5"      << endl;
    (*m_ssEntity) << getEntityHandle() << endl;
    if (m_version > 12) {
        (*m_ssEntity) << "330"      << endl;
        (*m_ssEntity) << m_saveModelSpaceHandle  << endl;
        (*m_ssEntity) << "100"      << endl;
        (*m_ssEntity) << "AcDbEntity"      << endl;
    }
    (*m_ssEntity) << "  8"            << endl;
    (*m_ssEntity) << getLayerName()       << endl;    // Layer name
    if (m_version > 12) {
        (*m_ssEntity) << "100"            << endl;    //100 groups are not part of R12
        (*m_ssEntity) << "AcDbPolyline"   << endl;
    }
    (*m_ssEntity) << " 66"            << endl;
    (*m_ssEntity) << "     1"         << endl;    // vertices follow
    (*m_ssEntity) << " 10"            << endl;
    (*m_ssEntity) << "0.0"            << endl;
    (*m_ssEntity) << " 20"            << endl;
    (*m_ssEntity) << "0.0"            << endl;
    (*m_ssEntity) << " 30"            << endl;
    (*m_ssEntity) << "0.0"            << endl;
    (*m_ssEntity) << " 70"            << endl;
    (*m_ssEntity) << "0"              << endl;
    for (auto& p: pd.Verts) {
        (*m_ssEntity) << "  0"        << endl;
        (*m_ssEntity) << "VERTEX"     << endl;
        (*m_ssEntity) << "  5"      << endl;
        (*m_ssEntity) << getEntityHandle() << endl;
        (*m_ssEntity) << "  8"        << endl;
        (*m_ssEntity) << getLayerName()   << endl;
        (*m_ssEntity) << " 10"        << endl;
        (*m_ssEntity) << p.x          << endl;
        (*m_ssEntity) << " 20"        << endl;
        (*m_ssEntity) << p.y          << endl;
        (*m_ssEntity) << " 30"        << endl;
        (*m_ssEntity) << "0.0"        << endl;
    } 
    (*m_ssEntity) << "  0"            << endl;
    (*m_ssEntity) << "SEQEND"         << endl;
    (*m_ssEntity) << "  5"            << endl;
    (*m_ssEntity) << getEntityHandle()      << endl;
    (*m_ssEntity) << "  8"            << endl;
    (*m_ssEntity) << getLayerName()       << endl;
}

void CDxfWrite::writePoint(const double* s)
{
    (*m_ssEntity) << "  0"            << endl;
    (*m_ssEntity) << "POINT"          << endl;
    (*m_ssEntity) << "  5"      << endl;
    (*m_ssEntity) << getEntityHandle() << endl;
    if (m_version > 12) {
        (*m_ssEntity) << "330"      << endl;
        (*m_ssEntity) << m_saveModelSpaceHandle  << endl;
        (*m_ssEntity) << "100"      << endl;
        (*m_ssEntity) << "AcDbEntity"      << endl;
    }
    (*m_ssEntity) << "  8"            << endl;    // Group code for layer name
    (*m_ssEntity) << getLayerName()       << endl;    // Layer name
    if (m_version > 12) {
        (*m_ssEntity) << "100"       << endl;
        (*m_ssEntity) << "AcDbPoint" << endl;
    }
    (*m_ssEntity) << " 10"            << endl;
    (*m_ssEntity) << s[0]             << endl;    // X in WCS coordinates
    (*m_ssEntity) << " 20"            << endl;
    (*m_ssEntity) << s[1]             << endl;    // Y in WCS coordinates
    (*m_ssEntity) << " 30"            << endl;
    (*m_ssEntity) << s[2]             << endl;    // Z in WCS coordinates
}

void CDxfWrite::writeArc(const double* s, const double* e, const double* c, bool dir)
{
    double ax = s[0] - c[0];
    double ay = s[1] - c[1];
    double bx = e[0] - c[0];
    double by = e[1] - c[1];

    double start_angle = atan2(ay, ax) * 180/M_PI;
    double end_angle = atan2(by, bx) * 180/M_PI;
    double radius = sqrt(ax*ax + ay*ay);
    if(!dir){
        double temp = start_angle;
        start_angle = end_angle;
        end_angle = temp;
    }
    (*m_ssEntity) << "  0"       << endl;
    (*m_ssEntity) << "ARC"       << endl;
    (*m_ssEntity) << "  5"      << endl;
    (*m_ssEntity) << getEntityHandle() << endl;
    if (m_version > 12) {
        (*m_ssEntity) << "330"      << endl;
        (*m_ssEntity) << m_saveModelSpaceHandle  << endl;
        (*m_ssEntity) << "100"      << endl;
        (*m_ssEntity) << "AcDbEntity"      << endl;
    }
    (*m_ssEntity) << "  8"       << endl;    // Group code for layer name
    (*m_ssEntity) << getLayerName()  << endl;    // Layer number
//    (*m_ssEntity) << " 62"          << endl;
//    (*m_ssEntity) << "     0"       << endl;
     if (m_version > 12) {
        (*m_ssEntity) << "100"          << endl;
        (*m_ssEntity) << "AcDbCircle"   << endl;
    }
    (*m_ssEntity) << " 10"       << endl;    // Centre X
    (*m_ssEntity) << c[0]        << endl;    // X in WCS coordinates
    (*m_ssEntity) << " 20"       << endl;
    (*m_ssEntity) << c[1]        << endl;    // Y in WCS coordinates
    (*m_ssEntity) << " 30"       << endl;
    (*m_ssEntity) << c[2]        << endl;    // Z in WCS coordinates
    (*m_ssEntity) << " 40"       << endl;    //
    (*m_ssEntity) << radius      << endl;    // Radius

    if (m_version > 12) {
        (*m_ssEntity) << "100"      << endl;
        (*m_ssEntity) << "AcDbArc" << endl;
    }
    (*m_ssEntity) << " 50"       << endl;
    (*m_ssEntity) << start_angle << endl;    // Start angle
    (*m_ssEntity) << " 51"       << endl;
    (*m_ssEntity) << end_angle   << endl;    // End angle
}

void CDxfWrite::writeCircle(const double* c, double radius)
{
    (*m_ssEntity) << "  0"       << endl;
    (*m_ssEntity) << "CIRCLE"    << endl;
    (*m_ssEntity) << "  5"      << endl;
    (*m_ssEntity) << getEntityHandle() << endl;
    if (m_version > 12) {
        (*m_ssEntity) << "330"      << endl;
        (*m_ssEntity) << m_saveModelSpaceHandle  << endl;
        (*m_ssEntity) << "100"      << endl;
        (*m_ssEntity) << "AcDbEntity"      << endl;
    }
    (*m_ssEntity) << "  8"       << endl;    // Group code for layer name
    (*m_ssEntity) << getLayerName()  << endl;    // Layer number
     if (m_version > 12) {
        (*m_ssEntity) << "100"          << endl;
        (*m_ssEntity) << "AcDbCircle"   << endl;
    }
    (*m_ssEntity) << " 10"       << endl;    // Centre X
    (*m_ssEntity) << c[0]        << endl;    // X in WCS coordinates
    (*m_ssEntity) << " 20"       << endl;
    (*m_ssEntity) << c[1]        << endl;    // Y in WCS coordinates
//    (*m_ssEntity) << " 30"       << endl;
//    (*m_ssEntity) << c[2]        << endl;    // Z in WCS coordinates
    (*m_ssEntity) << " 40"       << endl;    //
    (*m_ssEntity) << radius      << endl;    // Radius
}

void CDxfWrite::writeEllipse(const double* c, double major_radius, double minor_radius, 
                             double rotation, double start_angle, double end_angle,
                             bool endIsCW)
{
    double m[3];
    m[2]=0;
    m[0] = major_radius * sin(rotation);
    m[1] = major_radius * cos(rotation);

    double ratio = minor_radius/major_radius;

    if(!endIsCW){                          //end is NOT CW from start
        double temp = start_angle;
        start_angle = end_angle;
        end_angle = temp;
    }
    (*m_ssEntity) << "  0"       << endl;
    (*m_ssEntity) << "ELLIPSE"   << endl;
    (*m_ssEntity) << "  5"      << endl;
    (*m_ssEntity) << getEntityHandle() << endl;
    if (m_version > 12) {
        (*m_ssEntity) << "330"      << endl;
        (*m_ssEntity) << m_saveModelSpaceHandle  << endl;
        (*m_ssEntity) << "100"      << endl;
        (*m_ssEntity) << "AcDbEntity"      << endl;
    }
    (*m_ssEntity) << "  8"       << endl;    // Group code for layer name
    (*m_ssEntity) << getLayerName()  << endl;    // Layer number
     if (m_version > 12) {
        (*m_ssEntity) << "100"          << endl;
        (*m_ssEntity) << "AcDbEllipse"   << endl;
    }
    (*m_ssEntity) << " 10"       << endl;    // Centre X
    (*m_ssEntity) << c[0]        << endl;    // X in WCS coordinates
    (*m_ssEntity) << " 20"       << endl;
    (*m_ssEntity) << c[1]        << endl;    // Y in WCS coordinates
    (*m_ssEntity) << " 30"       << endl;
    (*m_ssEntity) << c[2]        << endl;    // Z in WCS coordinates
    (*m_ssEntity) << " 11"       << endl;    //
    (*m_ssEntity) << m[0]        << endl;    // Major X
    (*m_ssEntity) << " 21"       << endl;
    (*m_ssEntity) << m[1]        << endl;    // Major Y
    (*m_ssEntity) << " 31"       << endl;
    (*m_ssEntity) << m[2]        << endl;    // Major Z
    (*m_ssEntity) << " 40"       << endl;    //
    (*m_ssEntity) << ratio       << endl;    // Ratio
//    (*m_ssEntity) << "210"       << endl;    //extrusion dir??
//    (*m_ssEntity) << "0"         << endl;
//    (*m_ssEntity) << "220"       << endl;
//    (*m_ssEntity) << "0"         << endl;
//    (*m_ssEntity) << "230"       << endl;
//    (*m_ssEntity) << "1"         << endl;
    (*m_ssEntity) << " 41"       << endl;
    (*m_ssEntity) << start_angle << endl;    // Start angle (radians [0..2pi])
    (*m_ssEntity) << " 42"       << endl;
    (*m_ssEntity) << end_angle   << endl;    // End angle
}

//***************************
//writeSpline
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::writeSpline(const SplineDataOut &sd)
{
    (*m_ssEntity) << "  0"          << endl;
    (*m_ssEntity) << "SPLINE"       << endl;
    (*m_ssEntity) << "  5"      << endl;
    (*m_ssEntity) << getEntityHandle() << endl;
    if (m_version > 12) {
        (*m_ssEntity) << "330"      << endl;
        (*m_ssEntity) << m_saveModelSpaceHandle  << endl;
        (*m_ssEntity) << "100"      << endl;
        (*m_ssEntity) << "AcDbEntity"      << endl;
    }
    (*m_ssEntity) << "  8"          << endl;    // Group code for layer name
    (*m_ssEntity) << getLayerName()     << endl;    // Layer name
    if (m_version > 12) {
        (*m_ssEntity) << "100"          << endl;
        (*m_ssEntity) << "AcDbSpline"   << endl;
    }
    (*m_ssEntity) << "210"          << endl;
    (*m_ssEntity) << "0"            << endl;
    (*m_ssEntity) << "220"          << endl;
    (*m_ssEntity) << "0"            << endl;
    (*m_ssEntity) << "230"          << endl;
    (*m_ssEntity) << "1"            << endl;

    (*m_ssEntity) << " 70"          << endl;
    (*m_ssEntity) << sd.flag        << endl;      //flags
    (*m_ssEntity) << " 71"          << endl; 
    (*m_ssEntity) << sd.degree      << endl;
    (*m_ssEntity) << " 72"          << endl;
    (*m_ssEntity) << sd.knots       << endl;
    (*m_ssEntity) << " 73"          << endl;
    (*m_ssEntity) << sd.control_points   << endl;
    (*m_ssEntity) << " 74"          << endl; 
    (*m_ssEntity) << 0              << endl;

//    (*m_ssEntity) << " 12"          << endl;
//    (*m_ssEntity) << sd.starttan.x  << endl;
//    (*m_ssEntity) << " 22"          << endl;
//    (*m_ssEntity) << sd.starttan.y  << endl;
//    (*m_ssEntity) << " 32"          << endl;
//    (*m_ssEntity) << sd.starttan.z  << endl;
//    (*m_ssEntity) << " 13"          << endl;
//    (*m_ssEntity) << sd.endtan.x    << endl;
//    (*m_ssEntity) << " 23"          << endl;
//    (*m_ssEntity) << sd.endtan.y    << endl;
//    (*m_ssEntity) << " 33"          << endl;
//    (*m_ssEntity) << sd.endtan.z    << endl;

    for (auto& k: sd.knot) {
        (*m_ssEntity) << " 40"      << endl;  
        (*m_ssEntity) << k          << endl;  
    }

    for (auto& w : sd.weight) {
        (*m_ssEntity) << " 41"      << endl;  
        (*m_ssEntity) << w          << endl;  
    }

    for (auto& c: sd.control) {
        (*m_ssEntity) << " 10"      << endl;
        (*m_ssEntity) << c.x        << endl;    // X in WCS coordinates
        (*m_ssEntity) << " 20"      << endl;
        (*m_ssEntity) << c.y        << endl;    // Y in WCS coordinates
        (*m_ssEntity) << " 30"      << endl;
        (*m_ssEntity) << c.z        << endl;    // Z in WCS coordinates
    }
    for (auto& f: sd.fit) {
        (*m_ssEntity) << " 11"      << endl;
        (*m_ssEntity) << f.x        << endl;    // X in WCS coordinates
        (*m_ssEntity) << " 21"      << endl;
        (*m_ssEntity) << f.y        << endl;    // Y in WCS coordinates
        (*m_ssEntity) << " 31"      << endl;
        (*m_ssEntity) << f.z        << endl;    // Z in WCS coordinates
    }
}

//***************************
//writeVertex
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::writeVertex(double x, double y, double z)
{
    (*m_ssEntity) << "  0"          << endl;
    (*m_ssEntity) << "VERTEX"       << endl;
    (*m_ssEntity) << "  5"      << endl;
    (*m_ssEntity) << getEntityHandle() << endl;
    if (m_version > 12) {
        (*m_ssEntity) << "330"      << endl;
        (*m_ssEntity) << m_saveModelSpaceHandle  << endl;
        (*m_ssEntity) << "100"      << endl;
        (*m_ssEntity) << "AcDbEntity"      << endl;
    }
    (*m_ssEntity) << "  8"          << endl;
    (*m_ssEntity) << getLayerName()     << endl;
    if (m_version > 12) {
        (*m_ssEntity) << "100"          << endl;
        (*m_ssEntity) << "AcDbVertex"   << endl;
    }
    (*m_ssEntity) << " 10"          << endl;
    (*m_ssEntity) << x              << endl;
    (*m_ssEntity) << " 20"          << endl; 
    (*m_ssEntity) << y              << endl;
    (*m_ssEntity) << " 30"          << endl;
    (*m_ssEntity) << z              << endl;
    (*m_ssEntity) << " 70"          << endl;
    (*m_ssEntity) << 0              << endl;
}

void CDxfWrite::writeText(const char* text, const double* location1, const double* location2,
                          const double height, const int horizJust)
{
    putText(text, toVector3d(location1), toVector3d(location2),
            height, horizJust, 
            m_ssEntity, getEntityHandle(), m_saveModelSpaceHandle);
}                                     

//***************************
//putText
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::putText(const char* text, const gp_Vec& location1, const gp_Vec& location2,
                        const double height, const int horizJust,
                        std::ostringstream* outStream, const std::string handle,
                        const std::string ownerHandle)
{
    (void) location2;

    (*outStream) << "  0"          << endl;
    (*outStream) << "TEXT"         << endl;
    (*outStream) << "  5"      << endl;
    (*outStream) << handle << endl;
    if (m_version > 12) {
        (*outStream) << "330"      << endl;
        (*outStream) << ownerHandle  << endl;
        (*outStream) << "100"      << endl;
        (*outStream) << "AcDbEntity"      << endl;
    }
    (*outStream) << "  8"          << endl;
    (*outStream) << getLayerName()     << endl;
    if (m_version > 12) {
        (*outStream) << "100"          << endl;
        (*outStream) << "AcDbText"     << endl;
    }
//    (*outStream) << " 39"          << endl;
//    (*outStream) << 0              << endl;     //thickness
    (*outStream) << " 10"          << endl;     //first alignment point
    (*outStream) << location1.X()  << endl;
    (*outStream) << " 20"          << endl; 
    (*outStream) << location1.Y()  << endl;
    (*outStream) << " 30"          << endl;
    (*outStream) << location1.Z()  << endl;
    (*outStream) << " 40"          << endl;
    (*outStream) << height         << endl;
    (*outStream) << "  1"          << endl;
    (*outStream) << text           << endl;
//    (*outStream) << " 50"          << endl;
//    (*outStream) << 0              << endl;    //rotation
//    (*outStream) << " 41"          << endl;
//    (*outStream) << 1              << endl;
//    (*outStream) << " 51"          << endl;
//    (*outStream) << 0              << endl;

    (*outStream) << "  7"          << endl;
    (*outStream) << "STANDARD"     << endl;    //style
//    (*outStream) << " 71"          << endl;  //default
//    (*outStream) << "0"            << endl;
    (*outStream) << " 72"          << endl;
    (*outStream) << horizJust      << endl;
////    (*outStream) << " 73"          << endl;
////    (*outStream) << "0"            << endl;
    (*outStream) << " 11"          << endl;    //second alignment point
    (*outStream) << location2.X()  << endl;
    (*outStream) << " 21"          << endl; 
    (*outStream) << location2.Y()  << endl;
    (*outStream) << " 31"          << endl;
    (*outStream) << location2.Z()  << endl;
//    (*outStream) << "210"          << endl;
//    (*outStream) << "0"            << endl;
//    (*outStream) << "220"          << endl;
//    (*outStream) << "0"            << endl;
//    (*outStream) << "230"          << endl;
//    (*outStream) << "1"            << endl;
    if (m_version > 12) {
        (*outStream) << "100"          << endl;
        (*outStream) << "AcDbText"     << endl;
    }
    
}

void CDxfWrite::putArrow(const gp_Vec& arrowPos, const gp_Vec& barb1Pos, const gp_Vec& barb2Pos,
                         std::ostringstream* outStream, const std::string handle,
                         const std::string ownerHandle)
{
    (*outStream) << "  0"          << endl;
    (*outStream) << "SOLID"        << endl;
    (*outStream) << "  5"          << endl;
    (*outStream) << handle         << endl;
    if (m_version > 12) {
        (*outStream) << "330"      << endl;
        (*outStream) << ownerHandle << endl;
        (*outStream) << "100"      << endl;
        (*outStream) << "AcDbEntity"      << endl;
    }
    (*outStream) << "  8"          << endl;
    (*outStream) << "0"            << endl;
    (*outStream) << " 62"          << endl;
    (*outStream) << "     0"       << endl;
    if (m_version > 12) {
        (*outStream) << "100"      << endl;
        (*outStream) << "AcDbTrace" << endl;
    }
    (*outStream) << " 10"          << endl;
    (*outStream) << barb1Pos.X()   << endl;
    (*outStream) << " 20"          << endl;
    (*outStream) << barb1Pos.Y()   << endl;
    (*outStream) << " 30"          << endl;
    (*outStream) << barb1Pos.Z()   << endl;
    (*outStream) << " 11"          << endl;
    (*outStream) << barb2Pos.X()   << endl;
    (*outStream) << " 21"          << endl;
    (*outStream) << barb2Pos.Y()   << endl;
    (*outStream) << " 31"          << endl;
    (*outStream) << barb2Pos.Z()   << endl;
    (*outStream) << " 12"          << endl;
    (*outStream) << arrowPos.X()   << endl;
    (*outStream) << " 22"          << endl;
    (*outStream) << arrowPos.Y()   << endl;
    (*outStream) << " 32"          << endl;
    (*outStream) << arrowPos.Z()   << endl;
    (*outStream) << " 13"          << endl;
    (*outStream) << arrowPos.X()   << endl;
    (*outStream) << " 23"          << endl;
    (*outStream) << arrowPos.Y()   << endl;
    (*outStream) << " 33"          << endl;
    (*outStream) << arrowPos.Z()   << endl;
}

//***************************
//writeBlocksSection
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::writeBlocksSection(void)
{
    if (m_version < 14) {
        std::stringstream ss;
        ss << "blocks1" << m_version << ".rub";
        std::string fileSpec = m_dataDir + ss.str();
        (*m_ofs) << getPlateFile(fileSpec);
    }
    
    //write blocks content
    (*m_ofs) << (*m_ssBlock).str();

    (*m_ofs) << "  0"      << endl;
    (*m_ofs) << "ENDSEC"   << endl;
}

//***************************
//writeEntitiesSection
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::writeEntitiesSection(void)
{
    std::stringstream ss;
    ss << "entities" << m_version << ".rub";
    std::string fileSpec = m_dataDir + ss.str();
    (*m_ofs) << getPlateFile(fileSpec);
    
    //write entities content
    (*m_ofs) << (*m_ssEntity).str();
    

    (*m_ofs) << "  0"      << endl;
    (*m_ofs) << "ENDSEC"   << endl;
}

//***************************
//writeObjectsSection
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::writeObjectsSection(void)
{
    if (m_version < 14) {
        return;
    }
    std::stringstream ss;
    ss << "objects" << m_version << ".rub";
    std::string fileSpec = m_dataDir + ss.str();
    (*m_ofs) << getPlateFile(fileSpec);
}
