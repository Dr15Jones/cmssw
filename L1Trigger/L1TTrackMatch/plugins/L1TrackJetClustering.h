#ifndef L1Trigger_L1TTrackMatch_L1TrackJetClustering_HH
#define L1Trigger_L1TTrackMatch_L1TrackJetClustering_HH
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <string>
#include <cstdlib>
#include "DataFormats/L1Trigger/interface/TkJetWord.h"
#include "DataFormats/L1TrackTrigger/interface/TTTrack_TrackWord.h"
#include "DataFormats/L1TrackTrigger/interface/TTTypes.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

namespace l1ttrackjet {
  //For precision studies
  const unsigned int PT_INTPART_BITS{9};
  const unsigned int ETA_INTPART_BITS{3};
  const unsigned int kExtraGlobalPhiBit{4};

  static constexpr int kEtaWordLength = 15;
  static constexpr int kPhiWordLength = 12;

  //Constants used for jet clustering in eta direction
  static constexpr int kThirteenBitMask = 0b1111111111111;
  static constexpr int kEtaFineBinEdge1 = 0b0011001100110;
  static constexpr int kEtaFineBinEdge2 = 0b0110011001100;
  static constexpr int kEtaFineBinEdge3 = 0b1001100110010;
  static constexpr int kEtaFineBinEdge4 = 0b1100110011000;

  //Constants used for jet clustering in phi direction
  static constexpr int kTwelveBitMask = 0b011111111111;
  static constexpr int kPhiBinHalfWidth = 0b000100101111;
  static constexpr int kNumPhiBins = 27;
  static constexpr int kPhiBinZeroOffset = 12;  // phi bin zero offset between firmware and emulator

  typedef ap_ufixed<TTTrack_TrackWord::TrackBitWidths::kRinvSize - 1, PT_INTPART_BITS, AP_TRN, AP_SAT> pt_intern;
  typedef ap_fixed<TTTrack_TrackWord::TrackBitWidths::kTanlSize, ETA_INTPART_BITS, AP_TRN, AP_SAT> glbeta_intern;
  typedef ap_int<TTTrack_TrackWord::TrackBitWidths::kPhiSize + kExtraGlobalPhiBit> glbphi_intern;
  typedef ap_int<TTTrack_TrackWord::TrackBitWidths::kZ0Size> z0_intern;  // 40cm / 0.1
  typedef ap_uint<TTTrack_TrackWord::TrackBitWidths::kD0Size> d0_intern;

  inline const unsigned int DoubleToBit(double value, unsigned int maxBits, double step) {
    unsigned int digitized_value = std::floor(std::abs(value) / step);
    unsigned int digitized_maximum = (1 << (maxBits - 1)) - 1;  // The remove 1 bit from nBits to account for the sign
    if (digitized_value > digitized_maximum)
      digitized_value = digitized_maximum;
    if (value < 0)
      digitized_value = (1 << maxBits) - digitized_value;  // two's complement encoding
    return digitized_value;
  }
  inline const double BitToDouble(unsigned int bits, unsigned int maxBits, double step) {
    int isign = 1;
    unsigned int digitized_maximum = (1 << maxBits) - 1;
    if (bits & (1 << (maxBits - 1))) {  // check the sign
      isign = -1;
      bits = (1 << (maxBits + 1)) - bits;
    }
    return (double(bits & digitized_maximum) + 0.5) * step * isign;
  }

  // eta/phi clusters - simulation
  struct EtaPhiBin {
    float pTtot;
    int ntracks;
    int nxtracks;
    bool used;
    float phi;  //average phi value (halfway b/t min and max)
    float eta;  //average eta value
    std::vector<unsigned int> trackidx;
  };
  // z bin struct - simulation (used if z bin are many)
  struct MaxZBin {
    int znum;    //Numbered from 0 to nzbins (16, 32, or 64) in order
    int nclust;  //number of clusters in this bin
    float zbincenter;
    std::vector<EtaPhiBin> clusters;  //list of all the clusters in this bin
    float ht;                         //sum of all cluster pTs--only the zbin with the maximum ht is stored
  };

  // eta/phi clusters - emulation
  struct TrackJetEmulationEtaPhiBin {
    pt_intern pTtot;
    l1t::TkJetWord::nt_t ntracks;
    l1t::TkJetWord::nx_t nxtracks;
    bool used;
    glbphi_intern phi;  //average phi value (halfway b/t min and max)
    glbeta_intern eta;  //average eta value
    std::vector<unsigned int> trackidx;
  };

  // z bin struct - emulation (used if z bin are many)
  struct TrackJetEmulationMaxZBin {
    int znum;    //Numbered from 0 to nzbins (16, 32, or 64) in order
    int nclust;  //number of clusters in this bin
    z0_intern zbincenter;
    std::vector<TrackJetEmulationEtaPhiBin> clusters;  //list of all the clusters in this bin
    pt_intern ht;  //sum of all cluster pTs--only the zbin with the maximum ht is stored
  };

  // track quality cuts
  inline bool TrackQualitySelection(int trk_nstub,
                                    double trk_chi2,
                                    double trk_bendchi2,
                                    double nStubs4PromptBend_,
                                    double nStubs5PromptBend_,
                                    double nStubs4PromptChi2_,
                                    double nStubs5PromptChi2_,
                                    double nStubs4DisplacedBend_,
                                    double nStubs5DisplacedBend_,
                                    double nStubs4DisplacedChi2_,
                                    double nStubs5DisplacedChi2_,
                                    bool displaced_) {
    bool PassQuality = false;
    if (!displaced_) {
      if (trk_nstub == 4 && trk_bendchi2 < nStubs4PromptBend_ &&
          trk_chi2 < nStubs4PromptChi2_)  // 4 stubs are the lowest track quality and have different cuts
        PassQuality = true;
      if (trk_nstub > 4 && trk_bendchi2 < nStubs5PromptBend_ &&
          trk_chi2 < nStubs5PromptChi2_)  // above 4 stubs diffent selection imposed (genrally looser)
        PassQuality = true;
    } else {
      if (trk_nstub == 4 && trk_bendchi2 < nStubs4DisplacedBend_ &&
          trk_chi2 < nStubs4DisplacedChi2_)  // 4 stubs are the lowest track quality and have different cuts
        PassQuality = true;
      if (trk_nstub > 4 && trk_bendchi2 < nStubs5DisplacedBend_ &&
          trk_chi2 < nStubs5DisplacedChi2_)  // above 4 stubs diffent selection imposed (genrally looser)
        PassQuality = true;
    }
    return PassQuality;
  }

  // Eta binning following the hardware logic
  inline unsigned int eta_bin_firmwareStyle(int eta_word) {
    //Function that reads in 15-bit eta word (in two's complement) and returns the index of eta bin in which it belongs
    //Logic follows exactly according to the firmware
    //We will first determine if eta is pos/neg from the first bit. Each half of the grid is then split into four coarse bins. Bits 2&3 determine which coarse bin to assign
    //The coarse bins are split into 5 fine bins, final 13 bits determine which of these coarse bins this track needs to assign
    int eta_coarse = 0;
    int eta_fine = 0;

    if (eta_word & (1 << kEtaWordLength)) {  //If eta is negative (first/leftmost bit is 1)
      //Second and third bits contain information about which of four coarse bins
      eta_coarse = 5 * ((eta_word & (3 << (kEtaWordLength - 2))) >> (kEtaWordLength - 2));
    } else {  //else eta is positive (first/leftmost bit is 0)
      eta_coarse = 5 * (4 + ((eta_word & (3 << (kEtaWordLength - 2))) >> (kEtaWordLength - 2)));
    }

    //Now get the fine bin index. The numbers correspond to the decimal representation of fine bin edges in binary
    int j = eta_word & kThirteenBitMask;
    if (j < kEtaFineBinEdge1)
      eta_fine = 0;
    else if (j < kEtaFineBinEdge2)
      eta_fine = 1;
    else if (j < kEtaFineBinEdge3)
      eta_fine = 2;
    else if (j < kEtaFineBinEdge4)
      eta_fine = 3;
    else
      eta_fine = 4;

    //Final eta bin is coarse bin index + fine bin index, subtract 8 to make eta_bin at eta=-2.4 have index=0
    int eta_ = (eta_coarse + eta_fine) - 8;
    return eta_;
  }

  // Phi binning following the hardware logic
  inline unsigned int phi_bin_firmwareStyle(int phi_sector_raw, int phi_word) {
    //Function that reads in decimal integers phi_sector_raw and phi_word and returns the index of phi bin in which it belongs
    //phi_sector_raw is integer 0-8 correspoding to one of 9 phi sectors
    //phi_word is 12 bit word representing the phi value measured w.r.t the center of the sector

    int phi_coarse = 3 * phi_sector_raw;  //phi_coarse is index of phi coarse binning (sector edges)
    int phi_fine = 0;  //phi_fine is index of fine bin inside sectors. Each sector contains 3 fine bins

    //Determine fine bin. First bit is sign, next 11 bits determine fine bin
    //303 is distance from 0 to first fine bin edge
    //2047 is eleven 1's, use the &2047 to extract leftmost 11 bits.

    //The allowed range for phi goes further than the edges of bin 0 or 2 (bit value 909). There's an apparent risk of phi being > 909, however this will always mean the track is in the next link (i.e. track beyond bin 2 in this link means track is actually in bin 0 of adjacent link)

    if (phi_word & (1 << (kPhiWordLength - 1))) {  //if phi is negative (first bit 1)
      //Since negative, we 'flip' the phi word, then check if it is in fine bin 0 or 1
      if ((kTwelveBitMask - (phi_word & kTwelveBitMask)) > kPhiBinHalfWidth) {
        phi_fine = 0;
      } else if ((kTwelveBitMask - (phi_word & kTwelveBitMask)) < kPhiBinHalfWidth) {
        phi_fine = 1;
      }
    } else {  //else phi is positive (first bit 0)
      //positive phi, no 'flip' necessary. Just check if in  fine bin 1 or 2
      if ((phi_word & kTwelveBitMask) < kPhiBinHalfWidth) {
        phi_fine = 1;
      } else if ((phi_word & kTwelveBitMask) > kPhiBinHalfWidth) {
        phi_fine = 2;
      }
    }

    // Final operation is a shift by pi (half a grid) to make bin at index=0 at -pi
    int phi_bin_ = (phi_coarse + phi_fine + kPhiBinZeroOffset) % kNumPhiBins;

    return phi_bin_;
  }

  // L1 clustering (in eta)
  template <typename T, typename Pt, typename Eta, typename Phi>
  inline std::vector<T> L1_clustering(T *phislice, int etaBins_, Eta etaStep_) {
    std::vector<T> clusters;
    // Find eta bin with maxpT, make center of cluster, add neighbors if not already used
    int nclust = 0;

    // get tracks in eta bins in increasing eta order
    for (int etabin = 0; etabin < etaBins_; ++etabin) {
      Pt my_pt = 0;
      Pt previousbin_pt = 0;
      Pt nextbin_pt = 0;
      Pt nextbin2_pt = 0;

      // skip (already) used tracks
      if (phislice[etabin].used)
        continue;

      my_pt = phislice[etabin].pTtot;
      if (my_pt == 0)
        continue;

      //get previous bin pT
      if (etabin > 0 && !phislice[etabin - 1].used)
        previousbin_pt = phislice[etabin - 1].pTtot;

      // get next bins pt
      if (etabin < etaBins_ - 1 && !phislice[etabin + 1].used) {
        nextbin_pt = phislice[etabin + 1].pTtot;
        if (etabin < etaBins_ - 2 && !phislice[etabin + 2].used) {
          nextbin2_pt = phislice[etabin + 2].pTtot;
        }
      }
      // check if pT of current cluster is higher than neighbors
      if (my_pt < previousbin_pt || my_pt <= nextbin_pt) {
        // if unused pT in the left neighbor, spit it out as a cluster
        if (previousbin_pt > 0) {
          clusters.push_back(phislice[etabin - 1]);
          phislice[etabin - 1].used = true;
          nclust++;
        }
        continue;  //if it is not the local max pT skip
      }
      // here reach only unused local max clusters
      clusters.push_back(phislice[etabin]);
      phislice[etabin].used = true;  //if current bin a cluster
      if (previousbin_pt > 0) {
        clusters[nclust].pTtot += previousbin_pt;
        clusters[nclust].ntracks += phislice[etabin - 1].ntracks;
        clusters[nclust].nxtracks += phislice[etabin - 1].nxtracks;
        for (unsigned int itrk = 0; itrk < phislice[etabin - 1].trackidx.size(); itrk++)
          clusters[nclust].trackidx.push_back(phislice[etabin - 1].trackidx[itrk]);
      }

      if (my_pt >= nextbin2_pt && nextbin_pt > 0) {
        clusters[nclust].pTtot += nextbin_pt;
        clusters[nclust].ntracks += phislice[etabin + 1].ntracks;
        clusters[nclust].nxtracks += phislice[etabin + 1].nxtracks;
        for (unsigned int itrk = 0; itrk < phislice[etabin + 1].trackidx.size(); itrk++)
          clusters[nclust].trackidx.push_back(phislice[etabin + 1].trackidx[itrk]);
        phislice[etabin + 1].used = true;
      }

      nclust++;

    }  // for each etabin

    // Merge close-by clusters
    for (int m = 0; m < nclust - 1; ++m) {
      if (((clusters[m + 1].eta - clusters[m].eta) < (3 * etaStep_) / 2) &&
          (-(3 * etaStep_) / 2 < (clusters[m + 1].eta - clusters[m].eta))) {
        if (clusters[m + 1].pTtot > clusters[m].pTtot) {
          clusters[m].eta = clusters[m + 1].eta;
        }
        clusters[m].pTtot += clusters[m + 1].pTtot;
        clusters[m].ntracks += clusters[m + 1].ntracks;    // total ntrk
        clusters[m].nxtracks += clusters[m + 1].nxtracks;  // total ndisp
        for (unsigned int itrk = 0; itrk < clusters[m + 1].trackidx.size(); itrk++)
          clusters[m].trackidx.push_back(clusters[m + 1].trackidx[itrk]);

        // if remove the merged cluster - all the others must be closer to 0
        for (int m1 = m + 1; m1 < nclust - 1; ++m1)
          clusters[m1] = clusters[m1 + 1];

        clusters.erase(clusters.begin() + nclust);
        nclust--;
      }  // end if for cluster merging
    }  // end for (m) loop

    return clusters;
  }

  // Fill L2 clusters (helper function)
  template <typename T, typename Pt>
  inline void Fill_L2Cluster(T &bin, Pt pt, int ntrk, int ndtrk, std::vector<unsigned int> trkidx) {
    bin.pTtot += pt;
    bin.ntracks += ntrk;
    bin.nxtracks += ndtrk;
    for (unsigned int itrk = 0; itrk < trkidx.size(); itrk++)
      bin.trackidx.push_back(trkidx[itrk]);
  }

  inline glbphi_intern DPhi(glbphi_intern phi1, glbphi_intern phi2) {
    glbphi_intern x = phi1 - phi2;
    if (x >= DoubleToBit(
                 M_PI, TTTrack_TrackWord::TrackBitWidths::kPhiSize + kExtraGlobalPhiBit, TTTrack_TrackWord::stepPhi0))
      x -= DoubleToBit(
          2 * M_PI, TTTrack_TrackWord::TrackBitWidths::kPhiSize + kExtraGlobalPhiBit, TTTrack_TrackWord::stepPhi0);
    if (x < DoubleToBit(-1 * M_PI,
                        TTTrack_TrackWord::TrackBitWidths::kPhiSize + kExtraGlobalPhiBit,
                        TTTrack_TrackWord::stepPhi0))
      x += DoubleToBit(
          2 * M_PI, TTTrack_TrackWord::TrackBitWidths::kPhiSize + kExtraGlobalPhiBit, TTTrack_TrackWord::stepPhi0);
    return x;
  }

  inline float DPhi(float phi1, float phi2) {
    float x = phi1 - phi2;
    if (x >= M_PI)
      x -= 2 * M_PI;
    if (x < -1 * M_PI)
      x += 2 * M_PI;
    return x;
  }

  // L2 clustering (in phi)
  template <typename T, typename Pt, typename Eta, typename Phi>
  inline std::vector<T> L2_clustering(std::vector<std::vector<T> > &L1clusters,
                                      int phiBins_,
                                      Phi phiStep_,
                                      Eta etaStep_) {
    std::vector<T> clusters;
    for (int phibin = 0; phibin < phiBins_; ++phibin) {  //Find eta-phibin with highest pT
      if (L1clusters[phibin].empty())
        continue;

      // sort L1 clusters max -> min
      sort(L1clusters[phibin].begin(), L1clusters[phibin].end(), [](T &a, T &b) { return a.pTtot > b.pTtot; });

      for (unsigned int imax = 0; imax < L1clusters[phibin].size(); ++imax) {
        if (L1clusters[phibin][imax].used)
          continue;
        Pt pt_current = L1clusters[phibin][imax].pTtot;  //current cluster (pt0)
        Pt pt_next = 0;                                  // next phi bin (pt1)
        Pt pt_next2 = 0;                                 // next to next phi bin2 (pt2)
        int trk1 = 0;
        int trk2 = 0;
        int tdtrk1 = 0;
        int tdtrk2 = 0;
        std::vector<unsigned int> trkidx1;
        std::vector<unsigned int> trkidx2;
        clusters.push_back(L1clusters[phibin][imax]);

        L1clusters[phibin][imax].used = true;

        // if we are in the last phi bin, dont check phi+1 phi+2
        if (phibin == phiBins_ - 1)
          continue;

        std::vector<unsigned int> used_already;  //keep phi+1 clusters that have been used
        for (unsigned int icluster = 0; icluster < L1clusters[phibin + 1].size(); ++icluster) {
          if (L1clusters[phibin + 1][icluster].used)
            continue;

          if (((L1clusters[phibin + 1][icluster].eta - L1clusters[phibin][imax].eta) > (3 * etaStep_) / 2) ||
              ((L1clusters[phibin + 1][icluster].eta - L1clusters[phibin][imax].eta) < -(3 * etaStep_) / 2))
            continue;

          pt_next += L1clusters[phibin + 1][icluster].pTtot;
          trk1 += L1clusters[phibin + 1][icluster].ntracks;
          tdtrk1 += L1clusters[phibin + 1][icluster].nxtracks;

          for (unsigned int itrk = 0; itrk < L1clusters[phibin + 1][icluster].trackidx.size(); itrk++)
            trkidx1.push_back(L1clusters[phibin + 1][icluster].trackidx[itrk]);
          used_already.push_back(icluster);
        }

        if (pt_next < pt_current) {  // if pt1<pt1, merge both clusters
          Fill_L2Cluster<T, Pt>(clusters[clusters.size() - 1], pt_next, trk1, tdtrk1, trkidx1);
          for (unsigned int iused : used_already)
            L1clusters[phibin + 1][iused].used = true;
          continue;
        }
        // if phi = next to last bin there is no "next to next"
        if (phibin == phiBins_ - 2) {
          Fill_L2Cluster<T, Pt>(clusters[clusters.size() - 1], pt_next, trk1, tdtrk1, trkidx1);
          clusters[clusters.size() - 1].phi = L1clusters[phibin + 1][used_already[0]].phi;
          for (unsigned int iused : used_already)
            L1clusters[phibin + 1][iused].used = true;
          continue;
        }
        std::vector<int> used_already2;  //keep used clusters in phi+2
        for (unsigned int icluster = 0; icluster < L1clusters[phibin + 2].size(); ++icluster) {
          if (L1clusters[phibin + 2][icluster].used)
            continue;
          if (((L1clusters[phibin + 2][icluster].eta - L1clusters[phibin][imax].eta) > (3 * etaStep_) / 2) ||
              ((L1clusters[phibin + 2][icluster].eta - L1clusters[phibin][imax].eta) < -(3 * etaStep_) / 2))
            continue;
          pt_next2 += L1clusters[phibin + 2][icluster].pTtot;
          trk2 += L1clusters[phibin + 2][icluster].ntracks;
          tdtrk2 += L1clusters[phibin + 2][icluster].nxtracks;

          for (unsigned int itrk = 0; itrk < L1clusters[phibin + 2][icluster].trackidx.size(); itrk++)
            trkidx2.push_back(L1clusters[phibin + 2][icluster].trackidx[itrk]);
          used_already2.push_back(icluster);
        }
        if (pt_next2 < pt_next) {
          std::vector<unsigned int> trkidx_both;
          trkidx_both.reserve(trkidx1.size() + trkidx2.size());
          trkidx_both.insert(trkidx_both.end(), trkidx1.begin(), trkidx1.end());
          trkidx_both.insert(trkidx_both.end(), trkidx2.begin(), trkidx2.end());
          Fill_L2Cluster<T, Pt>(
              clusters[clusters.size() - 1], pt_next + pt_next2, trk1 + trk2, tdtrk1 + tdtrk2, trkidx_both);
          clusters[clusters.size() - 1].phi = L1clusters[phibin + 1][used_already[0]].phi;
          for (unsigned int iused : used_already)
            L1clusters[phibin + 1][iused].used = true;
          for (unsigned int iused : used_already2)
            L1clusters[phibin + 2][iused].used = true;
        }
      }  // for each L1 cluster
    }  // for each phibin

    int nclust = clusters.size();

    // merge close-by clusters
    for (int m = 0; m < nclust - 1; ++m) {
      for (int n = m + 1; n < nclust; ++n) {
        if (clusters[n].eta != clusters[m].eta)
          continue;
        if ((DPhi(clusters[n].phi, clusters[m].phi) > (3 * phiStep_) / 2) ||
            (DPhi(clusters[n].phi, clusters[m].phi) < -(3 * phiStep_) / 2))
          continue;
        if (clusters[n].pTtot > clusters[m].pTtot)
          clusters[m].phi = clusters[n].phi;
        clusters[m].pTtot += clusters[n].pTtot;
        clusters[m].ntracks += clusters[n].ntracks;
        clusters[m].nxtracks += clusters[n].nxtracks;
        for (unsigned int itrk = 0; itrk < clusters[n].trackidx.size(); itrk++)
          clusters[m].trackidx.push_back(clusters[n].trackidx[itrk]);
        for (int m1 = n; m1 < nclust - 1; ++m1)
          clusters[m1] = clusters[m1 + 1];
        clusters.erase(clusters.begin() + nclust);

        nclust--;
      }  // end of n-loop
    }  // end of m-loop
    return clusters;
  }
}  // namespace l1ttrackjet
#endif
