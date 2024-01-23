#ifndef CSCRecHit_CSCSegment_h
#define CSCRecHit_CSCSegment_h

/** \class CSCSegment
 *  Describes a reconstructed track segment in the 6 layers of a CSC chamber. 
 *  This is 4-dimensional since it has an origin (x,y) and a direction (x,y)
 *  in the local coordinate system of the chamber.
 *
 *  \author Matteo Sani
 *  \author Rick Wilkinson
 *  \author Tim Cox
 */

#include <DataFormats/TrackingRecHit/interface/RecSegment.h>
#include <DataFormats/CSCRecHit/interface/CSCRecHit2DCollection.h>

#include <iosfwd>

class CSCDetId;

class CSCSegment;

namespace cscsegment {
  class MatchedSegment {
  public:

    MatchedSegment(LocalPoint const& iOrigin,
                   LocalError const& iOriginError,
                   LocalVector const& iDirection,
                   LocalError const& iDirectionError) :
    theOrigin(iOrigin),
      theLocalDirection(iDirection),
      theOriginError(iOriginError),
      theDirectionError(iDirectionError) {}

    MatchedSegment() = default;
    MatchedSegment(const MatchedSegment&) = default;
    MatchedSegment(MatchedSegment&&) = default;
    MatchedSegment& operator=(const MatchedSegment&) = default;
    MatchedSegment& operator=(MatchedSegment&&) = default;

    MatchedSegment( const CSCSegment&);
    
    LocalPoint const& localPosition() const { return theOrigin; }
    LocalError const& localPositionError() const { return theOriginError; }
    
    LocalVector const& localDirection() const { return theLocalDirection; }
    LocalError localDirectionError() const { return theDirectionError; }

  private:
  LocalPoint theOrigin;             // in chamber frame - the GeomDet local coordinate system
  LocalVector theLocalDirection;    // in chamber frame - the GeomDet local coordinate system
  LocalError theOriginError;
  LocalError theDirectionError;
  };
}

class CSCSegment final : public RecSegment {
public:
  /// Default constructor
  CSCSegment() : theChi2(0.) {}

  /// Constructor
  CSCSegment(const std::vector<const CSCRecHit2D*>& proto_segment,
             LocalPoint origin,
             LocalVector direction,
             const AlgebraicSymMatrix& errors,
             double chi2);

  /// Destructor
  ~CSCSegment() override;

  //--- Base class interface
  CSCSegment* clone() const override { return new CSCSegment(*this); }

  LocalPoint localPosition() const override { return theOrigin; }
  LocalError localPositionError() const override;

  LocalVector localDirection() const override { return theLocalDirection; }
  LocalError localDirectionError() const override;

  /// Parameters of the segment, for the track fit in the order (dx/dz, dy/dz, x, y )
  AlgebraicVector parameters() const override;

  /// Covariance matrix of parameters()
  AlgebraicSymMatrix parametersError() const override { return theCovMatrix; }

  /// The projection matrix relates the trajectory state parameters to the segment parameters().
  AlgebraicMatrix projectionMatrix() const override;

  std::vector<const TrackingRecHit*> recHits() const override;

  std::vector<TrackingRecHit*> recHits() override;

  double chi2() const override { return theChi2; };

  int dimension() const override { return 4; }

  int degreesOfFreedom() const override { return 2 * nRecHits() - 4; }

  //--- Extension of the interface

  const std::vector<CSCRecHit2D>& specificRecHits() const { return theCSCRecHits; }

  int nRecHits() const { return theCSCRecHits.size(); }

  CSCDetId cscDetId() const { return geographicalId(); }

  void setDuplicateSegments(std::vector<CSCSegment*>& duplicates);

  bool isME11a_duplicate() const { return (!theDuplicateSegments.empty() ? true : false); }
  // a copy of the duplicated segments (ME1/1a only)
  const std::vector<cscsegment::MatchedSegment>& duplicateSegments() const { return theDuplicateSegments; }

  bool testSharesAllInSpecificRecHits(const std::vector<CSCRecHit2D>& specificRecHits_1,
                                      const std::vector<CSCRecHit2D>& specificRecHits_2,
                                      CSCRecHit2D::SharedInputType) const;

  //bool sharesRecHits(CSCSegment  & anotherSegment, CSCRecHit2D::SharedInputType);
  // checks if ALL the rechits share the specific input (allWires, allStrips or all)
  bool sharesRecHits(const CSCSegment& anotherSegment, CSCRecHit2D::SharedInputType sharesInput) const;
  // checks if ALL the rechits share SOME wire AND SOME strip input
  bool sharesRecHits(const CSCSegment& anotherSegment) const;

  float time() const;

  void print() const;

private:
  std::vector<CSCRecHit2D> theCSCRecHits;
  LocalPoint theOrigin;             // in chamber frame - the GeomDet local coordinate system
  LocalVector theLocalDirection;    // in chamber frame - the GeomDet local coordinate system
  AlgebraicSymMatrix theCovMatrix;  // the covariance matrix
  double theChi2;
  std::vector<cscsegment::MatchedSegment> theDuplicateSegments;  // ME1/1a only
};

inline cscsegment::MatchedSegment::MatchedSegment( const CSCSegment& iSeg):
  theOrigin(iSeg.localPosition()),
  theLocalDirection(iSeg.localDirection()),
  theOriginError(iSeg.localPositionError()),
  theDirectionError(iSeg.localDirectionError()) {}

std::ostream& operator<<(std::ostream& os, const CSCSegment& seg);

#endif  // CSCRecHit_CSCSegment_h
