#=============================================================================
#  The compare_and_swap(target, old, new) function compares the value of the
#  target to the old and updates the target accordingly. If they are the same.
#  new is stored into target and one is returned. Otherwise, the function
#  just returns zero. Everything is done atomically.
#
#  Inital registers:
#  %rdi = target
#  %rsi = old
#  %rdx = new
#
.globl compare_and_swap
compare_and_swap:
	
    # move old to rax for cmpxchg
    movq    %rsi, %rax

    # perform cmpxchg on target memory and new
    lock    cmpxchg  %rdx, (%rdi)

    # check is the zero flag is set (1 if target == old)
    setz    %al

    # get the lowest bit (1 or 0 for return)
    and     $1, %rax

    ret

