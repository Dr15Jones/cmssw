#include "Geometry/CaloGeometry/interface/CaloGenericDetId.h"
#include "Geometry/HcalTowerAlgo/interface/CaloTowerGeometry.h"

#include <Math/Transform3D.h>
#include <Math/EulerAngles.h>

typedef CaloCellGeometry::CCGFloat CCGFloat;
typedef CaloCellGeometry::Pt3D Pt3D;
typedef CaloCellGeometry::Pt3DVec Pt3DVec;
typedef CaloCellGeometry::Tr3D Tr3D;

CaloTowerGeometry::CaloTowerGeometry(const CaloTowerTopology* cttopo)
    : m_cttopo(cttopo),
      k_NumberOfCellsForCorners(m_cttopo->sizeForDenseIndexing()),
      k_NumberOfShapes(m_cttopo->lastHFRing()),
      m_cellVec(k_NumberOfCellsForCorners) {}

CaloTowerGeometry::~CaloTowerGeometry() {}

unsigned int CaloTowerGeometry::alignmentTransformIndexLocal(const DetId& id) {
  const CaloGenericDetId gid(id);
  assert(gid.isCaloTower());

  const CaloTowerDetId cid(id);
  const int iea(cid.ietaAbs());
  const unsigned int ip((cid.iphi() - 1) / 4);
  const int izoff((cid.zside() + 1) / 2);
  const unsigned int offset(izoff * 3 * 18);

  return (offset + ip +
          (m_cttopo->firstHFQuadPhiRing() <= iea ? 36 : (m_cttopo->firstHEDoublePhiRing() <= iea ? 18 : 0)));
}

unsigned int CaloTowerGeometry::alignmentTransformIndexGlobal(const DetId& id) { return (unsigned int)DetId::Calo - 1; }

void CaloTowerGeometry::localCorners(Pt3DVec& lc, const CCGFloat* pv, unsigned int i, Pt3D& ref) {
  IdealObliquePrism::localCorners(lc, pv, ref);
}

void CaloTowerGeometry::newCell(
    const GlobalPoint& f1, const GlobalPoint& f2, const GlobalPoint& f3, const CCGFloat* parm, const DetId& detId) {
  const CaloGenericDetId cgid(detId);

  assert(cgid.isCaloTower());

  const CaloTowerDetId cid(detId);

  const unsigned int di(m_cttopo->denseIndex(cid));

  m_cellVec[di] = IdealObliquePrism(f1, cornersMgr(), parm);
  addValidID(detId);
  m_dins.emplace_back(di);
}

CaloCellGeometryPtr CaloTowerGeometry::getGeometryRawPtr(uint32_t index) const {
  // Modify the RawPtr class
  return CaloCellGeometryPtr(m_cellVec.size() <= index || nullptr == m_cellVec[index].param() ? nullptr
                                                                                              : &m_cellVec[index]);
}

void CaloTowerGeometry::getSummary(CaloSubdetectorGeometry::TrVec& tVec,
                                   CaloSubdetectorGeometry::IVec& iVec,
                                   CaloSubdetectorGeometry::DimVec& dVec,
                                   CaloSubdetectorGeometry::IVec& dinsVec) const {
  tVec.reserve(numberOfCellsForCorners() * numberOfTransformParms());
  iVec.reserve(numberOfShapes() == 1 ? 1 : numberOfCellsForCorners());
  dVec.reserve(numberOfShapes() * numberOfParametersPerShape());
  dinsVec.reserve(numberOfCellsForCorners());

  for (const auto& pv : parVecVec()) {
    for (float iv : pv) {
      dVec.emplace_back(iv);
    }
  }

  for (unsigned int i(0); i < numberOfCellsForCorners(); ++i) {
    Tr3D tr;
    auto ptr(cellGeomPtr(i));

    if (nullptr != ptr) {
      dinsVec.emplace_back(i);

      ptr->getTransform(tr, (Pt3DVec*)nullptr);

      if (Tr3D() == tr) {  // for preshower there is no rotation
        const GlobalPoint& gp(ptr->getPosition());
        tr = HepGeom::Translate3D(gp.x(), gp.y(), gp.z());
      }

      const CLHEP::Hep3Vector tt(tr.getTranslation());
      tVec.emplace_back(tt.x());
      tVec.emplace_back(tt.y());
      tVec.emplace_back(tt.z());
      if (6 == numberOfTransformParms()) {
        const CLHEP::HepRotation rr(tr.getRotation());
        const ROOT::Math::Transform3D rtr(
            rr.xx(), rr.xy(), rr.xz(), tt.x(), rr.yx(), rr.yy(), rr.yz(), tt.y(), rr.zx(), rr.zy(), rr.zz(), tt.z());
        ROOT::Math::EulerAngles ea;
        rtr.GetRotation(ea);
        tVec.emplace_back(ea.Phi());
        tVec.emplace_back(ea.Theta());
        tVec.emplace_back(ea.Psi());
      }

      const CCGFloat* par(ptr->param());

      unsigned int ishape(9999);
      for (unsigned int ivv(0); ivv != parVecVec().size(); ++ivv) {
        bool ok(true);
        const CCGFloat* pv(&(*parVecVec()[ivv].begin()));
        for (unsigned int k(0); k != numberOfParametersPerShape(); ++k) {
          ok = ok && (fabs(par[k] - pv[k]) < 1.e-6);
        }
        if (ok) {
          ishape = ivv;
          break;
        }
      }
      assert(9999 != ishape);

      const unsigned int nn((numberOfShapes() == 1) ? (unsigned int)1 : m_dins.size());
      if (iVec.size() < nn)
        iVec.emplace_back(ishape);
    }
  }
}
