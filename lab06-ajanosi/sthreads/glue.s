#============================================================================
# Keep a pointer to the main scheduler context.  This variable should be
# initialized to %rsp, which is done in the __sthread_start routine.
#
        .data
        .align 8
scheduler_context:      .quad   0


#============================================================================
# __sthread_switch is the main entry point for the thread scheduler.
# It has three parts:
#
#    1. Save the context of the current thread on the stack.
#       The context includes all of the integer registers and RFLAGS.
#
#    2. Call __sthread_scheduler (the C scheduler function), passing the
#       context as an argument.  The scheduler stack *must* be restored by
#       setting %rsp to the scheduler_context before __sthread_scheduler is
#       called.
#
#    3. __sthread_scheduler will return the context of a new thread.
#       Restore the context, and return to the thread.
#
        .text
        .align 8
        .globl __sthread_switch
__sthread_switch:

        # push registers
        pushq   %rax
        pushq   %rcx
        pushq   %rdx
        pushq   %rbx
        pushq   %rsi
        pushq   %rdi
        pushq   %rbp
        pushq   %r8
        pushq   %r9
        pushq   %r10
        pushq   %r11
        pushq   %r12
        pushq   %r13
        pushq   %r14
        pushq   %r15
        pushf

        # Call the high-level scheduler with the current context as an argument
        movq    %rsp, %rdi
        movq    scheduler_context(%rip), %rsp
        call    __sthread_scheduler

        # The scheduler will return a context to start.
        # Restore the context to resume the thread.
__sthread_restore:

        # move return to stack pointer
        movq    %rax, %rsp

        # pop registers
        popf
        popq    %r15
        popq    %r14
        popq    %r13
        popq    %r12
        popq    %r11
        popq    %r10
        popq    %r9
        popq    %r8
        popq    %rbp
        popq    %rdi
        popq    %rsi
        popq    %rbx
        popq    %rdx
        popq    %rcx
        popq    %rax
        ret


#============================================================================
# Initialize a process context, given:
#    1. the stack for the process
#    2. the function to start
#    3. its argument
# The context should be consistent with that saved in the __sthread_switch
# routine.
#
# A pointer to the newly initialized context is returned to the caller.
# (This is the thread's stack-pointer after its context has been set up.)
#
# This function is described in more detail in sthread.c.
#
#
        .align 8
        .globl __sthread_initialize_context
__sthread_initialize_context:

        # set aside rsp
        movq    %rsp, %r15

        # move stack to rsp for switch
        movq    %rdi, %rsp

        # to be able to store address of finish
        leaq __sthread_finish(%rip), %rcx

        # push f and __sthread_finish
        pushq   %rcx
        pushq   %rsi

        # push fillers
        pushq   $0      # rax
        pushq   $0      # rcx
        pushq   $0      # rdx
        pushq   $0      # rbx
        pushq   $0      # rsi
        pushq   %rdx    # rdi - need args here
        pushq   $0      # rbp
        pushq   $0      # r8
        pushq   $0      # r9
        pushq   $0      # r10
        pushq   $0      # r11
        pushq   $0      # r12
        pushq   $0      # r13
        pushq   $0      # r14
        pushq   $0      # r15
        pushf           # flags

        # move stack pointer to output
        movq    %rsp, %rax

        # move back original rsp
        movq    %r15, %rsp

        ret


#============================================================================
# The start routine initializes the scheduler_context variable, and calls
# the __sthread_scheduler with a NULL context.
#
# The scheduler will return a context to resume.
#
        .align 8
        .globl __sthread_start
__sthread_start:
        # Remember the context
        movq    %rsp, scheduler_context(%rip)

        # Call the scheduler with no context
        movl    $0, %edi  # Also clears upper 4 bytes of %rdi
        call    __sthread_scheduler

        # Restore the context returned by the scheduler
        jmp     __sthread_restore

