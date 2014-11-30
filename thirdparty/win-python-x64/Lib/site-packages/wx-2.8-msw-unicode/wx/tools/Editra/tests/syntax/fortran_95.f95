! Fortran 95 Syntax Test File
! Some comments about this file

! Hello World
program HelloWorld
  write (*,*) 'Hello, world!'   ! This is an inline comment
end program HelloWorld

program HelloWorld2
      ihello = 0
   1  if (ihello.NE.10)     ! This line shows the label style
        write (*,*) "Hello, World2!"
        ihello = ihello + 1
        goto 1
      end if
end program HelloWord2
   
!----------------------------------------------------------
subroutine Swap_Real(a1, a2)

   implicit none

!  Input/Output
   real, intent(inout) :: a1(:), a2(:)

!  Locals
   integer :: lb(1), & !Lower bound
              ub(1)    !Upper bound
   integer i
   real a

!  Get bounds
   lb = lbound(a1)
   ub = ubound(a1)

!  Swap
   do i = lb(1), ub(1)
      a = a1(i)
      a1(i) = a2(i)
      a2(i) = a
   end do

end subroutine Swap_Real

!----------------------------------------------------------
