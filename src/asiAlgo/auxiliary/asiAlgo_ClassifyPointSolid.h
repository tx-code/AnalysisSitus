//-----------------------------------------------------------------------------
// Copyright (c) 2020-present, Quaoar Studio
// All rights reserved.
//-----------------------------------------------------------------------------

#ifndef ClassifyPt_h
#define ClassifyPt_h

// OpenCascade includes
#include <BRep_Builder.hxx>
#include <BRepBndLib.hxx>
#include <BVH_PrimitiveSet.hxx>
#include <Poly_Triangulation.hxx>
#include <Precision.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>

//-----------------------------------------------------------------------------

const static double REAL_MIN = std::numeric_limits<double>::min();
const static double REAL_MAX = std::numeric_limits<double>::max();

//-----------------------------------------------------------------------------

//! \ingroup ASI_CORE
//!
//! RNG (Random number generator) based on code presented by Ian C. Bullard (more
//! accurate than standard C rand() and at least 10 times faster).
class BullardRNG
{
public:

  BullardRNG(const unsigned seed = 1)
  {
    this->Initialize(seed);
  }

  void Initialize(const unsigned seed)
  {
    m_iHi = seed;
    m_iLo = seed ^ 0x49616E42;
  }

  unsigned RandInt()
  {
    static const int shift = sizeof(int) / 2;
    m_iHi = (m_iHi >> shift) + (m_iHi << shift);
    m_iHi += m_iLo;
    m_iLo += m_iHi;
    return m_iHi;
  }

  double RandDouble()
  {
    return RandInt() / (double) (0xFFFFFFFF);
  }

protected:

  unsigned m_iLo, m_iHi;

};

//-----------------------------------------------------------------------------

//! \ingroup ASI_CORE
//!
//! BVH-based accelerating structure representing CAD model's
//! facets in computations.
class ModelBvh : public BVH_PrimitiveSet<double, 3>
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

public:

  //! Creates the accelerating structure with immediate initialization.
  ModelBvh(const TopoDS_Shape& model)
  //
  : BVH_PrimitiveSet<double, 3> (),
    m_fBoundingDiag             (0.0)
  {
    this->init(model);
    this->MarkDirty();
  }

  //! Creates the accelerating structure with immediate initialization.
  ModelBvh(const Handle(Poly_Triangulation)& mesh)
  //
  : BVH_PrimitiveSet<double, 3> (),
    m_fBoundingDiag             (0.0)
  {
    this->init(mesh);
    this->MarkDirty();
  }

public:

  //! \return number of stored facets.
  virtual int Size() const override { return (int) m_facets.size(); }

  //! Builds an elementary box for a facet with the given index.
  virtual BVH_Box<double, 3> Box(const int index) const override
  {
    BVH_Box<double, 3> box;
    box.Add(m_facets[index].P0);
    box.Add(m_facets[index].P1);
    box.Add(m_facets[index].P2);
    return box;
  }

  //! Calculates center point of a facet with respect to the axis of interest.
  virtual double Center(const int index,
                        const int axis) const override
  {
    const t_facet& facet = m_facets[index];

    if ( axis == 0 )
      return (1.0 / 3.0) * ( facet.P0.x() + facet.P1.x() + facet.P2.x() );

    if ( axis == 1 )
      return (1.0 / 3.0) * ( facet.P0.y() + facet.P1.y() + facet.P2.y() );

    // The last possibility is "axis == 2"
    return (1.0 / 3.0) * ( facet.P0.z() + facet.P1.z() + facet.P2.z() );
  }

  //! Swaps two elements for BVH building.
  virtual void Swap(const int index1,
                    const int index2) override
  {
    std::swap(m_facets[index1], m_facets[index2]);
  }

  //! Returns vertices for a facet with the given 0-based index.
  void GetVertices(const int  index,
                   BVH_Vec3d& vertex1,
                   BVH_Vec3d& vertex2,
                   BVH_Vec3d& vertex3) const
  {
    vertex1 = m_facets[index].P0;
    vertex2 = m_facets[index].P1;
    vertex3 = m_facets[index].P2;
  }

  //! \return characteristic diagonal of the full model.
  double GetBoundingDiag() const { return m_fBoundingDiag; }

  //! Returns a facet by its 0-based index.
  //! \param[in] index index of the facet of interest.
  //! \return requested facet.
  const t_facet& GetFacet(const int index) { return m_facets[index]; }

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
  bool init(const TopoDS_Shape& model)
  {
    if ( model.IsNull() )
      return false;

    // Prepare builder
    myBuilder = new BVH_BinnedBuilder<double, 3, 32>(5, 32);

    // Explode shape on faces to get face indices
    if ( m_faces.IsEmpty() )
      TopExp::MapShapes(model, TopAbs_FACE, m_faces);

    // Initialize with facets taken from faces
    for ( int fidx = 1; fidx <= m_faces.Extent(); ++fidx )
    {
      const TopoDS_Face& face = TopoDS::Face( m_faces(fidx) );
      //
      if ( !this->addFace(face, fidx) )
        continue; // Do not return false, just skip as otherwise
                  // BVH will be incorrect for faulty shapes!
    }

    // Calculate bounding diagonal
    Bnd_Box aabb;
    BRepBndLib::Add(model, aabb);
    //
    m_fBoundingDiag = ( aabb.CornerMax().XYZ() - aabb.CornerMin().XYZ() ).Modulus();

    return true;
  }

  //! Initializes the accelerating structure with the given triangulation.
  bool init(const Handle(Poly_Triangulation)& mesh)
  {
    // Prepare builder
    myBuilder = new BVH_BinnedBuilder<double, 3, 32>(5, 32);

    // Initialize with the passed facets
    if ( !this->addTriangulation(mesh, TopLoc_Location(), -1, false) )
      return false;

    // Calculate bounding diagonal using fictive face to satisfy OpenCascade's API
    BRep_Builder BB;
    TopoDS_Face F;
    BB.MakeFace(F, mesh);
    Bnd_Box aabb;
    BRepBndLib::Add(F, aabb);
    //
    m_fBoundingDiag = ( aabb.CornerMax().XYZ() - aabb.CornerMin().XYZ() ).Modulus();

    return true;
  }

  //! Adds face to the accelerating structure.
  bool addFace(const TopoDS_Face& face, const int face_idx)
  {
    TopLoc_Location loc;
    const Handle(Poly_Triangulation)& tris = BRep_Tool::Triangulation(face, loc);
    return this->addTriangulation( tris, loc, face_idx, (face.Orientation() == TopAbs_REVERSED) );
  }

  //! Adds triangulation to the accelerating structure.
  bool addTriangulation(const Handle(Poly_Triangulation)& triangulation,
                        const TopLoc_Location&            loc,
                        const int                         face_idx,
                        const bool                        isReversed)
  {
    if ( triangulation.IsNull() )
      return false;

    // Internal collections of triangles and nodes
    for ( int elemId = 1; elemId <= triangulation->NbTriangles(); ++elemId )
    {
      const Poly_Triangle& tri = triangulation->Triangle(elemId);

      int n1, n2, n3;
      tri.Get(n1, n2, n3);

      gp_Pnt P0 = triangulation->Node(isReversed ? n3 : n1);
      P0.Transform(loc);
      //
      gp_Pnt P1 = triangulation->Node(n2);
      P1.Transform(loc);
      //
      gp_Pnt P2 = triangulation->Node(isReversed ? n1 : n3);
      P2.Transform(loc);

      // Create a new facet
      t_facet facet(face_idx == -1 ? elemId : face_idx);

      // Initialize nodes
      facet.P0 = BVH_Vec3d( P0.X(), P0.Y(), P0.Z() );
      facet.P1 = BVH_Vec3d( P1.X(), P1.Y(), P1.Z() );
      facet.P2 = BVH_Vec3d( P2.X(), P2.Y(), P2.Z() );

      /* Initialize normal */

      gp_Vec V1(P0, P1);
      //
      if ( V1.SquareMagnitude() < 1e-8 )
        continue; // Skip invalid facet.
      //
      V1.Normalize();

      gp_Vec V2(P0, P2);
      //
      if ( V2.SquareMagnitude() < 1e-8 )
        continue; // Skip invalid facet.
      //
      V2.Normalize();

      // Compute norm
      facet.N = V1.Crossed(V2);
      //
      if ( facet.N.SquareMagnitude() < 1e-8 )
        continue; // Skip invalid facet
      //
      facet.N.Normalize();

      // Store facet in the internal collection
      m_facets.push_back(facet);
    }

    return true;
  }

protected:

  //! Map of faces constructed by the BVH builder.
  TopTools_IndexedMapOfShape m_faces;

  //! Array of facets.
  std::vector<t_facet> m_facets;

  //! Characteristic size of the model.
  double m_fBoundingDiag;

};

//-----------------------------------------------------------------------------

//! \ingroup ASI_MODELING
//!
//! Distance function to be used for spatial shape representations.
class MeshDist : public Standard_Transient
{
public:

  //! Tool class describing a ray.
  struct t_ray
  {
    BVH_Vec3d Origin; //!< Origin point of a ray
    BVH_Vec3d Direct; //!< Direction vector of a ray

    //! Default ctor.
    t_ray() = default;

    //! Creates a new ray with the given origin and direction.
    t_ray(const BVH_Vec3d& O,
          const BVH_Vec3d& D) : Origin(O), Direct(D) {}
  };

public:

  //! Ctor.
  MeshDist(const int numRays = 3) : m_iNumRays(numRays), m_RNG(128) {}

  //! Ctor with initialization.
  MeshDist(const Handle(ModelBvh)& facets,
           const int               numRays = 3) : m_iNumRays(numRays), m_RNG(128)
  {
    this->Init(facets);
  }

  //! Initializes the distance function with the existing mesh.
  bool Init(const Handle(ModelBvh)& facets)
  {
    if ( facets.IsNull() )
      return false;

    m_facets = facets;
    return true;
  }

  //! Evaluates function for the given coordinates.
  //! \return evaluated distance.
  virtual double Eval(const double x, const double y, const double z) const
  {
    // Project point on mesh.
    gp_Pnt P(x, y, z);

    // Get unsigned distance.
    const double
      d2 = squaredDistanceToMesh( m_facets.get(), BVH_Vec3d(x, y, z) );
    //
    if ( d2 == REAL_MAX )
      return REAL_MAX;

    // Get the unsigned distance.
    const double ud = Sqrt(d2);

    // Check sign by ray casting several times with random direction.
    bool isOutside = true;
    int  vote      = 0;
    int  barrier   = int( std::ceil(double(m_iNumRays) / 2.) );

    for ( int rayIdx = 0; rayIdx < m_iNumRays; ++rayIdx )
    {
      if ( vote > barrier || vote < -barrier )
        break;

      // Initialize random ray.
      t_ray ray( BVH_Vec3d(x, y, z),
                 BVH_Vec3d( m_RNG.RandDouble() * 2.0 - 1.0,
                            m_RNG.RandDouble() * 2.0 - 1.0,
                            m_RNG.RandDouble() * 2.0 - 1.0) );
      //
      const int numBounces = rayMeshHitCount(m_facets.get(), ray);
      //
      if ( numBounces % 2 != 0 )
      {
        --vote;
      }
      else
      {
        ++vote;
      }
    }

    isOutside = vote > 0;

    return (isOutside ? 1 : -1) * ud;
  }

protected:

  static double intersectTriangle(const t_ray&     ray,
                                  const BVH_Vec3d& P0,
                                  const BVH_Vec3d& P1,
                                  const BVH_Vec3d& P2)
  {
    const BVH_Vec3d E0 = P1 - P0;
    const BVH_Vec3d E1 = P0 - P2;

    // Norm vector.
    const BVH_Vec3d N( E1.y()*E0.z() - E1.z()*E0.y(),
                       E1.z()*E0.x() - E1.x()*E0.z(),
                       E1.x()*E0.y() - E1.y()*E0.x() );

    const double NdotD = N.Dot(ray.Direct);
    //
    if ( Abs(NdotD) < Precision::Confusion() )
      return REAL_MAX;

    const double InvNdotD = 1.0 / NdotD;
    const BVH_Vec3d E2 ( ( P0.x() - ray.Origin.x() )*InvNdotD,
                         ( P0.y() - ray.Origin.y() )*InvNdotD,
                         ( P0.z() - ray.Origin.z() )*InvNdotD);

    const double time = N.Dot(E2);
    if ( time < 0.0 )
      return REAL_MAX;

    const BVH_Vec3d direct( ray.Direct.y()*E2.z() - ray.Direct.z()*E2.y(),
                            ray.Direct.z()*E2.x() - ray.Direct.x()*E2.z(),
                            ray.Direct.x()*E2.y() - ray.Direct.y()*E2.x() );

    const double U = direct.Dot(E1);
    const double V = direct.Dot(E0);

    return ( U < 0.0 || V < 0.0 || U + V > 1.0 ) ? REAL_MAX : time;
  }

  //! Computes number of ray-mesh intersections.
  static int rayMeshHitCount(ModelBvh* pMesh, const t_ray& ray)
  {
    const BVH_Tree<double, 3>* pBVH = (pMesh != nullptr) ? pMesh->BVH().get() : nullptr;
    if ( pBVH == nullptr )
      return 0;

    // Invert.
    BVH_Vec3d invDirect = ray.Direct.cwiseAbs();
    //
    invDirect.x() = 1.0 / std::max( std::numeric_limits<double>::epsilon(), invDirect.x() );
    invDirect.y() = 1.0 / std::max( std::numeric_limits<double>::epsilon(), invDirect.y() );
    invDirect.z() = 1.0 / std::max( std::numeric_limits<double>::epsilon(), invDirect.z() );
    //
    invDirect.x() = std::copysign( invDirect.x(), ray.Direct.x() );
    invDirect.y() = std::copysign( invDirect.y(), ray.Direct.y() );
    invDirect.z() = std::copysign( invDirect.z(), ray.Direct.z() );

    int head = -1; // Stack head.
    int node =  0; // Root index.
    int stack[64];
    //
    for ( int numBounces = 0 ; ; )
    {
      if ( node >= (int) pBVH->NodeInfoBuffer().size() )
        return 0;

      BVH_Vec4i data = pBVH->NodeInfoBuffer()[node];

      if ( data.x() == 0 ) // Inner node.
      {
        BVH_Vec3d time0 = ( pBVH->MinPoint( data.y() ) - ray.Origin ) * invDirect;
        BVH_Vec3d time1 = ( pBVH->MaxPoint( data.y() ) - ray.Origin ) * invDirect;

        BVH_Vec3d timeMax = time0.cwiseMax(time1);
        BVH_Vec3d timeMin = time0.cwiseMin(time1);

        time0 = ( pBVH->MinPoint(data.z() ) - ray.Origin) * invDirect;
        time1 = ( pBVH->MaxPoint(data.z() ) - ray.Origin) * invDirect;

        double timeFinal = std::min( timeMax.x(), std::min( timeMax.y(), timeMax.z() ) );
        double timeStart = std::max( timeMin.x(), std::max( timeMin.y(), timeMin.z() ) );

        timeMax = time0.cwiseMax(time1);
        timeMin = time0.cwiseMin(time1);

        const double timeMin1 = (timeStart <= timeFinal) && (timeFinal >= 0) ? timeStart : REAL_MAX;

        timeFinal = std::min( timeMax.x(), std::min( timeMax.y(), timeMax.z() ) );
        timeStart = std::max( timeMin.x(), std::max( timeMin.y(), timeMin.z() ) );

        const double timeMin2 = (timeStart <= timeFinal) && (timeFinal >= 0) ? timeStart : REAL_MAX;

        const bool hitLft = timeMin1 != REAL_MAX;
        const bool hitRgh = timeMin2 != REAL_MAX;

        if ( hitLft && hitRgh )
        {
          node = (timeMin1 < timeMin2) ? data.y() : data.z();

          stack[++head] = timeMin1 < timeMin2 ? data.z() : data.y();
        }
        else if ( hitLft || hitRgh )
        {
          node = hitLft ? data.y() : data.z();
        }
        else
        {
          if ( head < 0 )
          {
            return numBounces;
          }

          node = stack[head--];
        }
      }
      else // Leaf node.
      {
        for ( int tidx = data.y(); tidx <= data.z(); ++tidx )
        {
          const ModelBvh::t_facet& facet = pMesh->GetFacet(tidx);

          // Get next facet to test
          const gp_XYZ p0( facet.P0.x(), facet.P0.y(), facet.P0.z() );
          const gp_XYZ p1( facet.P1.x(), facet.P1.y(), facet.P1.z() );
          const gp_XYZ p2( facet.P2.x(), facet.P2.y(), facet.P2.z() );

          const BVH_Vec3d P0( p0.X(), p0.Y(), p0.Z() );
          const BVH_Vec3d P1( p1.X(), p1.Y(), p1.Z() );
          const BVH_Vec3d P2( p2.X(), p2.Y(), p2.Z() );

          // Precise test.
          const double hits = intersectTriangle(ray, P0, P1, P2);
          //
          if ( hits != REAL_MAX )
          {
            ++numBounces;
          }
        }

        if ( head < 0 )
        {
          return numBounces;
        }

        node = stack[head--];
      }
    }
  }

  static double squaredDistanceToTriangle(const BVH_Vec3d& P,
                                          const BVH_Vec3d& A,
                                          const BVH_Vec3d& B,
                                          const BVH_Vec3d& C)
  {
    // Special case 1.
    const BVH_Vec3d AB = B - A;
    const BVH_Vec3d AC = C - A;
    const BVH_Vec3d AP = P - A;
    //
    double ABdotAP = AB.Dot(AP);
    double ACdotAP = AC.Dot(AP);
    //
    if ( ABdotAP <= 0.0 && ACdotAP <= 0.0 )
    {
      return AP.Dot(AP);
    }

    // Special case 2.
    const BVH_Vec3d BC = C - B;
    const BVH_Vec3d BP = P - B;
    //
    double BAdotBP = -AB.Dot(BP);
    double BCdotBP =  BC.Dot(BP);
    //
    if ( BAdotBP <= 0.0 && BCdotBP <= 0.0 )
    {
      return BP.Dot(BP);
    }

    // Special case 3.
    const BVH_Vec3d CP = P - C;
    //
    double CBdotCP = -BC.Dot(CP);
    double CAdotCP = -AC.Dot(CP);
    if ( CAdotCP <= 0.0 && CBdotCP <= 0.0 )
    {
      return CP.Dot(CP);
    }

    // Special case 4.
    double ACdotBP = AC.Dot(BP);
    double VC      = ABdotAP*ACdotBP + BAdotBP*ACdotAP;
    //
    if ( VC <= 0.0 && ABdotAP > 0.0 && BAdotBP > 0.0 )
    {
      return ( AP - AB*(ABdotAP/(ABdotAP + BAdotBP)) ).SquareModulus();
    }

    // Special case 5.
    double ABdotCP = AB.Dot(CP);
    double VA      = BAdotBP*CAdotCP - ABdotCP*ACdotBP;
    if ( VA <= 0.0 && BCdotBP > 0.0 && CBdotCP > 0.0 )
    {
      return ( BP - BC*(BCdotBP/(BCdotBP + CBdotCP)) ).SquareModulus();
    }

    // Special case 6.
    double VB = ABdotCP*ACdotAP + ABdotAP*CAdotCP;
    if ( VB <= 0.0 && ACdotAP > 0.0 && CAdotCP > 0.0 )
    {
      return ( AP - AC*(ACdotAP/(ACdotAP + CAdotCP)) ).SquareModulus();
    }

    // General.
    double norm = VA + VB + VC;
    return (P - (A*VA + B*VB + C*VC)/norm).SquareModulus();
  }

  static double squaredDistanceToBox(const BVH_Vec3d& P,
                                     const BVH_Vec3d& boxMin,
                                     const BVH_Vec3d& boxMax)
  {
    double nearestX = std::min( std::max( P.x(), boxMin.x() ), boxMax.x() );
    double nearestY = std::min( std::max( P.y(), boxMin.y() ), boxMax.y() );
    double nearestZ = std::min( std::max( P.z(), boxMin.z() ), boxMax.z() );

    if ( nearestX == P.x() && nearestY == P.y() && nearestZ == P.z() )
      return 0.0;

    nearestX -= P.x();
    nearestY -= P.y();
    nearestZ -= P.z();

    return nearestX*nearestX + nearestY*nearestY + nearestZ*nearestZ;
  }

  static double squaredDistanceToMesh(ModelBvh*        pMesh,
                                      const BVH_Vec3d& P,
                                      const double     upperDist = REAL_MAX)
  {
    const BVH_Tree<double, 3>* pBVH = pMesh != nullptr ? pMesh->BVH().get() : nullptr;
    if ( pBVH == nullptr )
      return REAL_MAX;

    std::pair<int, double> stack[64];
    int head = -1;
    int node =  0; // Root node.

    for ( double minDist2 = upperDist; ; )
    {
      if ( node >= (int) pBVH->NodeInfoBuffer().size() )
        return REAL_MAX;

      BVH_Vec4i data = pBVH->NodeInfoBuffer()[node];

      if ( data.x() == 0 ) // Inner node.
      {
        const double distToLft = squaredDistanceToBox( P,
                                                       pBVH->MinPoint( data.y() ),
                                                       pBVH->MaxPoint( data.y() ) );

        const double distToRgh = squaredDistanceToBox( P,
                                                       pBVH->MinPoint( data.z() ),
                                                       pBVH->MaxPoint( data.z() ) );

        const bool hitLft = distToLft <= minDist2;
        const bool hitRgh = distToRgh <= minDist2;

        if ( hitLft & hitRgh )
        {
          node = (distToLft < distToRgh) ? data.y() : data.z();

          stack[++head] = std::make_pair( distToLft < distToRgh ? data.z() : data.y(),
                                          std::max(distToLft, distToRgh) );
        }
        else
        {
          if ( hitLft | hitRgh)
          {
            node = hitLft ? data.y() : data.z();
          }
          else
          {
            if ( head < 0 )
              return minDist2;

            std::pair<int, double>& entry = stack[head--];

            while ( entry.second > minDist2 )
            {
              if ( head < 0 )
              {
                return minDist2;
              }

              entry = stack[head--];
            }

            node = entry.first;
          }
        }
      }
      else // Leaf node.
      {
        for ( int tidx = data.y(); tidx <= data.z(); ++tidx )
        {
          const ModelBvh::t_facet& facet = pMesh->GetFacet(tidx);

          // Get next facet to test
          const gp_XYZ v0( facet.P0.x(), facet.P0.y(), facet.P0.z() );
          const gp_XYZ v1( facet.P1.x(), facet.P1.y(), facet.P1.z() );
          const gp_XYZ v2( facet.P2.x(), facet.P2.y(), facet.P2.z() );

          const BVH_Vec3d V0( v0.X(), v0.Y(), v0.Z() );
          const BVH_Vec3d V1( v1.X(), v1.Y(), v1.Z() );
          const BVH_Vec3d V2( v2.X(), v2.Y(), v2.Z() );

          const double triDist2 = squaredDistanceToTriangle(P,
                                                            V0,
                                                            V1,
                                                            V2);
          if ( triDist2 < minDist2 )
          {
            minDist2 = triDist2;
          }
        }

        if ( head < 0 )
        {
          return minDist2;
        }

        std::pair<int, double>& entry = stack[head--];
        while ( entry.second > minDist2 )
        {
          if ( head < 0 )
          {
            return minDist2;
          }

          entry = stack[head--];
        }

        node = entry.first;
      }
    }
  }

protected:

  Handle(ModelBvh)   m_facets;   //!< BVH for shape represented with facets.
  int                m_iNumRays; //!< Number of rays to check distance sign.
  mutable BullardRNG m_RNG;      //!< Random number generator.

};

//-----------------------------------------------------------------------------

//! \ingroup ASI_MODELING
//!
//! Point-solid PMC.
class asiAlgo_ClassifyPointSolid
{
public:

  asiAlgo_ClassifyPointSolid(const Handle(Poly_Triangulation)& mesh)
  {
    m_tris = mesh;
    m_bvh  = new ModelBvh(mesh);
    m_dist = new MeshDist(m_bvh);
  }

  bool IsIn(const gp_XYZ& pt, const double tol)
  {
    const double d = m_dist->Eval( pt.X(), pt.Y(), pt.Z() );
    return (d < 0) && (Abs(d) > tol);
  }

  bool IsOn(const gp_XYZ& pt, const double tol)
  {
    const double d = m_dist->Eval( pt.X(), pt.Y(), pt.Z() );
    return (Abs(d) < tol);
  }

  bool IsOut(const gp_XYZ& pt, const double tol)
  {
    const double d = m_dist->Eval( pt.X(), pt.Y(), pt.Z() );
    return (d > 0) && (Abs(d) > tol);
  }

protected:

  Handle(Poly_Triangulation) m_tris;
  Handle(ModelBvh)           m_bvh;
  Handle(MeshDist)           m_dist;
};

#endif
