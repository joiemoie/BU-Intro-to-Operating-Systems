In each thread, I store a thread ID, an environment variable, and a pointer to the bottom of the stack.
Thread create creates a create and then modifies the environment variable in the thread.
I set the stack pointer to the top of a malloc stack and the program counter to start_routine.
I then create an alarm which calls the schedule() function every 50 milliseconds.
In the schedule() function, I search for the next available thread. Then I setjmp the current thread and longjmp to the next.
In thread exit, I free the stack created from the malloc and set the state to EXITED.

Specific Implementation: Creating a new thread does interrupts the current thread and schedules a new one.

The limitations of this library is that it exits the program as soon as one single thread exits.