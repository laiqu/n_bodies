#include "cuda.h"
#include "types.h"
#include <vector>

namespace n_bodies {
// TODO(laiqu) figure out less retarded way to keep those variables (so pretty
// much create better init).
extern CUdevice cuDevice;
extern CUcontext cuContext;
extern CUmodule cuModule;
extern CUfunction cuBruteCalculateInteractions;
extern CUfunction cuUpdateVelocity;
extern CUfunction cuAdvanceBodies;
extern CUfunction cuSetZeroToAcceleration;

std::vector<Body> simulate(CUdeviceptr& bodies, int n, K tick,
        int dims=DIMENSIONS);

CUdeviceptr moveBodiesToDevice(const std::vector<Body>& bodies);
CUdeviceptr moveBodiesToDevice(Body const* bodies, int n);

bool init();
}
