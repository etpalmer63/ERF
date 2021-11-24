// Generated by Pankaj Jha on 05/31/2021
#include <TimeIntegration.H>

using namespace amrex;

AMREX_GPU_DEVICE
Real
ComputeStrainRate(const int &i, const int &j, const int &k,
                  const Array4<Real>& u, const Array4<Real>& v, const Array4<Real>& w,
                  const enum NextOrPrev &nextOrPrev,
                  const enum MomentumEqn &momentumEqn,
                  const enum DiffusionDir &diffDir,
                  const GpuArray<Real, AMREX_SPACEDIM>& cellSize) {
  Real dx = cellSize[0];
  Real dy = cellSize[1];
  Real dz = cellSize[2];

  Real strainRate = 0;

  switch (momentumEqn) {
  case MomentumEqn::x:
    switch (diffDir) {
    case DiffusionDir::x: // S11
      if (nextOrPrev == NextOrPrev::next)
        strainRate = (u(i+1, j, k) - u(i, j, k))/dx; // S11 (i+1/2)
      else // nextOrPrev == NextOrPrev::prev
        strainRate = (u(i, j, k) - u(i-1, j, k))/dx; // S11 (i-1/2)
      break;
    case DiffusionDir::y: // S12
      if (nextOrPrev == NextOrPrev::next)
        strainRate = (u(i, j+1, k) - u(i, j, k))/dy + (v(i, j+1, k) - v(i-1, j+1, k))/dx; // S12 (j+1/2)
      else // nextOrPrev == NextOrPrev::prev
        strainRate = (u(i, j, k) - u(i, j-1, k))/dy + (v(i, j, k) - v(i-1, j, k))/dx; // S12 (j-1/2)
      strainRate *= 0.5;
      break;
    case DiffusionDir::z: // S13
      if (nextOrPrev == NextOrPrev::next)
        strainRate = (u(i, j, k+1) - u(i, j, k))/dz + (w(i, j, k+1) - w(i-1, j, k+1))/dx; // S13 (k+1/2)
      else // nextOrPrev == NextOrPrev::prev
        strainRate = (u(i, j, k) - u(i, j, k-1))/dz + (w(i, j, k) - w(i-1, j, k))/dx; // S13 (k-1/2)
      strainRate *= 0.5;
      break;
    default:
      amrex::Abort("Error: Diffusion direction is unrecognized");
    }
    break;
  case MomentumEqn::y:
    switch (diffDir) {
    case DiffusionDir::x: // S21
      if (nextOrPrev == NextOrPrev::next)
        strainRate = (u(i+1, j, k) - u(i+1, j-1, k))/dy + (v(i+1, j, k) - v(i, j, k))/dx; // S21 (i+1/2)
      else // nextOrPrev == NextOrPrev::prev
        strainRate = (u(i, j, k) - u(i, j-1, k))/dy + (v(i, j, k) - v(i-1, j, k))/dx; // S21 (i-1/2)
      strainRate *= 0.5;
      break;
    case DiffusionDir::y: // S22
      if (nextOrPrev == NextOrPrev::next)
        strainRate = (v(i, j+1, k) - v(i, j, k))/dy; // S22 (j+1/2)
      else // nextOrPrev == NextOrPrev::prev
        strainRate = (v(i, j, k) - v(i, j-1, k))/dy; // S22 (j-1/2)
      break;
    case DiffusionDir::z: // S23
      if (nextOrPrev == NextOrPrev::next)
        strainRate = (v(i, j, k+1) - v(i, j, k))/dz + (w(i, j, k+1) - w(i, j-1, k+1))/dy; // S23 (k+1/2) //TODO: Check this with Branko
      else // nextOrPrev == NextOrPrev::prev
        strainRate = (v(i, j, k) - v(i, j, k-1))/dz + (w(i, j, k) - w(i, j-1, k))/dy; // S23 (k-1/2) //TODO: Check this with Branko
      strainRate *= 0.5;
      break;
    default:
      amrex::Abort("Error: Diffusion direction is unrecognized");
    }
    break;
  case MomentumEqn::z:
    switch (diffDir) {
    case DiffusionDir::x: // S31
      if (nextOrPrev == NextOrPrev::next)
        strainRate = (u(i+1, j, k) - u(i+1, j, k-1))/dz + (w(i+1, j, k) - w(i, j, k))/dx; // S31 (i+1/2)
      else // nextOrPrev == NextOrPrev::prev
        strainRate = (u(i, j, k) - u(i, j, k-1))/dz + (w(i, j, k) - w(i-1, j, k))/dx; // S31 (i-1/2)
      strainRate *= 0.5;
      break;
    case DiffusionDir::y: // S32
      if (nextOrPrev == NextOrPrev::next)
        strainRate = (v(i, j+1, k) - v(i, j+1, k-1))/dz + (w(i, j+1, k) - w(i, j, k))/dy; // S32 (j+1/2) //TODO: Check this with Branko
      else // nextOrPrev == NextOrPrev::prev
        strainRate = (v(i, j, k) - v(i, j, k-1))/dz + (w(i, j, k) - w(i, j-1, k))/dy; // S32 (j-1/2) //TODO: Check this with Branko
      strainRate *= 0.5;
      break;
    case DiffusionDir::z: // S33
      if (nextOrPrev == NextOrPrev::next)
        strainRate = (w(i, j, k+1) - w(i, j, k))/dz; // S33 (k+1/2)
      else // nextOrPrev == NextOrPrev::prev
        strainRate = (w(i, j, k) - w(i, j, k-1))/dz; // S33 (k-1/2)
      break;
    default:
      amrex::Abort("Error: Diffusion direction is unrecognized");
    }
    break;
  default:
    amrex::Abort("Error: Momentum equation is unrecognized");
  }

  return strainRate;
}

AMREX_GPU_DEVICE
Real
ComputeExpansionRate(const int &i, const int &j, const int &k,
                     const Array4<Real>& u, const Array4<Real>& v, const Array4<Real>& w,
                     const enum NextOrPrev &nextOrPrev,
                     const enum MomentumEqn &momentumEqn,
                     const enum DiffusionDir &diffDir,
                     const GpuArray<Real, AMREX_SPACEDIM>& cellSize) {
    Real dx = cellSize[0];
    Real dy = cellSize[1];
    Real dz = cellSize[2];

    Real rotationRate = 0;

    //TODO: Implement the correct discretization of diagonal terms

    switch (momentumEqn) {
        case MomentumEqn::x:
            switch (diffDir) {
                case DiffusionDir::x: // D11
                    if (nextOrPrev == NextOrPrev::next)
                        rotationRate = (u(i+1, j, k) - u(i, j, k))/dx; // S11 (i+1/2)
                    else // nextOrPrev == NextOrPrev::prev
                        rotationRate = (u(i, j, k) - u(i-1, j, k))/dx; // S11 (i-1/2)
                    break;
                case DiffusionDir::y: // D12
                    rotationRate = 0.0;
                    break;
                case DiffusionDir::z: // D13
                    rotationRate = 0.0;
                    break;
                default:
                    amrex::Abort("Error: Diffusion direction is unrecognized");
            }
            break;
        case MomentumEqn::y:
            switch (diffDir) {
                case DiffusionDir::x: // D21
                    rotationRate = 0.0;
                    break;
                case DiffusionDir::y: // D22
                    if (nextOrPrev == NextOrPrev::next)
                        rotationRate = (v(i, j+1, k) - v(i, j, k))/dy; // S22 (j+1/2)
                    else // nextOrPrev == NextOrPrev::prev
                        rotationRate = (v(i, j, k) - v(i, j-1, k))/dy; // S22 (j-1/2)
                    break;
                case DiffusionDir::z: // D23
                    rotationRate = 0.0;
                    break;
                default:
                    amrex::Abort("Error: Diffusion direction is unrecognized");
            }
            break;
        case MomentumEqn::z:
            switch (diffDir) {
                case DiffusionDir::x: // D31
                    rotationRate = 0.0;
                    break;
                case DiffusionDir::y: // D32
                    rotationRate = 0.0;
                    break;
                case DiffusionDir::z: // D33
                    if (nextOrPrev == NextOrPrev::next)
                        rotationRate = (w(i, j, k+1) - w(i, j, k))/dz; // S33 (k+1/2)
                    else // nextOrPrev == NextOrPrev::prev
                        rotationRate = (w(i, j, k) - w(i, j, k-1))/dz; // S33 (k-1/2)
                    break;
                default:
                    amrex::Abort("Error: Diffusion direction is unrecognized");
            }
            break;
        default:
            amrex::Abort("Error: Momentum equation is unrecognized");
    }

    return rotationRate;
}
