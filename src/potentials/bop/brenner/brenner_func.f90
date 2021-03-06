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
!! Cut-off function: fC(r), dfC(r)
!<
elemental subroutine fCin(this, ijpot, dr, val, dval)
  implicit none

  type(BOP_TYPE), intent(in)  :: this
  integer, intent(in)         :: ijpot
  real(DP), intent(in)        :: dr
  real(DP), intent(out)       :: val
  real(DP), intent(out)       :: dval

  ! ---

  real(DP)  :: arg
#ifdef EXP_CUTOFF
  real(DP)  :: arg2
#endif

  ! ---

  if (dr < this%cut_in_l(ijpot)) then
     val   = 1.0_DP
     dval  = 0.0_DP
#ifdef EXP_CUTOFF
  else
     arg  = this%cut_in_fca(ijpot)*(dr-this%cut_in_l(ijpot))
     arg2 = arg*arg
     val  = exp(-arg*arg2)
     dval = -3*this%cut_in_fca(ijpot)*arg2*val
  endif
#else
  else if (dr > this%cut_in_h(ijpot)) then
     val   = 0.0_DP
     dval  = 0.0_DP
  else
     arg   = this%cut_in_fca(ijpot)*( dr-this%cut_in_l(ijpot) )
     val   = 0.5_DP * ( 1.0_DP + cos( arg ) )
     dval  = this%cut_in_fc(ijpot) * sin( arg )
  endif
#endif

endsubroutine fCin


#ifdef SCREENING

!>
!! Outer cut-off function: fC(r), dfC(r)
!<
subroutine fCar(this, ijpot, dr, val, dval)
  implicit none

  type(BOP_TYPE), intent(in)  :: this
  integer, intent(in)         :: ijpot
  real(DP), intent(in)        :: dr
  real(DP), intent(out)       :: val
  real(DP), intent(out)       :: dval

  ! ---

  real(DP)  :: arg
#ifdef EXP_CUTOFF
  real(DP)  :: arg2
#endif

  ! ---

  if (dr < this%cut_out_l(ijpot)) then
     val   = 1.0_DP
     dval  = 0.0_DP
#ifdef EXP_CUTOFF
  else
     arg  = this%cut_out_fca(ijpot)*(dr-this%cut_out_l(ijpot))
     arg2 = arg*arg
     val  = exp(-arg*arg2)
     dval = -3*this%cut_out_fca(ijpot)*arg2*val
  endif
#else
  else if (dr > this%cut_out_h(ijpot)) then
     val   = 0.0_DP
     dval  = 0.0_DP
  else
     arg   = this%cut_out_fca(ijpot)*( dr-this%cut_out_l(ijpot) )
     val   = 0.5_DP * ( 1.0_DP + cos( arg ) )
     dval  = this%cut_out_fc(ijpot) * sin( arg )
  endif
#endif

endsubroutine fCar


!>
!! Outer cut-off function: fC(r), dfC(r)
!<
subroutine fCbo(this, ijpot, dr, val, dval)
  implicit none

  type(BOP_TYPE), intent(in)  :: this
  integer, intent(in)         :: ijpot
  real(DP), intent(in)        :: dr
  real(DP), intent(out)       :: val
  real(DP), intent(out)       :: dval

  ! ---

  real(DP)  :: arg
#ifdef EXP_CUTOFF
  real(DP)  :: arg2
#endif

  ! ---

  if (dr < this%cut_bo_l(ijpot)) then
     val   = 1.0_DP
     dval  = 0.0_DP
#ifdef EXP_CUTOFF
  else
     arg  = this%cut_bo_fca(ijpot)*(dr-this%cut_bo_l(ijpot))
     arg2 = arg*arg
     val  = exp(-arg*arg2)
     dval = -3*this%cut_bo_fca(ijpot)*arg2*val
  endif
#else
  else if (dr > this%cut_bo_h(ijpot)) then
     val   = 0.0_DP
     dval  = 0.0_DP
  else
     arg   = this%cut_bo_fca(ijpot)*( dr-this%cut_bo_l(ijpot) )
     val   = 0.5_DP * ( 1.0_DP + cos( arg ) )
     dval  = this%cut_bo_fc(ijpot) * sin( arg )
  endif
#endif

endsubroutine fCbo

#endif


!>
!! Attractive potential: VA(r), dVA(r)
!<
elemental subroutine VA(this, ijpot, dr, val, dval)
  implicit none

  type(BOP_TYPE), intent(in)  :: this
  integer, intent(in)         :: ijpot
  real(DP), intent(in)        :: dr
  real(DP), intent(out)       :: val
  real(DP), intent(out)       :: dval

  ! ---

  real(DP)  :: expval

  ! ---

  expval  = exp(-this%expA(ijpot)*(dr-this%db%r0(ijpot)))
  val     = -this%VA_f(ijpot)*expval
  dval    = this%VA_f(ijpot)*this%expA(ijpot)*expval

endsubroutine VA


!>
!! Repulsive potential: VA(r), dVA(r)
!<
elemental subroutine VR(this, ijpot, dr, val, dval)
  implicit none

  type(BOP_TYPE), intent(in)  :: this
  integer, intent(in)         :: ijpot
  real(DP), intent(in)        :: dr
  real(DP), intent(out)       :: val
  real(DP), intent(out)       :: dval

  ! ---

  real(DP)  :: expval

  ! ---

  expval  = exp(-this%expR(ijpot)*(dr-this%db%r0(ijpot)))
  val     = this%VR_f(ijpot)*expval
  dval    = -this%VR_f(ijpot)*this%expR(ijpot)*expval

endsubroutine VR


!>
!! Angular contribution to the bond order: g(cos(theta)), dg(cos(theta))
!<
elemental subroutine g(this, ktypj, ktypi, ktypk, ijpot, ikpot, costh, val, dval_dcosth)
  implicit none

  type(BOP_TYPE), intent(in)  :: this
  integer, intent(in)         :: ktypj
  integer, intent(in)         :: ktypi
  integer, intent(in)         :: ktypk
  integer, intent(in)         :: ijpot
  integer, intent(in)         :: ikpot
  real(DP), intent(in)        :: costh
  real(DP), intent(out)       :: val
  real(DP), intent(out)       :: dval_dcosth

  ! ---

  real(DP)  :: h

  ! ---


  h           = this%d_sq(ikpot)+(this%db%h(ikpot)+costh)**2
  val         = this%db%gamma(ikpot)*(1+this%c_d(ikpot)-this%c_sq(ikpot)/h)
  dval_dcosth = 2*this%db%gamma(ikpot)*this%c_sq(ikpot)*(this%db%h(ikpot)+costh)/(h**2)

endsubroutine g


!>
!! Bond order function
!<
subroutine bo(this, ktypi, ijpot, zij, fcij, faij, bij, dfbij)
  implicit none

  type(BOP_TYPE), intent(in)  :: this
  integer, intent(in)         :: ktypi
  integer, intent(in)         :: ijpot
  real(DP), intent(in)        :: zij
  real(DP), intent(in)        :: fcij
  real(DP), intent(in)        :: faij
  real(DP), intent(out)       :: bij
  real(DP), intent(out)       :: dfbij

  ! ---

  real(DP) :: arg

  ! ---

  if (this%db%n(ijpot) == 1.0_DP) then

     arg    = 1.0_DP + zij
     bij    = arg ** this%bo_exp(ijpot)
     dfbij  = this%bo_fac(ijpot) * fcij * faij * arg ** this%bo_exp1(ijpot)

  else

     arg    = 1.0_DP + zij ** this%db%n(ijpot)
     bij    = arg ** this%bo_exp(ijpot)

     if (zij == 0.0_DP) then
        dfbij  = 0.0_DP
     else
        dfbij  = &
             this%bo_fac(ijpot) * fcij * faij &
             * zij ** ( this%db%n(ijpot) - 1.0_DP ) &
             * arg ** this%bo_exp1(ijpot)
     endif

  endif

endsubroutine bo


!>
!! Length dependent contribution to the bond order: h(dr), dh(dr)
!<
elemental subroutine h(this, ktypj, ktypi, ktypk, ijpot, ikpot, dr, val, dval)
  implicit none

  type(BOP_TYPE), intent(in)  :: this
  integer, intent(in)         :: ktypj
  integer, intent(in)         :: ktypi
  integer, intent(in)         :: ktypk
  integer, intent(in)         :: ijpot
  integer, intent(in)         :: ikpot
  real(DP), intent(in)        :: dr
  real(DP), intent(out)       :: val
  real(DP), intent(out)       :: dval

  ! ---

  real(DP)  :: mu, arg
  integer   :: m

  ! ---

  mu = this%db%mu(ikpot)

  if (mu == 0.0_DP) then
     val  = 1.0_DP
     dval = 0.0_DP
  else
     m = this%db%m(ikpot)

     if (m == 1) then
        val  = exp(2*mu*dr)
        dval = 2*mu*val
     else
        if (m == 3) then
           arg  = 2*mu*dr
           val  = exp(arg*arg*arg)
           dval = 6*mu * arg*arg * val 
        else
           val  = exp((2*mu*dr)**m)
           dval = 2*mu*m * (2*mu*dr)**(m-1) * val
        endif
     endif
  endif

endsubroutine h


!>
!! Generate an index for this *pair* if elements
!<
elemental function Z2pair(this, i, j)
  implicit none

  type(BOP_TYPE), intent(in)  :: this
  integer, intent(in)         :: i
  integer, intent(in)         :: j
  integer                     :: Z2pair

  ! ---

  Z2pair = PAIR_INDEX(i, j, this%db%nel)

endfunction Z2pair
  
