#ifndef PlaneAverage_H
#define PlaneAverage_H

#include "AMReX_MultiFab.H"
#include "DirectionSelector.H"

class PlaneAverage {
public:
    explicit PlaneAverage(const amrex::MultiFab* field_in,
                          const amrex::Geometry geom_in,
                          int axis_int);
    PlaneAverage() = default;
    ~PlaneAverage() = default;

    void operator()();

    /** evaluate line average at specific location for any average component */
    amrex::Real line_average_interpolated(amrex::Real x, int comp) const;

    /** change precision of text file output */
    void set_precision(int p) { m_precision = p; };

    amrex::Real dx() const { return m_dx; };
    amrex::Real xlo() const { return m_xlo; };

    int axis() const { return m_axis; };
    int level() const { return m_level; };
    int ncomp() const { return m_ncomp; };
    int ncell_plane() const { return m_ncell_plane; };
    int ncell_line() const { return m_ncell_line; };

    const amrex::Vector<amrex::Real>& line_average() const
    {
        return m_line_average;
    };
    void line_average(int comp, amrex::Vector<amrex::Real>& l_vec);
    const amrex::Vector<amrex::Real>& line_centroids() const
    {
        return m_line_xcentroid;
    };

    const amrex::MultiFab& field() const { return *m_field; };

protected:
    int m_ncomp; /** number of average components */

    /** line storage for the average velocity and tracer variables */
    amrex::Vector<amrex::Real> m_line_average;

    amrex::Vector<amrex::Real> m_line_xcentroid; /** line storage for centroids of each cell along a line*/

    amrex::Real m_dx;  /** mesh spacing in axis direction*/
    amrex::Real m_xlo; /** bottom of domain in axis direction */

    int m_ncell_plane; /** number of cells in plane */
    int m_ncell_line;  /** number of cells along line */

    int m_precision = 4; /** precision for line plot text file */
    const int m_level = 0; /** level for plane averaging for now fixed at level=0 */

    const amrex::MultiFab* m_field;
    amrex::Geometry m_geom;
    const int m_axis;

public:
    /** fill line storage with averages */
    template <typename IndexSelector>
    void compute_averages(const IndexSelector& idxOp, const amrex::MultiFab& mfab);
};

inline PlaneAverage::PlaneAverage(
             const amrex::MultiFab* field_in,
             amrex::Geometry geom_in,
             int axis_in)
             : m_field(field_in)
             , m_geom(geom_in)
             , m_axis(axis_in)
{
    AMREX_ALWAYS_ASSERT(m_axis >= 0 && m_axis < AMREX_SPACEDIM);

    m_xlo = m_geom.ProbLo(m_axis);
    m_dx  = m_geom.CellSize(m_axis);
    m_ncomp = m_field->nComp();

    amrex::Box domain = m_geom.Domain();
    amrex::IntVect dom_lo(domain.loVect());
    amrex::IntVect dom_hi(domain.hiVect());

    m_ncell_line = dom_hi[m_axis] - dom_lo[m_axis] + 1;

    m_ncell_plane = 1;
    for (int i = 0; i < AMREX_SPACEDIM; ++i) {
       if (i != m_axis) m_ncell_plane *= (dom_hi[i] - dom_lo[i] + 1);
    }

    m_line_average.resize(static_cast<size_t>(m_ncell_line) * m_ncomp, 0.0);
    m_line_xcentroid.resize(m_ncell_line);

    for (int i = 0; i < m_ncell_line; ++i) {
       m_line_xcentroid[i] = m_xlo + (i + 0.5) * m_dx;
    }
}

inline amrex::Real
PlaneAverage::line_average_interpolated(amrex::Real x, int comp) const
{
    AMREX_ALWAYS_ASSERT(comp >= 0 && comp < m_ncomp);

    amrex::Real c = 0.0;
    int ind = 0;

    if (x > m_xlo + 0.5 * m_dx) {
        ind = static_cast<int>(floor((x - m_xlo) / m_dx - 0.5));
        const amrex::Real x1 = m_xlo + (ind + 0.5) * m_dx;
        c = (x - x1) / m_dx;
    }

    if (ind + 1 >= m_ncell_line) {
        ind = m_ncell_line - 2;
        c = 1.0;
    }

    AMREX_ALWAYS_ASSERT(ind >= 0 && ind + 1 < m_ncell_line);

    return m_line_average[m_ncomp * ind + comp] * (1.0 - c) +
           m_line_average[m_ncomp * (ind + 1) + comp] * c;
}

inline void PlaneAverage::line_average(int comp, amrex::Vector<amrex::Real>& l_vec)
{
    AMREX_ALWAYS_ASSERT(comp >= 0 && comp < m_ncomp);

    for (int i = 0; i < m_ncell_line; i++)
        l_vec[i] = m_line_average[m_ncomp * i + comp];
}

inline void PlaneAverage::operator()()
{
    std::fill(m_line_average.begin(), m_line_average.end(), 0.0);
    switch (m_axis) {
    case 0:
        compute_averages(XDir(), *m_field);
        break;
    case 1:
        compute_averages(YDir(), *m_field);
        break;
    case 2:
        compute_averages(ZDir(), *m_field);
        break;
    default:
        amrex::Abort("axis must be equal to 0, 1, or 2");
        break;
    }
}

template <typename IndexSelector>
inline void PlaneAverage::compute_averages(
            const IndexSelector& idxOp, const amrex::MultiFab& mfab)
{
    const amrex::Real denom = 1.0 / (amrex::Real)m_ncell_plane;
    amrex::AsyncArray<amrex::Real> lavg(m_line_average.data(), m_line_average.size());
    amrex::Real* line_avg = lavg.data();
    const int ncomp = m_ncomp;

#ifdef _OPENMP
#pragma omp parallel if (Gpu::notInLaunchRegion())
#endif
    for (amrex::MFIter mfi(mfab, amrex::TilingIfNotGPU()); mfi.isValid(); ++mfi) {
        amrex::Box bx = mfi.tilebox();
        auto fab_arr = mfab.const_array(mfi);
        amrex::Box pbx = PerpendicularBox<IndexSelector>(bx, amrex::IntVect{0, 0, 0});

        ParallelFor(amrex::Gpu::KernelInfo().setReduction(true), pbx, [=]
           AMREX_GPU_DEVICE( int p_i, int p_j, int p_k,
                             amrex::Gpu::Handler const& handler) noexcept {
            // Loop over the direction perpendicular to the plane.
            // This reduces the atomic pressure on the destination arrays.

            amrex::Box lbx = ParallelBox<IndexSelector>(bx, amrex::IntVect{p_i, p_j, p_k});

            for (int k = lbx.smallEnd(2); k <= lbx.bigEnd(2); ++k) {
                for (int j = lbx.smallEnd(1); j <= lbx.bigEnd(1); ++j) {
                    for (int i = lbx.smallEnd(0); i <= lbx.bigEnd(0); ++i) {
                        int ind = idxOp.getIndx(i, j, k);
                        for (int n = 0; n < ncomp; ++n) {
                            amrex::Gpu::deviceReduceSum(&line_avg[ncomp * ind + n],
                                            fab_arr(i, j, k, n) * denom, handler);
                        }
                     }
                 }
             }
       });
    }

    lavg.copyToHost(m_line_average.data(), m_line_average.size());
    amrex::ParallelDescriptor::ReduceRealSum(m_line_average.data(), m_line_average.size());
}
#endif /* PlaneAverage_H */
