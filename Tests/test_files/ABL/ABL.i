# ------------------  INPUTS TO MAIN PROGRAM  -------------------
max_step = 10

amrex.fpe_trap_invalid = 1

fabarray.mfiter_tile_size = 1024 1024 1024

# PROBLEM SIZE & GEOMETRY
geometry.is_periodic = 1 1 0
geometry.prob_extent =  1024     1024    1024
amr.n_cell           =    64       64      64

# >>>>>>>>>>>>>  BC KEYWORDS <<<<<<<<<<<<<<<<<<<<<<
# Interior, SimSlipWall, Symmetry, SlipWall, NoSlipWall
# >>>>>>>>>>>>>  BC KEYWORDS <<<<<<<<<<<<<<<<<<<<<<
zlo.type = "NoSlipWall"
zhi.type = "SlipWall"

# TIME STEP CONTROL
erf.use_native_mri     = 1
erf.fixed_dt           = 0.1  # fixed time step depending on grid resolution

# DIAGNOSTICS & VERBOSITY
erf.sum_interval   = 1       # timesteps between computing mass
erf.v              = 1       # verbosity in ERF.cpp
amr.v                = 1       # verbosity in Amr.cpp
amr.data_log         = datlog

# REFINEMENT / REGRIDDING
amr.max_level       = 0       # maximum level number allowed

# CHECKPOINT FILES
amr.check_file      = chk        # root name of checkpoint file
amr.check_int       = 100        # number of timesteps between checkpoints

# PLOTFILES
amr.plot_file       = plt        # root name of plotfile
amr.plot_int        = 10         # number of timesteps between plotfiles
amr.plot_vars        =  density rhoadv_0 x_velocity y_velocity z_velocity
amr.derive_plot_vars = pressure temp theta

# SOLVER CHOICE
erf.alpha_T = 0.0
erf.alpha_C = 1.0
erf.use_gravity = false

erf.molec_diff_type = "None"
erf.les_type        = "Smagorinsky"
erf.Cs              = 0.1

erf.spatial_order = 2

# PROBLEM PARAMETERS
prob.rho_0 = 1.0
prob.A_0 = 1.0

prob.U_0 = 10.0
prob.V_0 = 0.0
prob.W_0 = 0.0

# Higher values of perturbations lead to instability
# Instability seems to be coming from BC
prob.U_0_Pert_Mag = 0.08
prob.V_0_Pert_Mag = 0.08 #
prob.W_0_Pert_Mag = 0.0
