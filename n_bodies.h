#include "cuda.h"
#include "types.h"
#include <vector>

namespace n_bodies {
// TODO(laiqu) figure out less retarded way to keep those variables (so pretty
// much create better init).
extern CUdevice cuDevice;
extern CUcontext cuContext;
extern CUmodule cuModule;

std::vector<Body> simulate(const std::vector<Body>& bodies, K tick,
        int dims=DIMENSIONS);
std::vector<Body> simulate(Body const* bodies, int n, K tick,
        int dims=DIMENSIONS);
std::vector<Body> simulate(CUdeviceptr& bodies, int n, K tick,
        int dims=DIMENSIONS);

bool init();
}
