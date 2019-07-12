#=============================================================================
# The output(data, size) function takes in a string message and the size.
# The function moves the message to %rsi, size to %rdx, and 1 in %rdi
# This sets up the syscall for writing, which then outputs the string.
# The output of syscall is 0, stored at %rax.
#
.global output
output:
	movq    %rsi,%rdx       # move count to rdx
	movq    %rdi,%rsi       # move message to buf
	movq    $1,%rdi         # fd
	movq    $1,%rax         # sys_write with arguments in right spot
    syscall
   
