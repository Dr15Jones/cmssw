#ifndef RecoLocalTracker_SiPixelClusterizer_alpaka_PixelClustering_h
#define RecoLocalTracker_SiPixelClusterizer_alpaka_PixelClustering_h

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <type_traits>

#include <alpaka/alpaka.hpp>

#include "DataFormats/SiPixelClusterSoA/interface/ClusteringConstants.h"
#include "FWCore/Utilities/interface/DeviceGlobal.h"
#include "FWCore/Utilities/interface/HostDeviceConstant.h"
#include "Geometry/CommonTopologies/interface/SimplePixelTopology.h"
#include "HeterogeneousCore/AlpakaInterface/interface/HistoContainer.h"
#include "HeterogeneousCore/AlpakaInterface/interface/SimpleVector.h"
#include "HeterogeneousCore/AlpakaInterface/interface/config.h"
#include "HeterogeneousCore/AlpakaInterface/interface/warpsize.h"

//#define GPU_DEBUG

// TODO move to HeterogeneousCore/AlpakaInterface or upstream to alpaka
template <typename TAcc, typename T>
ALPAKA_FN_ACC inline T atomicLoadFromShared(TAcc const& acc [[maybe_unused]], T* arg) {
#if defined(ALPAKA_ACC_GPU_CUDA_ENABLED) or defined(ALPAKA_ACC_GPU_HIP_ENABLED)
  // GPU backend, use a volatile read to force a non-cached acess
  return *reinterpret_cast<volatile T*>(arg);
#elif defined(ALPAKA_ACC_SYCL_ENABLED)
  // SYCL backend, use an atomic load
  sycl::atomic_ref<uint32_t,
                   sycl::memory_order::relaxed,
                   sycl::memory_scope::work_group,
                   sycl::access::address_space::local_space>
      ref{*arg};
  return ref.load();
#elif defined(ALPAKA_ACC_CPU_B_SEQ_T_THREADS_ENABLED) or defined(ALPAKA_ACC_CPU_B_SEQ_T_OMP2_ENABLED)
  // CPU backend with multiple threads per block, use an atomic load
  std::atomic_ref<uint32_t> ref{*arg};
  return ref.load();
#elif defined(ALPAKA_ACC_CPU_B_SEQ_T_SEQ_ENABLED) or defined(ALPAKA_ACC_CPU_B_TBB_T_SEQ_ENABLED) or \
    defined(ALPAKA_ACC_CPU_B_OMP2_T_SEQ_ENABLED)
  // CPU backend with one thread per block, use a standard read
  return *arg;
#else
#error "Unsupported alpaka backend"
  return T{};
#endif
}

namespace ALPAKA_ACCELERATOR_NAMESPACE::pixelClustering {

#ifdef GPU_DEBUG
  DEVICE_GLOBAL uint32_t gMaxHit = 0;
#endif

  namespace pixelStatus {
    // Phase-1 pixel modules
    constexpr uint32_t pixelSizeX = pixelTopology::Phase1::numRowsInModule;  // 2 x 80 = 160
    constexpr uint32_t pixelSizeY = pixelTopology::Phase1::numColsInModule;  // 8 x 52 = 416

    enum Status : uint32_t { kEmpty = 0x00, kFound = 0x01, kDuplicate = 0x03 };

    // 2-bit per pixel Status packed in 32-bit words
    constexpr uint32_t bits = 2;
    constexpr uint32_t mask = (0x01 << bits) - 1;
    constexpr uint32_t valuesPerWord = sizeof(uint32_t) * 8 / bits;     // 16 values per 32-bit word
    constexpr uint32_t size = pixelSizeX * pixelSizeY / valuesPerWord;  // 160 x 416 / 16 = 4160 32-bit words

    ALPAKA_FN_ACC ALPAKA_FN_INLINE constexpr uint32_t getIndex(uint16_t x, uint16_t y) {
      return (pixelSizeX * y + x) / valuesPerWord;
    }

    ALPAKA_FN_ACC ALPAKA_FN_INLINE constexpr uint32_t getShift(uint16_t x, uint16_t y) {
      return (x % valuesPerWord) * bits;
    }

    // Return the current status of a pixel based on its coordinates.
    ALPAKA_FN_ACC ALPAKA_FN_INLINE constexpr Status getStatus(uint32_t const* __restrict__ status,
                                                              uint16_t x,
                                                              uint16_t y) {
      uint32_t index = getIndex(x, y);
      uint32_t shift = getShift(x, y);
      return Status{(status[index] >> shift) & mask};
    }

    // Check whether a pixel at the given coordinates has been marked as duplicate.
    ALPAKA_FN_ACC ALPAKA_FN_INLINE constexpr bool isDuplicate(uint32_t const* __restrict__ status,
                                                              uint16_t x,
                                                              uint16_t y) {
      return getStatus(status, x, y) == kDuplicate;
    }

    // Record a pixel at the given coordinates and return the updated status.
    ALPAKA_FN_ACC ALPAKA_FN_INLINE Status promote(Acc1D const& acc,
                                                  uint32_t* status,
                                                  const uint16_t x,
                                                  const uint16_t y) {
      uint32_t index = getIndex(x, y);
      uint32_t shift = getShift(x, y);
      uint32_t old_word = atomicLoadFromShared(acc, status + index);
      uint32_t expected;
      Status new_status;
      do {
        expected = old_word;
        Status old_status{(old_word >> shift) & mask};
        if (kDuplicate == old_status) {
          // this pixel has already been marked as duplicate
          return kDuplicate;
        }
        new_status = (kEmpty == old_status) ? kFound : kDuplicate;
        uint32_t new_word = old_word | (static_cast<uint32_t>(new_status) << shift);
        old_word = alpaka::atomicCas(acc, &status[index], expected, new_word, alpaka::hierarchy::Threads{});
      } while (expected != old_word);
      return new_status;
    }

  }  // namespace pixelStatus

  template <typename TrackerTraits>
  struct CountModules {
    ALPAKA_FN_ACC void operator()(Acc1D const& acc,
                                  SiPixelDigisSoAView digi_view,
                                  SiPixelClustersSoAView clus_view,
                                  const unsigned int numElements) const {
      // Make sure the atomicInc below does not overflow
      static_assert(TrackerTraits::numberOfModules < ::pixelClustering::maxNumModules);

#ifdef GPU_DEBUG
      if (cms::alpakatools::once_per_grid(acc)) {
        printf("Starting to count modules to set module starts:");
      }
#endif
      for (int32_t i : cms::alpakatools::uniform_elements(acc, numElements)) {
        // Initialise each pixel with a cluster id equal to its index
        digi_view[i].clus() = i;
        if (::pixelClustering::invalidModuleId == digi_view[i].moduleId())
          continue;

        int32_t j = i - 1;
        while (j >= 0 and digi_view[j].moduleId() == ::pixelClustering::invalidModuleId)
          --j;
        if (j < 0 or digi_view[j].moduleId() != digi_view[i].moduleId()) {
          // Found a module boundary: count the number of modules in  clus_view[0].moduleStart()
          auto loc = alpaka::atomicInc(acc,
                                       &clus_view[0].moduleStart(),
                                       static_cast<uint32_t>(::pixelClustering::maxNumModules),
                                       alpaka::hierarchy::Blocks{});
          ALPAKA_ASSERT_ACC(loc < TrackerTraits::numberOfModules);
#ifdef GPU_DEBUG
          printf("> New module (no. %d) found at digi %d \n", loc, i);
#endif
          clus_view[loc + 1].moduleStart() = i;
        }
      }
    }
  };

  template <typename TrackerTraits>
  struct FindClus {
    // assume that we can cover the whole module with up to 16 blockDimension-wide iterations
    static constexpr uint32_t maxIterGPU = 16;

    // this must be larger than maxPixInModule / maxIterGPU, and should be a multiple of the warp size
    static constexpr uint32_t maxElementsPerBlock =
        cms::alpakatools::round_up_by(TrackerTraits::maxPixInModule / maxIterGPU, 128);

    ALPAKA_FN_ACC void operator()(Acc1D const& acc,
                                  SiPixelDigisSoAView digi_view,
                                  SiPixelClustersSoAView clus_view,
                                  const unsigned int numElements) const {
      static_assert(TrackerTraits::numberOfModules < ::pixelClustering::maxNumModules);

      auto& lastPixel = alpaka::declareSharedVar<unsigned int, __COUNTER__>(acc);
#ifdef GPU_DEBUG
      auto& goodPixels = alpaka::declareSharedVar<uint32_t, __COUNTER__>(acc);
#endif

      const uint32_t lastModule = clus_view[0].moduleStart();
      for (uint32_t module : cms::alpakatools::independent_groups(acc, lastModule)) {
        auto firstPixel = clus_view[1 + module].moduleStart();
        uint32_t thisModuleId = digi_view[firstPixel].moduleId();
        ALPAKA_ASSERT_ACC(thisModuleId < TrackerTraits::numberOfModules);

#ifdef GPU_DEBUG
        auto block = alpaka::getIdx<alpaka::Grid, alpaka::Blocks>(acc)[0u];
        if (thisModuleId % 100 == 1)
          if (cms::alpakatools::once_per_block(acc))
            printf("start clusterizer for module %4d in block %4d\n", thisModuleId, block);
#endif

        // Find the index of the first pixel not belonging to this module (or invalid).
        // Note: modules are not consecutive in clus_view, so we cannot use something like
        // lastPixel = (module + 1 == lastModule) ? numElements : clus_view[2 + module].moduleStart();
        lastPixel = numElements;
#ifdef GPU_DEBUG
        goodPixels = 0;
#endif
        alpaka::syncBlockThreads(acc);

        for (uint32_t i : cms::alpakatools::independent_group_elements(acc, firstPixel, numElements)) {
          auto id = digi_view[i].moduleId();
          // skip invalid pixels
          if (id == ::pixelClustering::invalidModuleId)
            continue;
          // find the first pixel in a different module
          if (id != thisModuleId) {
            alpaka::atomicMin(acc, &lastPixel, i, alpaka::hierarchy::Threads{});
            break;
          }
        }

        using Hist = cms::alpakatools::HistoContainer<uint16_t,
                                                      TrackerTraits::clusterBinning,
                                                      TrackerTraits::maxPixInModule,
                                                      TrackerTraits::clusterBits,
                                                      uint16_t>;
        constexpr int warpSize = cms::alpakatools::warpSize;
        auto& hist = alpaka::declareSharedVar<Hist, __COUNTER__>(acc);
        auto& ws = alpaka::declareSharedVar<typename Hist::Counter[warpSize], __COUNTER__>(acc);
        for (uint32_t j : cms::alpakatools::independent_group_elements(acc, Hist::totbins())) {
          hist.off[j] = 0;
        }
        alpaka::syncBlockThreads(acc);

        ALPAKA_ASSERT_ACC((lastPixel == numElements) or
                          ((lastPixel < numElements) and (digi_view[lastPixel].moduleId() != thisModuleId)));
        // Limit the number of pixels to to maxPixInModule.
        // FIXME if this happens frequently (and not only in simulation with low threshold) one will need to implement something cleverer.
        if (cms::alpakatools::once_per_block(acc)) {
          if (lastPixel - firstPixel > TrackerTraits::maxPixInModule) {
            printf("Too many pixels in module %u: %u > %u\n",
                   thisModuleId,
                   lastPixel - firstPixel,
                   TrackerTraits::maxPixInModule);
            lastPixel = TrackerTraits::maxPixInModule + firstPixel;
          }
        }
        alpaka::syncBlockThreads(acc);
        ALPAKA_ASSERT_ACC(lastPixel - firstPixel <= TrackerTraits::maxPixInModule);

        // remove duplicate pixels
        constexpr bool isPhase2 = std::is_base_of<pixelTopology::Phase2, TrackerTraits>::value;
        if constexpr (not isPhase2) {
          // packed words array used to store the pixelStatus of each pixel
          auto& image = alpaka::declareSharedVar<uint32_t[pixelStatus::size], __COUNTER__>(acc);

          if (lastPixel > 1) {
            for (uint32_t i : cms::alpakatools::independent_group_elements(acc, pixelStatus::size)) {
              image[i] = 0;
            }
            alpaka::syncBlockThreads(acc);

            for (uint32_t i : cms::alpakatools::independent_group_elements(acc, firstPixel, lastPixel)) {
              // skip invalid pixels
              if (digi_view[i].moduleId() == ::pixelClustering::invalidModuleId)
                continue;
              pixelStatus::promote(acc, image, digi_view[i].xx(), digi_view[i].yy());
            }
            alpaka::syncBlockThreads(acc);

            for (uint32_t i : cms::alpakatools::independent_group_elements(acc, firstPixel, lastPixel)) {
              // skip invalid pixels
              if (digi_view[i].moduleId() == ::pixelClustering::invalidModuleId)
                continue;
              auto status = pixelStatus::getStatus(image, digi_view[i].xx(), digi_view[i].yy());
              if (pixelStatus::kDuplicate == status) {
                // Mark all duplicate pixels as invalid.
                // Note: the alternative approach to keep a single one of the duplicates would probably make more sense.
                // According to Danek (16 March 2022): "The best would be to ignore such hits, most likely they are just
                // noise. Accepting just one is also OK, any of them."
                digi_view[i].moduleId() = ::pixelClustering::invalidModuleId;
                digi_view[i].rawIdArr() = 0;
              }
            }
            alpaka::syncBlockThreads(acc);
          }  // if (lastPixel > 1)
        }  // if constexpr (not isPhase2)

        // fill histo
        for (uint32_t i : cms::alpakatools::independent_group_elements(acc, firstPixel, lastPixel)) {
          // skip invalid pixels
          if (digi_view[i].moduleId() != ::pixelClustering::invalidModuleId) {
            hist.count(acc, digi_view[i].yy());
#ifdef GPU_DEBUG
            alpaka::atomicAdd(acc, &goodPixels, 1u, alpaka::hierarchy::Threads{});
#endif
          }
        }
        for (uint32_t i : cms::alpakatools::independent_group_elements(acc, warpSize)) {
          ws[i] = 0;  // used by prefix scan...
        }
        alpaka::syncBlockThreads(acc);
        hist.finalize(acc, ws);
        alpaka::syncBlockThreads(acc);

#ifdef GPU_DEBUG
        ALPAKA_ASSERT_ACC(hist.size() == goodPixels);
        if (thisModuleId % 100 == 1) {
          if (cms::alpakatools::once_per_block(acc)) {
            printf("histo size %d\n", hist.size());
          }
        }
#endif
        for (uint32_t i : cms::alpakatools::independent_group_elements(acc, firstPixel, lastPixel)) {
          // skip invalid pixels
          if (digi_view[i].moduleId() != ::pixelClustering::invalidModuleId) {
            // For valid pixels, the id used in the histogram is `i - firstPixel`, ranging from `0` to
            // `lastPixel - firstPixel` (excluded), so in the `[0, TrackerTraits::maxPixInModule)` range.
            hist.fill(acc, digi_view[i].yy(), i - firstPixel);
          }
        }

#ifdef GPU_DEBUG
        // look for anomalous high occupancy
        auto& n40 = alpaka::declareSharedVar<uint32_t, __COUNTER__>(acc);
        auto& n60 = alpaka::declareSharedVar<uint32_t, __COUNTER__>(acc);
        if (cms::alpakatools::once_per_block(acc)) {
          n40 = 0;
          n60 = 0;
        }
        alpaka::syncBlockThreads(acc);
        for (uint32_t j : cms::alpakatools::independent_group_elements(acc, Hist::nbins())) {
          if (hist.size(j) > 60)
            alpaka::atomicAdd(acc, &n60, 1u, alpaka::hierarchy::Threads{});
          if (hist.size(j) > 40)
            alpaka::atomicAdd(acc, &n40, 1u, alpaka::hierarchy::Threads{});
        }
        alpaka::syncBlockThreads(acc);
        if (cms::alpakatools::once_per_block(acc)) {
          if (n60 > 0)
            printf("columns with more than 60 px %d in %d\n", n60, thisModuleId);
          else if (n40 > 0)
            printf("columns with more than 40 px %d in %d\n", n40, thisModuleId);
        }
        alpaka::syncBlockThreads(acc);
#endif

        [[maybe_unused]] const uint32_t blockDimension = alpaka::getWorkDiv<alpaka::Block, alpaka::Elems>(acc)[0u];
        // assume that we can cover the whole module with up to maxIterGPU blockDimension-wide iterations
        ALPAKA_ASSERT_ACC((hist.size() / blockDimension) < maxIterGPU);

        // number of elements per thread
        constexpr uint32_t maxElements =
            cms::alpakatools::requires_single_thread_per_block_v<Acc1D> ? maxElementsPerBlock : 1;
        ALPAKA_ASSERT_ACC((alpaka::getWorkDiv<alpaka::Thread, alpaka::Elems>(acc)[0u] <= maxElements));

        constexpr unsigned int maxIter = maxIterGPU * maxElements;

        // nearest neighbours (nn)
        constexpr int maxNeighbours = 8;
        uint16_t nn[maxIter][maxNeighbours];
        uint8_t nnn[maxIter];  // number of nn
        for (uint32_t k = 0; k < maxIter; ++k) {
          nnn[k] = 0;
        }

        alpaka::syncBlockThreads(acc);  // for hit filling!

        // fill the nearest neighbours
        uint32_t k = 0;
        for (uint32_t j : cms::alpakatools::independent_group_elements(acc, hist.size())) {
          ALPAKA_ASSERT_ACC(k < maxIter);
          auto p = hist.begin() + j;
          auto i = *p + firstPixel;
          ALPAKA_ASSERT_ACC(digi_view[i].moduleId() != ::pixelClustering::invalidModuleId);
          ALPAKA_ASSERT_ACC(digi_view[i].moduleId() == thisModuleId);  // same module
          auto bin = Hist::bin(digi_view[i].yy() + 1);
          auto end = hist.end(bin);
          ++p;
          ALPAKA_ASSERT_ACC(0 == nnn[k]);
          for (; p < end; ++p) {
            auto m = *p + firstPixel;
            ALPAKA_ASSERT_ACC(m != i);
            ALPAKA_ASSERT_ACC(int(digi_view[m].yy()) - int(digi_view[i].yy()) >= 0);
            ALPAKA_ASSERT_ACC(int(digi_view[m].yy()) - int(digi_view[i].yy()) <= 1);
            if (std::abs(int(digi_view[m].xx()) - int(digi_view[i].xx())) <= 1) {
              auto l = nnn[k]++;
              ALPAKA_ASSERT_ACC(l < maxNeighbours);
              nn[k][l] = *p;
            }
          }
          ++k;
        }

        // For each pixel, look at all the pixels until the end of the module;
        // when two valid pixels within +/- 1 in x or y are found, set their id to the minimum;
        // after the loop, all the pixel in each cluster should have the id equeal to the lowest
        // pixel in the cluster ( clus[i] == i ).
        bool done = false;
        while (alpaka::syncBlockThreadsPredicate<alpaka::BlockOr>(acc, not done)) {
          done = true;
          uint32_t k = 0;
          for (uint32_t j : cms::alpakatools::independent_group_elements(acc, hist.size())) {
            ALPAKA_ASSERT_ACC(k < maxIter);
            auto p = hist.begin() + j;
            auto i = *p + firstPixel;
            for (int kk = 0; kk < nnn[k]; ++kk) {
              auto l = nn[k][kk];
              auto m = l + firstPixel;
              ALPAKA_ASSERT_ACC(m != i);
              // The algorithm processes one module per block, so the atomic operation's scope
              // is "Threads" (all threads in the current block)
              auto old =
                  alpaka::atomicMin(acc, &digi_view[m].clus(), digi_view[i].clus(), alpaka::hierarchy::Threads{});
              if (old != digi_view[i].clus()) {
                // end the loop only if no changes were applied
                done = false;
              }
              alpaka::atomicMin(acc, &digi_view[i].clus(), old, alpaka::hierarchy::Threads{});
            }  // neighbours loop
            ++k;
          }  // pixel loop
          alpaka::syncBlockThreads(acc);
          for (uint32_t j : cms::alpakatools::independent_group_elements(acc, hist.size())) {
            auto p = hist.begin() + j;
            auto i = *p + firstPixel;
            auto m = digi_view[i].clus();
            while (m != digi_view[m].clus())
              m = digi_view[m].clus();
            digi_view[i].clus() = m;
          }
        }  // end while

        /*
        // check that all threads in the block have executed the same number of iterations
        auto& n0 = alpaka::declareSharedVar<int, __COUNTER__>(acc);
        if (cms::alpakatools::once_per_block(acc))
          n0 = nloops;
        alpaka::syncBlockThreads(acc);
        ALPAKA_ASSERT_ACC(alpaka::syncBlockThreadsPredicate<alpaka::BlockAnd>(acc, nloops == n0));
        if (thisModuleId % 100 == 1)
          if (cms::alpakatools::once_per_block(acc))
            printf("# loops %d\n", nloops);
        */

        auto& foundClusters = alpaka::declareSharedVar<unsigned int, __COUNTER__>(acc);
        foundClusters = 0;
        alpaka::syncBlockThreads(acc);

        // find the number of different clusters, identified by a pixels with clus[i] == i;
        // mark these pixels with a negative id.
        for (uint32_t i : cms::alpakatools::independent_group_elements(acc, firstPixel, lastPixel)) {
          // skip invalid pixels
          if (digi_view[i].moduleId() == ::pixelClustering::invalidModuleId)
            continue;
          if (digi_view[i].clus() == static_cast<int>(i)) {
            auto old = alpaka::atomicInc(acc, &foundClusters, 0xffffffff, alpaka::hierarchy::Threads{});
            digi_view[i].clus() = -(old + 1);
          }
        }
        alpaka::syncBlockThreads(acc);

        // propagate the negative id to all the pixels in the cluster.
        for (uint32_t i : cms::alpakatools::independent_group_elements(acc, firstPixel, lastPixel)) {
          // skip invalid pixels
          if (digi_view[i].moduleId() == ::pixelClustering::invalidModuleId)
            continue;
          if (digi_view[i].clus() >= 0) {
            // mark each pixel in a cluster with the same id as the first one
            digi_view[i].clus() = digi_view[digi_view[i].clus()].clus();
          }
        }
        alpaka::syncBlockThreads(acc);

        // adjust the cluster id to be a positive value starting from 0
        for (uint32_t i : cms::alpakatools::independent_group_elements(acc, firstPixel, lastPixel)) {
          if (digi_view[i].moduleId() == ::pixelClustering::invalidModuleId) {
            // mark invalid pixels with an invalid cluster index
            digi_view[i].clus() = ::pixelClustering::invalidClusterId;
          } else {
            digi_view[i].clus() = -digi_view[i].clus() - 1;
          }
        }
        alpaka::syncBlockThreads(acc);

        if (cms::alpakatools::once_per_block(acc)) {
          clus_view[thisModuleId].clusInModule() = foundClusters;
          clus_view[module].moduleId() = thisModuleId;
#ifdef GPU_DEBUG
          if (foundClusters > gMaxHit) {
            gMaxHit = foundClusters;
            if (foundClusters > 8)
              printf("max hit %d in %d\n", foundClusters, thisModuleId);
          }
          if (thisModuleId % 100 == 1)
            printf("%d clusters in module %d\n", foundClusters, thisModuleId);
#endif
        }
      }  // module loop
    }
  };

}  // namespace ALPAKA_ACCELERATOR_NAMESPACE::pixelClustering

#endif  // plugin_SiPixelClusterizer_alpaka_PixelClustering.h
