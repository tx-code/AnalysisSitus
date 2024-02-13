// dxf.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
// modified 2018 wandererfan

#ifndef _dxf_h_
#define _dxf_h_

// asiAlgo includes
#include <asiAlgo.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

// Standard includes
#include <algorithm>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <math.h>

// Following is required to be defined on Ubuntu with OCC 6.3.1
#ifndef HAVE_IOSTREAM
#define HAVE_IOSTREAM
#endif

typedef int Aci_t; // AutoCAD color index

//! \ingroup ASI_INTEROP
//!
typedef enum
{
    eUnspecified = 0,   // Unspecified (No units)
    eInches,
    eFeet,
    eMiles,
    eMillimeters,
    eCentimeters,
    eMeters,
    eKilometers,
    eMicroinches,
    eMils,
    eYards,
    eAngstroms,
    eNanometers,
    eMicrons,
    eDecimeters,
    eDekameters,
    eHectometers,
    eGigameters,
    eAstronomicalUnits,
    eLightYears,
    eParsecs
} eDxfUnits_t;


//! \ingroup ASI_INTEROP
//!
//spline data for reading
struct SplineData
{
    double norm[3];
    int degree;
    int knots;
    int control_points;
    int fit_points;
    int flag;
    std::list<double> starttanx;
    std::list<double> starttany;
    std::list<double> starttanz;
    std::list<double> endtanx;
    std::list<double> endtany;
    std::list<double> endtanz;
    std::list<double> knot;
    std::list<double> weight;
    std::list<double> controlx;
    std::list<double> controly;
    std::list<double> controlz;
    std::list<double> fitx;
    std::list<double> fity;
    std::list<double> fitz;
};

//! \ingroup ASI_INTEROP
//!
//***************************
//data structures for writing
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
struct point3D
{
    double x;
    double y;
    double z;
};

//! \ingroup ASI_INTEROP
//!
struct SplineDataOut
{
    point3D norm;
    int degree;
    int knots;
    int control_points;
    int fit_points;
    int flag;
    point3D starttan;
    point3D endtan;
    std::vector<double> knot;
    std::vector<double> weight;
    std::vector<point3D> control;
    std::vector<point3D> fit;
};

//! \ingroup ASI_INTEROP
//!
struct LWPolyDataOut
{
    double nVert;
    int    Flag;
    double Width;
    double Elev;
    double Thick;
    std::vector<point3D> Verts;
    std::vector<double> StartWidth;
    std::vector<double> EndWidth;
    std::vector<double> Bulge;
    point3D Extr;
};
//********************

//! \ingroup ASI_INTEROP
//!
class CDxfWrite : public ActAPI_IAlgorithm
{
private:
    std::ofstream* m_ofs;
    bool m_fail;
    std::ostringstream* m_ssBlock;
    std::ostringstream* m_ssBlkRecord;
    std::ostringstream* m_ssEntity;
    std::ostringstream* m_ssLayer;

protected:
    void putLine(const gp_Vec& s, const gp_Vec& e,
                 std::ostringstream* outStream, const std::string handle,
                 const std::string ownerHandle);
    void putText(const char* text, const gp_Vec&location1, const gp_Vec& location2,
                 const double height, const int horizJust,
                 std::ostringstream* outStream, const std::string handle,
                 const std::string ownerHandle);
    void putArrow(const gp_Vec& arrowPos, const gp_Vec& barb1Pos, const gp_Vec& barb2Pos,
                  std::ostringstream* outStream, const std::string handle,
                  const std::string ownerHandle);

    //! copy boiler plate file
    std::string getPlateFile(std::string fileSpec);
    void setDataDir(std::string s) { m_dataDir = s; }
    std::string getHandle(void);
    std::string getEntityHandle(void);
    std::string getLayerHandle(void);
    std::string getBlockHandle(void);
    std::string getBlkRecordHandle(void);

    std::string m_optionSource;
    int m_version;
    int m_handle;
    int m_entityHandle;
    int m_layerHandle;
    int m_blockHandle;
    int m_blkRecordHandle;
    bool m_polyOverride;
    
    std::string m_saveModelSpaceHandle;
    std::string m_savePaperSpaceHandle;
    std::string m_saveBlockRecordTableHandle;
    std::string m_saveBlkRecordHandle;
    std::string m_currentBlock;
    std::string m_dataDir;
    std::string m_layerName;
    std::vector<std::string> m_layerList;
    std::vector<std::string> m_blockList;
    std::vector<std::string> m_blkRecordList;

public:

    asiAlgo_EXPORT
      CDxfWrite(const char*          filepath,
                ActAPI_ProgressEntry progress,
                ActAPI_PlotterEntry  plotter);

    asiAlgo_EXPORT
      ~CDxfWrite();

public:

    void init(void);
    void endRun(void);

    bool failed() const {return m_fail;}
//    void setOptions(void);
//    bool isVersionValid(int vers);
    std::string getLayerName() { return m_layerName; }
    void setLayerName(std::string s);
    void setVersion(int v) { m_version = v;}
    void setPolyOverride(bool b) { m_polyOverride = b; }
    void addBlockName(std::string s, std::string blkRecordHandle);

    void writeLine(const double* s, const double* e);
    void writePoint(const double*);
    void writeArc(const double* s, const double* e, const double* c, bool dir);
    void writeEllipse(const double* c, double major_radius, double minor_radius, 
                      double rotation, double start_angle, double end_angle, bool endIsCW);
    void writeCircle(const double* c, double radius );
    void writeSpline(const SplineDataOut &sd);
    void writeLWPolyLine(const LWPolyDataOut &pd);
    void writePolyline(const LWPolyDataOut &pd);
    void writeVertex(double x, double y, double z);
    void writeText(const char* text, const double* location1, const double* location2,
                   const double height, const int horizJust);

    void writeHeaderSection(void);
    void writeTablesSection(void);
    void writeBlocksSection(void);
    void writeEntitiesSection(void);
    void writeObjectsSection(void);
    void writeClassesSection(void);

    void makeLayerTable(void);
    void makeBlockRecordTableHead(void);
    void makeBlockRecordTableBody(void);
    void makeBlockSectionHead(void);
};

#endif // _dxf_h_
