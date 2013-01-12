!! ======================================================================
!! MDCORE - Interatomic potential library
!! https://github.com/pastewka/mdcore
!! Lars Pastewka, lars.pastewka@iwm.fraunhofer.de, and others
!! See the AUTHORS file in the top-level MDCORE directory.
!!
!! Copyright (2005-2013) Fraunhofer IWM
!! This software is distributed under the GNU General Public License.
!! See the LICENSE file in the top-level MDCORE directory.
!! ======================================================================

  !>
  !! The BOP class
  !<
  public :: BOP_TYPE
  type BOP_TYPE

     !
     ! Current database
     !

     type(BOP_DB_TYPE)  :: db = Erhart_PRB_71_035211_SiC

     !
     ! String to a reference
     !
     character     :: ref(ALBE_BOP_MAX_REF) = "*"

     integer       :: Z2db(MAX_Z)

     !
     ! Precomputed constants
     !

     real(DP)  :: c_sq(ALBE_BOP_MAX_PAIRS)
     real(DP)  :: d_sq(ALBE_BOP_MAX_PAIRS)
     real(DP)  :: c_d(ALBE_BOP_MAX_PAIRS) 

     real(DP)  :: VR_f(ALBE_BOP_MAX_PAIRS)
     real(DP)  :: expR(ALBE_BOP_MAX_PAIRS)
     real(DP)  :: VA_f(ALBE_BOP_MAX_PAIRS)
     real(DP)  :: expA(ALBE_BOP_MAX_PAIRS)

     !
     ! Cutoff parameters
     !

     real(DP)  :: cut_in_h(ALBE_BOP_MAX_PAIRS)
     real(DP)  :: cut_in_h2(ALBE_BOP_MAX_PAIRS)
     real(DP)  :: cut_in_l(ALBE_BOP_MAX_PAIRS)
     real(DP)  :: cut_in_fca(ALBE_BOP_MAX_PAIRS)
#ifndef EXP_BOP
     real(DP)  :: cut_in_fc(ALBE_BOP_MAX_PAIRS)
#endif

#ifdef SCREENING

! The other cutoff are identical!
#define cut_ar_h  cut_out_h

     real(DP)  :: cut_out_h(ALBE_BOP_MAX_PAIRS)
     real(DP)  :: cut_out_l(ALBE_BOP_MAX_PAIRS)
     real(DP)  :: cut_out_fca(ALBE_BOP_MAX_PAIRS)
#ifndef EXP_BOP
     real(DP)  :: cut_out_fc(ALBE_BOP_MAX_PAIRS)
#endif

     real(DP)  :: cut_bo_h(ALBE_BOP_MAX_PAIRS)
     real(DP)  :: cut_bo_l(ALBE_BOP_MAX_PAIRS)
     real(DP)  :: cut_bo_fca(ALBE_BOP_MAX_PAIRS)
#ifndef EXP_BOP
     real(DP)  :: cut_bo_fc(ALBE_BOP_MAX_PAIRS)
#endif

     real(DP)  :: max_cut_sq(ALBE_BOP_MAX_PAIRS)

     real(DP)  :: Cmin(ALBE_BOP_MAX_PAIRS)
     real(DP)  :: Cmax(ALBE_BOP_MAX_PAIRS)
     real(DP)  :: dC(ALBE_BOP_MAX_PAIRS)
     real(DP)  :: C_dr_cut(ALBE_BOP_MAX_PAIRS)

     real(DP)  :: screening_threshold  = log(1d-6)
     real(DP)  :: dot_threshold        = 1e-10
#endif

     !
     ! Bond-order stuff
     !

     real(DP)  :: bo_exp(ALBE_BOP_MAX_PAIRS)
     real(DP)  :: bo_fac(ALBE_BOP_MAX_PAIRS)
     real(DP)  :: bo_exp1(ALBE_BOP_MAX_PAIRS)

     !
     ! Counters
     !

     logical  :: neighbor_list_allocated  = .false.
     integer  :: it                       = 0

     !
     ! Internal neighbor lists
     !

     integer                :: nebmax = 40
     integer                :: nebavg = 40

     integer, allocatable   :: neb(:)
     integer, allocatable   :: nbb(:)
#ifndef LAMMPS
     integer, allocatable   :: dcell(:)
#endif

     integer, allocatable   :: bndtyp(:)
     real(DP), allocatable  :: bndlen(:)
     real(DP), allocatable  :: bndnm(:, :)
     real(DP), allocatable  :: cutfcnar(:), cutdrvar(:)

#ifdef SCREENING
     real(DP), allocatable  :: cutfcnbo(:), cutdrvbo(:)
     ! "screened" neighbor list (all neighbors of a bond which sit in the
     ! screening cutoff)
     integer, allocatable   :: sneb_seed(:)
     integer, allocatable   :: sneb_last(:)
     integer, allocatable   :: sneb(:)
     integer, allocatable   :: sbnd(:)

     ! for force calculation
     real(DP), allocatable  :: sfacbo(:)

     real(DP), allocatable  :: cutdrarik(:), cutdrarjk(:)
     real(DP), allocatable  :: cutdrboik(:), cutdrbojk(:)
#endif

  endtype BOP_TYPE


  public :: init
  interface init
     module procedure INIT_FUNC
  endinterface

  public :: del
  interface del
     module procedure DEL_FUNC
  endinterface

  public :: bind_to
  interface bind_to
     module procedure BIND_TO_FUNC
  endinterface

  public :: energy_and_forces
  interface energy_and_forces
     module procedure COMPUTE_FUNC
  endinterface


