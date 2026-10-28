# Decoder profiler callback lifetime repair

The operation profiler registers a callback borrowing a stack OpTimer. Existing
normal and compute-status-error exits unregister it, but a C++ exception from
compute could leave the scheduler pointing at expired stack data.

A scoped registration now unregisters before status handling and on exception
unwinding. Disabled profiling does not change the scheduler callback. This
assumes the existing exclusive/non-reentrant decoder scheduler; it does not
introduce inference locking, CUDA recovery or shared-backend safety.

Focused test links the production guard to a recording scheduler and exercises
normal completion, exception unwinding, disabled registration, and independent
schedulers. It passed compiled with C++17, -Wall -Wextra -Werror. The complete
modified decoder translation unit passed g++ -fsyntax-only using actual engine
headers. Both ran in a disposable CPU-only CUDA12.8.1 development container on
Defiant, network disabled, read-only source, one CPU/1GB. No host packages added.
The first execution was blocked by a noexec tmpfs; an executable temporary mount
resolved that. Initial integration syntax errors were fixed before the passing
run. Registered as decoder_callback_scope in CTest.

This is a source lifetime repair, not proof of the original illegal-address
root cause. Full Dax timing-enabled engine build and GPU stress remain pending.
No live engine or voice service was changed.
