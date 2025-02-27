#include <SpatialStencils.H>
#include <StressTerm.H>

using namespace amrex;

#ifdef ERF_USE_TERRAIN

AMREX_GPU_DEVICE
Real
DiffusionSrcForMom(const int &i, const int &j, const int &k,
                   const Array4<const Real>& u, const Array4<const Real>& v, const Array4<const Real>& w,
                   const Array4<const Real>& cons,
                   const enum MomentumEqn &momentumEqn,
                   const GpuArray<Real, AMREX_SPACEDIM>& cellSizeInv,
                   const Array4<Real>& K_turb,
                   const SolverChoice &solverChoice,
                   const Array4<const Real>& z_nd, const Array4<const Real>& detJ,
                   const Box& domain, const amrex::BCRec* bc_ptr)
{
  auto dxInv = cellSizeInv[0], dyInv = cellSizeInv[1], dzInv = cellSizeInv[2];
  Real diffContrib = 0.0;

  int l_spatial_order = solverChoice.spatial_order;
  Real met_h_xi,met_h_eta,met_h_zeta;

  // Nodal in k for w-momentum
  int k_extrap_lb = domain.smallEnd(2);
  int k_extrap_ub = domain.bigEnd(2) + 1;

  switch (momentumEqn) {
  case MomentumEqn::x:
    Real tau11Next, tau11Prev, tau12Next, tau12Prev, tau13Next, tau13Prev;
    Real tau11BarN, tau11BarP, tau12BarN, tau12BarP;
    Real Tmp11, Tmp12;

    // 11 Next
    // Metric at cell center
    ComputeMetricAtCellCenter(i  ,j  ,k  ,met_h_xi,met_h_eta,met_h_zeta,
                              cellSizeInv,z_nd,TerrainMet::h_zeta);
    tau11Next = ComputeStressTerm(i+1,j  ,k  , u, v, w, momentumEqn,
                                  DiffusionDir::x, cellSizeInv, K_turb, solverChoice,
                                  z_nd, domain, bc_ptr);
    // Save for average
    Tmp11 = tau11Next;
    // Scale by metric
    tau11Next *= met_h_zeta;
    //-----------------------------------------------------------------------------------
    // 11 Prev
    // Metric at cell center
    ComputeMetricAtCellCenter(i-1,j  ,k  ,met_h_xi,met_h_eta,met_h_zeta,
                              cellSizeInv,z_nd,TerrainMet::h_zeta);
    tau11Prev = ComputeStressTerm(i  ,j  ,k  , u, v, w, momentumEqn,
                                  DiffusionDir::x, cellSizeInv, K_turb, solverChoice,
                                  z_nd, domain, bc_ptr);
    // Accumulate for average
    Tmp11 += tau11Prev;
    // Scale by metric
    tau11Prev *= met_h_zeta;
    //************************************************************************************
    // 12 Next
    // Metric at EdgeCenterK
    ComputeMetricAtEdgeCenterK(i  ,j+1,k  ,met_h_xi,met_h_eta,met_h_zeta,
                                 cellSizeInv,z_nd,TerrainMet::h_zeta);
    tau12Next = ComputeStressTerm(i  ,j+1,k  , u, v, w, momentumEqn,
                                  DiffusionDir::y, cellSizeInv, K_turb, solverChoice,
                                  z_nd, domain, bc_ptr);
    // Save for average
    Tmp12 = tau12Next;
    // Scale by metric
    tau12Next *= met_h_zeta;
    //-----------------------------------------------------------------------------------
    // 12 Prev
    // Metric at EdgeCenterK
    ComputeMetricAtEdgeCenterK(i  ,j  ,k  ,met_h_xi,met_h_eta,met_h_zeta,
                                 cellSizeInv,z_nd,TerrainMet::h_zeta);
    tau12Prev = ComputeStressTerm(i  ,j  ,k  , u, v, w, momentumEqn,
                                  DiffusionDir::y, cellSizeInv, K_turb, solverChoice,
                                  z_nd, domain, bc_ptr);
    // Accumulate for average
    Tmp12 += tau12Prev;
    // Scale by metric
    tau12Prev *= met_h_zeta;
    //************************************************************************************
    // 13 Next
    // Accumulate averages
    tau11BarN  = Tmp11;
    tau12BarN  = Tmp12;
    tau11BarN += ComputeStressTerm(i+1,j  ,k+1, u, v, w, momentumEqn,
                                  DiffusionDir::x, cellSizeInv, K_turb, solverChoice,
                                  z_nd, domain, bc_ptr);
    tau11BarN += ComputeStressTerm(i  ,j  ,k+1, u, v, w, momentumEqn,
                                  DiffusionDir::x, cellSizeInv, K_turb, solverChoice,
                                  z_nd, domain, bc_ptr);
    tau11BarN *= 0.25;
    tau12BarN += ComputeStressTerm(i  ,j+1,k+1, u, v, w, momentumEqn,
                                  DiffusionDir::y, cellSizeInv, K_turb, solverChoice,
                                  z_nd, domain, bc_ptr);
    tau12BarN += ComputeStressTerm(i  ,j  ,k+1, u, v, w, momentumEqn,
                                  DiffusionDir::y, cellSizeInv, K_turb, solverChoice,
                                  z_nd, domain, bc_ptr);
    tau12BarN *= 0.25;

    // Metric at EdgeCenterJ
    ComputeMetricAtEdgeCenterJ(i  ,j  ,k+1,met_h_xi,met_h_eta,met_h_zeta,
                               cellSizeInv,z_nd,TerrainMet::h_xi_eta);
    tau13Next = -met_h_xi * tau11BarN - met_h_eta * tau12BarN
               + ComputeStressTerm(i  ,j  ,k+1, u, v, w, momentumEqn,
                                   DiffusionDir::z, cellSizeInv, K_turb, solverChoice,
                                   z_nd, domain, bc_ptr);
    //-----------------------------------------------------------------------------------
    // 13 Prev
    // Accumulate averages
    tau11BarP = Tmp11;
    tau12BarP = Tmp12;
    tau11BarP += ComputeStressTerm(i+1,j  ,k-1, u, v, w, momentumEqn,
                                   DiffusionDir::x, cellSizeInv, K_turb, solverChoice,
                                   z_nd, domain, bc_ptr);
    tau11BarP += ComputeStressTerm(i  ,j  ,k-1, u, v, w, momentumEqn,
                                   DiffusionDir::x, cellSizeInv, K_turb, solverChoice,
                                   z_nd, domain, bc_ptr);
    tau11BarP *= 0.25;
    tau12BarP += ComputeStressTerm(i  ,j+1,k-1, u, v, w, momentumEqn,
                                   DiffusionDir::y, cellSizeInv, K_turb, solverChoice,
                                   z_nd, domain, bc_ptr);
    tau12BarP += ComputeStressTerm(i  ,j  ,k-1, u, v, w, momentumEqn,
                                   DiffusionDir::y, cellSizeInv, K_turb, solverChoice,
                                   z_nd, domain, bc_ptr);
    tau12BarP *= 0.25;

    // Metric at EdgeCenterJ
    ComputeMetricAtEdgeCenterJ(i  ,j  ,k  ,met_h_xi,met_h_eta,met_h_zeta,
                               cellSizeInv,z_nd,TerrainMet::h_xi_eta);
    tau13Prev = -met_h_xi * tau11BarP - met_h_eta * tau12BarP
               + ComputeStressTerm(i  ,j  ,k  , u, v, w, momentumEqn,
                                   DiffusionDir::z, cellSizeInv, K_turb, solverChoice,
                                   z_nd, domain, bc_ptr);
    //************************************************************************************
    diffContrib = (tau11Next - tau11Prev) * dxInv  // Contribution to x-mom eqn from diffusive flux in x-dir
                + (tau12Next - tau12Prev) * dyInv  // Contribution to x-mom eqn from diffusive flux in y-dir
                + (tau13Next - tau13Prev) * dzInv; // Contribution to x-mom eqn from diffusive flux in z-dir

    diffContrib /= 0.5*(detJ(i,j,k) + detJ(i-1,j,k)); // Terrain grid stretching

    if (solverChoice.molec_diff_type == MolecDiffType::ConstantAlpha)
    {
      diffContrib *= InterpolateFromCellOrFace(i, j, k, cons, Rho_comp, u(i,j,k), Coord::x, l_spatial_order) /
        solverChoice.rho0_trans;
    }
    break;
  case MomentumEqn::y:
    Real tau21Next, tau21Prev, tau22Next, tau22Prev, tau23Next, tau23Prev;
    Real tau21BarN, tau22BarN, tau21BarP, tau22BarP;
    Real Tmp21, Tmp22;

    // 21 Next
    ComputeMetricAtEdgeCenterK(i+1,j  ,k  ,met_h_xi,met_h_eta,met_h_zeta,
                               cellSizeInv,z_nd,TerrainMet::h_zeta);
    tau21Next = ComputeStressTerm(i+1,j  ,k  , u, v, w, momentumEqn,
                                  DiffusionDir::x, cellSizeInv, K_turb, solverChoice,
                                  z_nd, domain, bc_ptr);
    Tmp21 = tau21Next;
    tau21Next *= met_h_zeta;
    //-----------------------------------------------------------------------------------
    // 21 Prev
    ComputeMetricAtEdgeCenterK(i  ,j  ,k  ,met_h_xi,met_h_eta,met_h_zeta,
                               cellSizeInv,z_nd,TerrainMet::h_zeta);
    tau21Prev = ComputeStressTerm(i  ,j  ,k  , u, v, w, momentumEqn,
                                  DiffusionDir::x, cellSizeInv, K_turb, solverChoice,
                                  z_nd, domain, bc_ptr);
    Tmp21 += tau21Prev;
    tau21Prev *= met_h_zeta;
    //************************************************************************************
    // 22 Next
    ComputeMetricAtCellCenter(i  ,j  ,k  ,met_h_xi,met_h_eta,met_h_zeta,
                              cellSizeInv,z_nd,TerrainMet::h_zeta);
    tau22Next = ComputeStressTerm(i  ,j+1,k  , u, v, w, momentumEqn,
                                  DiffusionDir::y, cellSizeInv, K_turb, solverChoice,
                                  z_nd, domain, bc_ptr);
    Tmp22 = tau22Next;
    tau22Next *= met_h_zeta;
    //-----------------------------------------------------------------------------------
    // 22 Prev
    ComputeMetricAtCellCenter(i  ,j-1,k  ,met_h_xi,met_h_eta,met_h_zeta,
                              cellSizeInv,z_nd,TerrainMet::h_zeta);
    tau22Prev = ComputeStressTerm(i  ,j  ,k  , u, v, w, momentumEqn,
                                  DiffusionDir::y, cellSizeInv, K_turb, solverChoice,
                                  z_nd, domain, bc_ptr);
    Tmp22 += tau22Prev;
    tau22Prev *= met_h_zeta;
    //************************************************************************************
    // 23 Next
    // Accumulate averages
    tau21BarN  = Tmp21;
    tau22BarN  = Tmp22;
    tau21BarN += ComputeStressTerm(i+1,j  ,k+1, u, v, w, momentumEqn,
                                  DiffusionDir::x, cellSizeInv, K_turb, solverChoice,
                                  z_nd, domain, bc_ptr);
    tau21BarN += ComputeStressTerm(i  ,j  ,k+1, u, v, w, momentumEqn,
                                  DiffusionDir::x, cellSizeInv, K_turb, solverChoice,
                                  z_nd, domain, bc_ptr);
    tau21BarN *= 0.25;
    tau22BarN += ComputeStressTerm(i  ,j+1,k+1, u, v, w, momentumEqn,
                                  DiffusionDir::y, cellSizeInv, K_turb, solverChoice,
                                  z_nd, domain, bc_ptr);
    tau22BarN += ComputeStressTerm(i  ,j  ,k+1, u, v, w, momentumEqn,
                                  DiffusionDir::y, cellSizeInv, K_turb, solverChoice,
                                  z_nd, domain, bc_ptr);
    tau22BarN *= 0.25;

    // Metric at EdgeCenterI
    ComputeMetricAtEdgeCenterI(i  ,j  ,k+1,met_h_xi,met_h_eta,met_h_zeta,
                               cellSizeInv,z_nd,TerrainMet::h_xi_eta);
    tau23Next = -met_h_xi * tau21BarN - met_h_eta * tau22BarN
               + ComputeStressTerm(i  ,j  ,k+1, u, v, w, momentumEqn,
                                   DiffusionDir::z, cellSizeInv, K_turb, solverChoice,
                                   z_nd, domain, bc_ptr);
    //-----------------------------------------------------------------------------------
    // 23 Prev
    // Accumulate averages
    tau21BarP = Tmp21;
    tau22BarP = Tmp22;
    tau21BarP += ComputeStressTerm(i+1,j  ,k-1, u, v, w, momentumEqn,
                                   DiffusionDir::x, cellSizeInv, K_turb, solverChoice,
                                   z_nd, domain, bc_ptr);
    tau21BarP += ComputeStressTerm(i  ,j  ,k-1, u, v, w, momentumEqn,
                                   DiffusionDir::x, cellSizeInv, K_turb, solverChoice,
                                   z_nd, domain, bc_ptr);
    tau21BarP *= 0.25;
    tau22BarP += ComputeStressTerm(i  ,j+1,k-1, u, v, w, momentumEqn,
                                   DiffusionDir::y, cellSizeInv, K_turb, solverChoice,
                                   z_nd, domain, bc_ptr);
    tau22BarP += ComputeStressTerm(i  ,j  ,k-1, u, v, w, momentumEqn,
                                   DiffusionDir::y, cellSizeInv, K_turb, solverChoice,
                                   z_nd, domain, bc_ptr);
    tau22BarP *= 0.25;

    // Metric at EdgeCenterI
    ComputeMetricAtEdgeCenterI(i  ,j  ,k  ,met_h_xi,met_h_eta,met_h_zeta,
                               cellSizeInv,z_nd,TerrainMet::h_xi_eta);
    tau23Prev = -met_h_xi * tau21BarP - met_h_eta * tau22BarP
               + ComputeStressTerm(i  ,j  ,k  , u, v, w, momentumEqn,
                                   DiffusionDir::z, cellSizeInv, K_turb, solverChoice,
                                   z_nd, domain, bc_ptr);
    //************************************************************************************
    diffContrib = (tau21Next - tau21Prev) * dxInv  // Contribution to y-mom eqn from diffusive flux in x-dir
                + (tau22Next - tau22Prev) * dyInv  // Contribution to y-mom eqn from diffusive flux in y-dir
                + (tau23Next - tau23Prev) * dzInv; // Contribution to y-mom eqn from diffusive flux in z-dir

    diffContrib /= 0.5*(detJ(i,j,k) + detJ(i,j-1,k)); // Terrain grid stretching

    if (solverChoice.molec_diff_type == MolecDiffType::ConstantAlpha)
    {
      diffContrib *= InterpolateFromCellOrFace(i, j, k, cons, Rho_comp, v(i,j,k), Coord::y, l_spatial_order) /
        solverChoice.rho0_trans;
    }
    break;
  case MomentumEqn::z:
    Real tau31Next, tau31Prev, tau32Next, tau32Prev, tau33Next, tau33Prev, normv;

    Real tau31BarN, tau32BarN, tau31BarP, tau32BarP;
    Real Tmp31, Tmp32;

    // 31 Next
    ComputeMetricAtEdgeCenterJ(i+1,j  ,k  ,met_h_xi,met_h_eta,met_h_zeta,
                               cellSizeInv,z_nd,TerrainMet::h_zeta);
    tau31Next = ComputeStressTerm(i+1,j  ,k  , u, v, w, momentumEqn,
                                  DiffusionDir::x, cellSizeInv, K_turb, solverChoice,
                                  z_nd, domain, bc_ptr);
    Tmp31 = tau31Next;
    tau31Next *= met_h_zeta;
    //-----------------------------------------------------------------------------------
    //31 Prev
    ComputeMetricAtEdgeCenterJ(i  ,j  ,k  ,met_h_xi,met_h_eta,met_h_zeta,
                               cellSizeInv,z_nd,TerrainMet::h_zeta);
    tau31Prev = ComputeStressTerm(i  ,j  ,k  , u, v, w, momentumEqn,
                                  DiffusionDir::x, cellSizeInv, K_turb, solverChoice,
                                  z_nd, domain, bc_ptr);
    Tmp31 += tau31Prev;
    tau31Prev *= met_h_zeta;
    //************************************************************************************
    // 32 Next
    ComputeMetricAtEdgeCenterI(i  ,j+1,k  ,met_h_xi,met_h_eta,met_h_zeta,
                               cellSizeInv,z_nd,TerrainMet::h_zeta);
    tau32Next = ComputeStressTerm(i  ,j+1,k  , u, v, w, momentumEqn,
                                  DiffusionDir::y, cellSizeInv, K_turb, solverChoice,
                                  z_nd, domain, bc_ptr);
    Tmp32 = tau32Next;
    tau32Next *= met_h_zeta;
    //-----------------------------------------------------------------------------------
    // 32 Prev
    ComputeMetricAtEdgeCenterI(i  ,j  ,k  ,met_h_xi,met_h_eta,met_h_zeta,
                               cellSizeInv,z_nd,TerrainMet::h_zeta);
    tau32Prev = ComputeStressTerm(i  ,j  ,k  , u, v, w, momentumEqn,
                                  DiffusionDir::y, cellSizeInv, K_turb, solverChoice,
                                  z_nd, domain, bc_ptr);
    Tmp32 += tau32Prev;
    tau32Prev *= met_h_zeta;
    //************************************************************************************
    // 33 Next
    tau31BarN  = Tmp31;
    tau32BarN  = Tmp32;
    if (k==k_extrap_ub) {
      // Extrapolate to upper edge center
      tau31BarN *= 0.75;
      tau31BarN -= 0.25 * ComputeStressTerm(i+1,j  ,k-1, u, v, w, momentumEqn,
                                            DiffusionDir::x, cellSizeInv, K_turb, solverChoice,
                                            z_nd, domain, bc_ptr);
      tau31BarN -= 0.25 * ComputeStressTerm(i  ,j  ,k-1, u, v, w, momentumEqn,
                                            DiffusionDir::x, cellSizeInv, K_turb, solverChoice,
                                            z_nd, domain, bc_ptr);
      tau32BarN *= 0.75;
      tau32BarN -= 0.25 * ComputeStressTerm(i  ,j+1,k-1, u, v, w, momentumEqn,
                                            DiffusionDir::y, cellSizeInv, K_turb, solverChoice,
                                            z_nd, domain, bc_ptr);
      tau32BarN -= 0.25 * ComputeStressTerm(i  ,j  ,k-1, u, v, w, momentumEqn,
                                            DiffusionDir::y, cellSizeInv, K_turb, solverChoice,
                                            z_nd, domain, bc_ptr);
    } else {
      // Accumulate averages
      tau31BarN += ComputeStressTerm(i+1,j  ,k+1, u, v, w, momentumEqn,
                                     DiffusionDir::x, cellSizeInv, K_turb, solverChoice,
                                     z_nd, domain, bc_ptr);
      tau31BarN += ComputeStressTerm(i  ,j  ,k+1, u, v, w, momentumEqn,
                                     DiffusionDir::x, cellSizeInv, K_turb, solverChoice,
                                     z_nd, domain, bc_ptr);
      tau31BarN *= 0.25;
      tau32BarN += ComputeStressTerm(i  ,j+1,k+1, u, v, w, momentumEqn,
                                     DiffusionDir::y, cellSizeInv, K_turb, solverChoice,
                                     z_nd, domain, bc_ptr);
      tau32BarN += ComputeStressTerm(i  ,j  ,k+1, u, v, w, momentumEqn,
                                     DiffusionDir::y, cellSizeInv, K_turb, solverChoice,
                                     z_nd, domain, bc_ptr);
      tau32BarN *= 0.25;
    }

    // Metric at cell center
    ComputeMetricAtCellCenter(i  ,j  ,k  ,met_h_xi,met_h_eta,met_h_zeta,
                              cellSizeInv,z_nd,TerrainMet::h_xi_eta);

    tau33Next = -met_h_xi * tau31BarN - met_h_eta * tau32BarN
               + ComputeStressTerm(i  ,j  ,k+1, u, v, w, momentumEqn,
                                   DiffusionDir::z, cellSizeInv, K_turb, solverChoice,
                                   z_nd, domain, bc_ptr);
    //-----------------------------------------------------------------------------------
    // 33 Prev
    tau31BarP  = Tmp31;
    tau32BarP  = Tmp32;
    if (k==k_extrap_lb) {
      // Extrapolate to lower edge center
      tau31BarP *= 0.75;
      tau31BarP -= 0.25 * ComputeStressTerm(i+1,j  ,k+1, u, v, w, momentumEqn,
                                            DiffusionDir::x, cellSizeInv, K_turb, solverChoice,
                                            z_nd, domain, bc_ptr);
      tau31BarP -= 0.25 * ComputeStressTerm(i  ,j  ,k+1, u, v, w, momentumEqn,
                                            DiffusionDir::x, cellSizeInv, K_turb, solverChoice,
                                            z_nd, domain, bc_ptr);
      tau32BarP *= 0.75;
      tau32BarP -= 0.25 * ComputeStressTerm(i  ,j+1,k+1, u, v, w, momentumEqn,
                                            DiffusionDir::y, cellSizeInv, K_turb, solverChoice,
                                            z_nd, domain, bc_ptr);
      tau32BarP -= 0.25 * ComputeStressTerm(i  ,j  ,k+1, u, v, w, momentumEqn,
                                            DiffusionDir::y, cellSizeInv, K_turb, solverChoice,
                                            z_nd, domain, bc_ptr);
    } else {
      // Accumulate average to tau31BarN
      tau31BarP += ComputeStressTerm(i+1,j  ,k-1, u, v, w, momentumEqn,
                                     DiffusionDir::x, cellSizeInv, K_turb, solverChoice,
                                     z_nd, domain, bc_ptr);
      tau31BarP += ComputeStressTerm(i  ,j  ,k-1, u, v, w, momentumEqn,
                                     DiffusionDir::x, cellSizeInv, K_turb, solverChoice,
                                     z_nd, domain, bc_ptr);
      tau31BarP *= 0.25;
      tau32BarP += ComputeStressTerm(i  ,j+1,k-1, u, v, w, momentumEqn,
                                     DiffusionDir::y, cellSizeInv, K_turb, solverChoice,
                                     z_nd, domain, bc_ptr);
      tau32BarP += ComputeStressTerm(i  ,j  ,k-1, u, v, w, momentumEqn,
                                     DiffusionDir::y, cellSizeInv, K_turb, solverChoice,
                                     z_nd, domain, bc_ptr);
      tau32BarP *= 0.25;
    }

    // Metrics at cell center
    ComputeMetricAtCellCenter(i  ,j  ,k-1,met_h_xi,met_h_eta,met_h_zeta,
                              cellSizeInv,z_nd,TerrainMet::h_xi_eta);

    tau33Prev = -met_h_xi * tau31BarP - met_h_eta * tau32BarP
               + ComputeStressTerm(i  ,j  ,k  , u, v, w, momentumEqn,
                                   DiffusionDir::z, cellSizeInv, K_turb, solverChoice,
                                   z_nd, domain, bc_ptr);
    //************************************************************************************
    diffContrib = (tau31Next - tau31Prev) * dxInv  // Contribution to z-mom eqn from diffusive flux in x-dir
                + (tau32Next - tau32Prev) * dyInv  // Contribution to z-mom eqn from diffusive flux in y-dir
                + (tau33Next - tau33Prev) * dzInv; // Contribution to z-mom eqn from diffusive flux in z-dir

    normv = (k == 0) ? detJ(i,j,k) : 0.5*( detJ(i,j,k) + detJ(i,j,k-1) ); // Terrain grid stretching
    diffContrib /= normv;

    if (solverChoice.molec_diff_type == MolecDiffType::ConstantAlpha)
    {
      diffContrib *= InterpolateFromCellOrFace(i, j, k, cons, Rho_comp, w(i,j,k), Coord::z, l_spatial_order) /
        solverChoice.rho0_trans;
    }
    break;
  default:
    amrex::Abort("Error: Momentum equation is unrecognized");
  }

  return diffContrib;
}

#else
AMREX_GPU_DEVICE
Real
DiffusionSrcForMom(const int &i, const int &j, const int &k,
                            const Array4<const Real>& u, const Array4<const Real>& v, const Array4<const Real>& w,
                            const Array4<const Real>& cons,
                            const enum MomentumEqn &momentumEqn,
                            const GpuArray<Real, AMREX_SPACEDIM>& cellSizeInv,
                            const Array4<Real>& K_turb,
                            const SolverChoice &solverChoice,
                            const Box& domain, const amrex::BCRec* bc_ptr)
{
    auto dxInv = cellSizeInv[0], dyInv = cellSizeInv[1], dzInv = cellSizeInv[2];
    Real diffContrib = 0.0;

    int l_spatial_order = solverChoice.spatial_order;

    switch (momentumEqn) {
        case MomentumEqn::x:
            Real tau11Next, tau11Prev, tau12Next, tau12Prev, tau13Next, tau13Prev;
            tau11Next = ComputeStressTerm(i+1, j, k, u, v, w, momentumEqn,
                                          DiffusionDir::x, cellSizeInv, K_turb, solverChoice, domain, bc_ptr);
            tau11Prev = ComputeStressTerm(i  , j, k, u, v, w, momentumEqn,
                                          DiffusionDir::x, cellSizeInv, K_turb, solverChoice, domain, bc_ptr);
            tau12Next = ComputeStressTerm(i, j+1, k, u, v, w, momentumEqn,
                                          DiffusionDir::y, cellSizeInv, K_turb, solverChoice, domain, bc_ptr);
            tau12Prev = ComputeStressTerm(i, j  , k, u, v, w, momentumEqn,
                                          DiffusionDir::y, cellSizeInv, K_turb, solverChoice, domain, bc_ptr);
            tau13Next = ComputeStressTerm(i, j, k+1, u, v, w, momentumEqn,
                                          DiffusionDir::z, cellSizeInv, K_turb, solverChoice, domain, bc_ptr);
            tau13Prev = ComputeStressTerm(i, j, k  , u, v, w, momentumEqn,
                                          DiffusionDir::z, cellSizeInv, K_turb, solverChoice, domain, bc_ptr);

            diffContrib = (tau11Next - tau11Prev) * dxInv  // Contribution to x-mom eqn from diffusive flux in x-dir
                        + (tau12Next - tau12Prev) * dyInv  // Contribution to x-mom eqn from diffusive flux in y-dir
                        + (tau13Next - tau13Prev) * dzInv; // Contribution to x-mom eqn from diffusive flux in z-dir
            if (solverChoice.molec_diff_type == MolecDiffType::ConstantAlpha)
            {
                diffContrib *= InterpolateFromCellOrFace(i, j, k, cons, Rho_comp, u(i,j,k), Coord::x, l_spatial_order) /
                               solverChoice.rho0_trans;
            }
            break;
        case MomentumEqn::y:
            Real tau21Next, tau21Prev, tau22Next, tau22Prev, tau23Next, tau23Prev;
            tau21Next = ComputeStressTerm(i+1, j, k, u, v, w, momentumEqn,
                                          DiffusionDir::x, cellSizeInv, K_turb, solverChoice, domain, bc_ptr);
            tau21Prev = ComputeStressTerm(i  , j, k, u, v, w, momentumEqn,
                                          DiffusionDir::x, cellSizeInv, K_turb, solverChoice, domain, bc_ptr);
            tau22Next = ComputeStressTerm(i, j+1, k, u, v, w, momentumEqn,
                                          DiffusionDir::y, cellSizeInv, K_turb, solverChoice, domain, bc_ptr);
            tau22Prev = ComputeStressTerm(i, j  , k, u, v, w, momentumEqn,
                                          DiffusionDir::y, cellSizeInv, K_turb, solverChoice, domain, bc_ptr);
            tau23Next = ComputeStressTerm(i, j, k+1, u, v, w, momentumEqn,
                                          DiffusionDir::z, cellSizeInv, K_turb, solverChoice, domain, bc_ptr);
            tau23Prev = ComputeStressTerm(i, j, k  , u, v, w, momentumEqn,
                                          DiffusionDir::z, cellSizeInv, K_turb, solverChoice, domain, bc_ptr);

            diffContrib = (tau21Next - tau21Prev) * dxInv  // Contribution to y-mom eqn from diffusive flux in x-dir
                        + (tau22Next - tau22Prev) * dyInv  // Contribution to y-mom eqn from diffusive flux in y-dir
                        + (tau23Next - tau23Prev) * dzInv; // Contribution to y-mom eqn from diffusive flux in z-dir
            if (solverChoice.molec_diff_type == MolecDiffType::ConstantAlpha)
            {
                diffContrib *= InterpolateFromCellOrFace(i, j, k, cons, Rho_comp, v(i,j,k), Coord::y, l_spatial_order) /
                               solverChoice.rho0_trans;
            }
            break;
        case MomentumEqn::z:
            Real tau31Next, tau31Prev, tau32Next, tau32Prev, tau33Next, tau33Prev;
            tau31Next = ComputeStressTerm(i+1, j, k, u, v, w, momentumEqn,
                                          DiffusionDir::x, cellSizeInv, K_turb, solverChoice, domain, bc_ptr);
            tau31Prev = ComputeStressTerm(i  , j, k, u, v, w, momentumEqn,
                                          DiffusionDir::x, cellSizeInv, K_turb, solverChoice, domain, bc_ptr);
            tau32Next = ComputeStressTerm(i, j+1, k, u, v, w, momentumEqn,
                                          DiffusionDir::y, cellSizeInv, K_turb, solverChoice, domain, bc_ptr);
            tau32Prev = ComputeStressTerm(i, j  , k, u, v, w, momentumEqn,
                                          DiffusionDir::y, cellSizeInv, K_turb, solverChoice, domain, bc_ptr);
            tau33Next = ComputeStressTerm(i, j, k+1, u, v, w, momentumEqn,
                                          DiffusionDir::z, cellSizeInv, K_turb, solverChoice, domain, bc_ptr);
            tau33Prev = ComputeStressTerm(i, j, k  , u, v, w, momentumEqn,
                                          DiffusionDir::z, cellSizeInv, K_turb, solverChoice, domain, bc_ptr);

            diffContrib = (tau31Next - tau31Prev) * dxInv  // Contribution to z-mom eqn from diffusive flux in x-dir
                        + (tau32Next - tau32Prev) * dyInv  // Contribution to z-mom eqn from diffusive flux in y-dir
                        + (tau33Next - tau33Prev) * dzInv; // Contribution to z-mom eqn from diffusive flux in z-dir
            if (solverChoice.molec_diff_type == MolecDiffType::ConstantAlpha)
            {
                diffContrib *= InterpolateFromCellOrFace(i, j, k, cons, Rho_comp, w(i,j,k), Coord::z, l_spatial_order) /
                               solverChoice.rho0_trans;
            }
            break;
        default:
            amrex::Abort("Error: Momentum equation is unrecognized");
    }

    return diffContrib;
}
#endif

AMREX_GPU_DEVICE
amrex::Real ComputeDiffusionFluxForState(const int &i, const int &j, const int &k,
                     const Array4<const Real>& cell_data,
                     const Array4<const Real>& cell_prim, const int & prim_index,
                     const amrex::Real invCellWidth,
                     const Array4<Real>& K_turb,
                     const SolverChoice &solverChoice,
                     const enum Coord& coordDir)
{
  // Get indices of states to left and right of the face on which we want the flux
  const int il = i - (coordDir == Coord::x);
  const int ir = i;
  const int jl = j - (coordDir == Coord::y);
  const int jr = j;
  const int kl = k - (coordDir == Coord::z);
  const int kr = k;

  // Get diffusion coefficients
  amrex::Real rhoAlpha_molec;
  int eddy_diff_idx;

  amrex::Real rhoFace;
  if (solverChoice.molec_diff_type == MolecDiffType::ConstantAlpha) {
    rhoFace = (cell_data(il, jl, kl, Rho_comp) + cell_data(ir, jr, kr, Rho_comp)) * 0.5;
  }

  switch(prim_index) {
      case PrimTheta_comp: // Potential Temperature
          if (solverChoice.molec_diff_type == MolecDiffType::ConstantAlpha) {
              rhoAlpha_molec = rhoFace * solverChoice.alpha_T;
          } else {
              rhoAlpha_molec = solverChoice.rhoAlpha_T;
          }
          if (coordDir == Coord::z) {
              eddy_diff_idx = EddyDiff::Theta_v;
          } else {
              eddy_diff_idx = EddyDiff::Theta_h;
          }
          break;

      case PrimKE_comp: // Turbulent KE
          rhoAlpha_molec = 0.;
          if (coordDir == Coord::z) {
              eddy_diff_idx = EddyDiff::KE_v;
          } else {
              eddy_diff_idx = EddyDiff::KE_h;
          }
          break;

      case PrimQKE_comp: // Turbulent QKE
          rhoAlpha_molec = 0.;
          if (coordDir == Coord::z) {
              eddy_diff_idx = EddyDiff::QKE_v;
          } else {
              eddy_diff_idx = EddyDiff::QKE_h;
          }
          break;

      case PrimScalar_comp: // Scalar
          if (solverChoice.molec_diff_type == MolecDiffType::ConstantAlpha) {
              rhoAlpha_molec = rhoFace * solverChoice.alpha_C;
          } else {
              rhoAlpha_molec = solverChoice.rhoAlpha_C;
          }
          if (coordDir == Coord::z) {
              eddy_diff_idx = EddyDiff::Scalar_v;
          } else {
              eddy_diff_idx = EddyDiff::Scalar_h;
          }
          break;
      default:
          amrex::Abort("Error: Diffusion term for the data index isn't implemented");
  }

  amrex::Real rhoAlpha = 0.0;

  if ( (solverChoice.molec_diff_type == MolecDiffType::Constant) ||
       (solverChoice.molec_diff_type == MolecDiffType::ConstantAlpha) ) {
    rhoAlpha += rhoAlpha_molec;
  }

  if ( (solverChoice.les_type == LESType::Smagorinsky) ||
       (solverChoice.les_type == LESType::Deardorff  ) ||
       (solverChoice.pbl_type == PBLType::MYNN25     ) ) {
    rhoAlpha += 0.5*(K_turb(ir,jr,kr,eddy_diff_idx) + K_turb(il,jl,kl,eddy_diff_idx));
  }

  // Compute the flux
  amrex::Real diffusionFlux = rhoAlpha * invCellWidth *
      (cell_prim(ir, jr, kr, prim_index) - cell_prim(il, jl, kl, prim_index));

  return diffusionFlux;
}

AMREX_GPU_DEVICE
Real
DiffusionSrcForState(const int &i, const int &j, const int &k,
                              const Array4<const Real>& cell_data,
                              const Array4<const Real>& cell_prim, const int & qty_index,
                              const Array4<Real>& xflux, const Array4<Real>& yflux, const Array4<Real>& zflux,
                              const GpuArray<Real, AMREX_SPACEDIM>& cellSizeInv,
                              const Array4<Real>& K_turb,
                              const SolverChoice &solverChoice)
{
  const amrex::Real dx_inv = cellSizeInv[0];
  const amrex::Real dy_inv = cellSizeInv[1];
  const amrex::Real dz_inv = cellSizeInv[2];

  const int prim_index = qty_index - RhoTheta_comp;

  // TODO : could be more efficient to compute and save all fluxes before taking divergence (now all fluxes are computed 2x);
  xflux(i+1,j,k,qty_index) = ComputeDiffusionFluxForState(i+1, j, k, cell_data, cell_prim, prim_index, dx_inv, K_turb, solverChoice, Coord::x);
  xflux(i  ,j,k,qty_index) = ComputeDiffusionFluxForState(i  , j, k, cell_data, cell_prim, prim_index, dx_inv, K_turb, solverChoice, Coord::x);

  yflux(i,j+1,k,qty_index) = ComputeDiffusionFluxForState(i, j+1, k, cell_data, cell_prim, prim_index, dy_inv, K_turb, solverChoice, Coord::y);
  yflux(i,j  ,k,qty_index) = ComputeDiffusionFluxForState(i, j  , k, cell_data, cell_prim, prim_index, dy_inv, K_turb, solverChoice, Coord::y);

  zflux(i,j,k+1,qty_index) = ComputeDiffusionFluxForState(i, j, k+1, cell_data, cell_prim, prim_index, dz_inv, K_turb, solverChoice, Coord::z);
  zflux(i,j,k  ,qty_index) = ComputeDiffusionFluxForState(i, j, k  , cell_data, cell_prim, prim_index, dz_inv, K_turb, solverChoice, Coord::z);

  Real diffusionSrc =
      (xflux(i+1,j,k,qty_index) - xflux(i  ,j,k,qty_index)) * dx_inv   // Diffusive flux in x-dir
     +(yflux(i,j+1,k,qty_index) - yflux(i,j  ,k,qty_index)) * dy_inv   // Diffusive flux in y-dir
     +(zflux(i,j,k+1,qty_index) - zflux(i,j,k  ,qty_index)) * dz_inv;  // Diffusive flux in z-dir

  return diffusionSrc;
}
