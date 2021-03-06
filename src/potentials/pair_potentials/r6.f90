! @meta
!   shared
!   classtype:r6_t classname:r6 interface:potentials
! @endmeta

!>
!! The r^6 potential
!!
!! The r^6 potential
!<

#include "macros.inc"
#include "filter.inc"

module r6
  use libAtoms_module

  use ptrdict

  use logging
  use timer

  use neighbors
  use particles
  use filter

  implicit none

  private

  public :: r6_t
  type r6_t

     !
     ! Element on which to apply the force
     !

     character(MAX_EL_STR)  :: element1 = "*"
     character(MAX_EL_STR)  :: element2 = "*"
     integer                :: el1
     integer                :: el2

     !
     ! constants
     !
     
     real(DP) :: A = 1.0_DP
     real(DP) :: r0 = 0.0_DP
     real(DP) :: cutoff = 1.0_DP

  endtype r6_t


  public :: init
  interface init
     module procedure r6_init
  endinterface

  public :: del
  interface del
     module procedure r6_del
  endinterface

  public :: bind_to
  interface bind_to
     module procedure r6_bind_to
  endinterface

  public :: energy_and_forces
  interface energy_and_forces
     module procedure r6_energy_and_forces
  endinterface

  public :: register
  interface register
     module procedure r6_register
  endinterface

contains

  !>
  !! Constructor
  !!
  !! Constructor
  !<
  subroutine r6_init(this)
    implicit none

    type(r6_t), intent(inout)     :: this

    ! ---

  endsubroutine r6_init


  !>
  !! Destructor
  !!
  !! Destructor
  !<
  subroutine r6_del(this)
    implicit none

    type(r6_t), intent(inout)  :: this

    ! ---

  endsubroutine r6_del


  !>
  !! Initialization
  !!
  !! Initialization
  !<
  subroutine r6_bind_to(this, p, nl, ierror)
    implicit none

    type(r6_t),        intent(inout) :: this
    type(particles_t), intent(in)    :: p
    type(neighbors_t), intent(inout) :: nl
    integer, optional, intent(inout) :: ierror

    ! ---

    integer :: i, j, k

    ! ---

    this%el1 = filter_from_string(this%element1, p)
    this%el2 = filter_from_string(this%element2, p)

    write (ilog, '(A)')            "- r6_init -"
    call filter_prlog(this%el1, p, indent=5)
    call filter_prlog(this%el2, p, indent=5)
    write (ilog, '(5X,A,F20.10)')  "A      = ", this%A
    write (ilog, '(5X,A,F20.10)')  "r0     = ", this%r0
    write (ilog, '(5X,A,F20.10)')  "cutoff = ", this%cutoff

    do i = 1, p%nel
       do j = 1, p%nel
          if (IS_EL2(this%el1, i) .and. IS_EL2(this%el2, j)) then
             call request_interaction_range(nl, this%cutoff, i, j)
          endif
       enddo
    enddo

    write (ilog, *)

  endsubroutine r6_bind_to


  !>
  !! Compute the force
  !!
  !! Compute the force
  !<
  subroutine r6_energy_and_forces(this, p, nl, epot, f, wpot, epot_per_at, epot_per_bond, f_per_bond, wpot_per_at, wpot_per_bond, ierror)
    implicit none

    type(r6_t),         intent(inout) :: this
    type(particles_t),  intent(in)    :: p
    type(neighbors_t),  intent(inout) :: nl
    real(DP),           intent(inout) :: epot
    real(DP),           intent(inout) :: f(3, p%maxnatloc)  !< forces
    real(DP),           intent(inout) :: wpot(3, 3)
    real(DP), optional, intent(inout) :: epot_per_at(p%maxnatloc)
    real(DP), optional, intent(inout) :: epot_per_bond(nl%neighbors_size)
    real(DP), optional, intent(inout) :: f_per_bond(3, nl%neighbors_size)
    real(DP), optional, intent(inout) :: wpot_per_at(3, 3, p%maxnatloc)
    real(DP), optional, intent(inout) :: wpot_per_bond(3, 3, nl%neighbors_size)
    integer,  optional, intent(inout) :: ierror

    ! ---

    integer   :: i, jn, j
    real(DP)  :: dr(3), df(3), dw(3, 3)
    real(DP)  :: cut_sq, abs_dr, for, en

    ! ---

    call timer_start("r6_force")

#ifdef MDCORE_INTERFACE
    call update(nl, p, ierror)
    PASS_ERROR(ierror)
#endif

    cut_sq = this%cutoff**2

    do i = 1, p%nat
       do jn = nl%seed(i), nl%last(i)
          j = nl%neighbors(jn)

          if (i > j) then
             if ( ( IS_EL(this%el1, p, i) .and. IS_EL(this%el2, p, j) ) .or. &
                  ( IS_EL(this%el2, p, i) .and. IS_EL(this%el1, p, j) ) ) then

                DIST_SQ(p, nl, i, jn, dr, abs_dr)

                if (abs_dr < cut_sq) then
                   abs_dr = sqrt(abs_dr)

                   en  = this%A / (this%r0 + abs_dr)**6
                   for = 6*en/(this%r0 + abs_dr)

                   epot = epot + en
                   df   = for * dr/abs_dr

                   VEC3(f, i) = VEC3(f, i) + df
                   VEC3(f, j) = VEC3(f, j) - df

                   dw    = -outer_product(dr, df)
                   wpot  = wpot + dw

                   if (present(wpot_per_at)) then
                      dw                       = dw/2
                      wpot_per_at(1:3, 1:3, i) = wpot_per_at(1:3, 1:3, i) + dw
                      wpot_per_at(1:3, 1:3, j) = wpot_per_at(1:3, 1:3, j) + dw
                   endif

                endif
             endif
          endif
       enddo
    enddo

    call timer_stop("r6_force")

  endsubroutine r6_energy_and_forces


  subroutine r6_register(this, cfg, m)
    use, intrinsic :: iso_c_binding

    implicit none

    type(r6_t),  target      :: this
    type(c_ptr), intent(in)  :: cfg
    type(c_ptr), intent(out) :: m

    ! ---

    m = ptrdict_register_section(cfg, CSTR("r6"), &
         CSTR("r^6 potential (London dispersion forces). Computes: A*(r0+r)^-6"))

    call ptrdict_register_string_property(m, c_loc(this%element1), &
         MAX_EL_STR, CSTR("el1"), CSTR("First element."))
    call ptrdict_register_string_property(m, c_loc(this%element2), &
         MAX_EL_STR, CSTR("el2"), CSTR("Second element."))

    call ptrdict_register_real_property(m, c_loc(this%A), CSTR("A"), &
         CSTR("Prefactor A."))
    call ptrdict_register_real_property(m, c_loc(this%r0), CSTR("r0"), &
         CSTR("Offset r0."))
    call ptrdict_register_real_property(m, c_loc(this%cutoff), CSTR("cutoff"), &
         CSTR("Cutoff length."))

  endsubroutine r6_register

endmodule r6
