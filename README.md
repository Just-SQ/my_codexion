*This project has been created as part of the 42 curriculum by asaqi.*

# Codexion

## Description

Codexion is a concurrency simulation written in C with POSIX threads.

A number of coders sit around a circular table. Between every pair of
neighbours lies one USB dongle, so there are exactly as many dongles as
coders. A coder repeats the cycle **compile -> debug -> refactor**, and
compiling requires holding **both** of its dongles at the same time: the
one on its left and the one on its right.

Two extra rules make the problem harder than the classic dining
philosophers:

* **Cooldown** - once released, a dongle cannot be taken again until
  `dongle_cooldown` milliseconds have passed.
* **Arbitration** - when both neighbours want the same dongle, the winner
  is chosen by an explicit scheduling policy, `fifo` or `edf`, backed by a
  priority queue.

A coder burns out if it does not *start* a new compile within
`time_to_burnout` milliseconds of the start of its previous one. The
simulation ends either when a coder burns out or when every coder has
completed `number_of_compiles_required` compiles.

Each coder is one thread. One additional **monitor** thread watches the
deadlines and stops the simulation. There are no global variables: every
piece of shared state lives in a single `t_sim` structure passed by
pointer.

## Instructions

Build:

```
make            # build ./codexion
make clean      # remove object files
make fclean     # remove object files and the binary
make re         # fclean + all
```

The project compiles with `cc -Wall -Wextra -Werror -pthread`.

Run:

```
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown \
           scheduler
```

All eight arguments are mandatory. Times are in milliseconds and must be
non-negative integers; `number_of_coders` and
`number_of_compiles_required` must be at least 1; `scheduler` must be
exactly `fifo` or `edf`.

Examples:

```
./codexion 1 800 200 200 200 10 0 fifo      # 1 coder, 1 dongle: must burn out ~800
./codexion 5 2000 200 200 200 10 0 fifo     # completes, no burnout
./codexion 5 2000 200 200 200 7 0 edf       # same with the edf scheduler
./codexion 5 500 200 200 200 10 0 fifo      # infeasible by design: must burn out ~500
./codexion 5 3000 200 200 200 10 800 fifo   # heavy cooldown, coders must queue
```

Output format, one line per state change:

```
timestamp_in_ms X has taken a dongle
timestamp_in_ms X is compiling
timestamp_in_ms X is debugging
timestamp_in_ms X is refactoring
timestamp_in_ms X burned out
```

## Blocking cases handled

**Deadlock (Coffman's four conditions).** Deadlock needs all four to hold
at once; this design breaks two of them.

* *Hold and wait* is eliminated: a coder never holds one dongle while
  waiting for the other. Both dongles are acquired **atomically** inside a
  single critical section, or neither is. `take_dongles()` checks both and
  only then marks both as taken.
* *Circular wait* is eliminated: all three mutexes are always taken in the
  fixed order `table -> log -> data`, and `table` and `log` are never held
  at the same time. The wait-for graph has no cycle.

*Mutual exclusion* and *no preemption* remain, which is fine - breaking any
single condition is enough.

**Livelock.** Coders never spin retrying; they block on a condition
variable and are woken only when the shared state actually changes.

**Starvation.** Each dongle sits between exactly two coders, so its waiting
queue never holds more than two requests - waiting is bounded by
construction. Ordering inside the queue is a strict total order (`fifo`
uses a global arrival ticket; `edf` uses the burnout deadline with the
ticket as tie-breaker), which guarantees that the waiter with the globally
smallest key is at the head of *both* of its queues and can always
proceed. Under `edf` the system is additionally self-correcting: a coder
that has just compiled gets the latest possible deadline and therefore
goes to the back of the line, while a coder that has been waiting keeps
the earliest deadline and is served first.

**Idle resources / priority inversion by blocking.** Strict queue order
alone is correct but wasteful: a waiter that is first in line for a dongle
but cannot start - because its *other* dongle is still held or still
cooling down - would reserve a dongle nobody can use. `can_take()`
therefore allows a coder to be served ahead of the queue head **only when
that head cannot start right now**. Policy order is fully respected among
the coders that are actually able to proceed.

**Cooldown.** A released dongle stores an absolute `free_at` timestamp and
is refused until that time passes. Because nothing signals the passing of
time, a coder blocked only by a cooldown arms its own alarm with
`pthread_cond_timedwait()` set to that timestamp; a coder blocked by
anything else waits indefinitely and is woken by a broadcast.

**Precise burnout detection.** A dedicated monitor thread polls every
300 microseconds, so a burnout is detected far inside the 10 ms budget.
The deadline itself is reset inside the same critical section that grants
the dongles, so the monitor can never observe a coder that already holds
its dongles but still carries a stale deadline.

**Log serialization and the final line.** Every message is printed while
holding the `log` mutex, so two lines can never interleave. The three
lines that announce a compile (two `has taken a dongle` plus
`is compiling`) are printed inside a single critical section so no other
coder can slip a line between them. The monitor sets the stop flag *while
holding the log mutex* and only then prints `burned out`; every coder that
reaches the logging function afterwards finds the flag set and prints
nothing, which guarantees the burnout line is the last line of output.

**Single coder.** With one coder there is only one dongle, so compiling is
impossible by definition. That case is handled explicitly: the coder logs
`has taken a dongle` once and then waits for the monitor to declare its
burnout, instead of falling into the acquisition loop.

**Memory.** The only heap allocations are the coder and dongle arrays.
Both are freed on every exit path, including argument errors and thread
creation failures. Verified with `valgrind --leak-check=full`.

## Thread synchronization mechanisms

### Primitives

Three `pthread_mutex_t` and one `pthread_cond_t`, all inside `t_sim`:

| Primitive | Protects |
|---|---|
| `table` | every dongle: `is_taken`, `free_at`, its waiting queue, and the arrival `ticket` counter |
| `log`   | standard output, and the write of the stop flag |
| `data`  | the stop flag and every coder's `last_start` and `compiles` |
| `cond`  | paired with `table`; signals that the state of the table has changed |

`data` is a *leaf*: no other lock is ever taken while holding it. Together
with the fixed order `table -> log -> data` this makes the lock graph a
DAG.

### How races are prevented

*Dongle duplication.* `is_taken` is read and written only under `table`.
The test "are both dongles free, off cooldown and mine to take?" and the
assignment `is_taken = 1` on both happen in the **same** critical section,
so two coders can never observe the same dongle as free.

*Counters.* `last_start` and `compiles` are written by a coder and read by
the monitor, always under `data`. `ticket` is incremented under `table`,
which is what makes it a genuine global arrival order.

*Output.* Two `printf` calls from different threads can never interleave
because both must hold `log`.

### Waiting: the condition variable

Acquisition uses the classic monitor pattern - a predicate re-tested in a
loop:

```
lock(table)
push my request into both waiting queues
while (!stopped)
{
    if (both dongles are takeable by me)
        take them, unlock, return
    wait on cond            /* releases table while asleep */
}
unlock(table)
```

The `while` is required, not stylistic: POSIX allows spurious wakeups, and
a broadcast wakes every waiter although only one can win, so the predicate
must be re-checked after every wake. Waking too early is always harmless.

Two kinds of wait are used, chosen by what is actually blocking the coder:

* blocked because a dongle is *held*, or because it is not yet this
  coder's turn - `pthread_cond_wait()`, since another thread will
  eventually change that state and broadcast;
* blocked only because a dongle is *cooling down* - `pthread_cond_timedwait()`
  with the deadline set to `free_at`, because the passing of time is not an
  event any thread will signal. This works because `now_ms()` is built on
  `gettimeofday()` and the default condition variable clock is
  `CLOCK_REALTIME`, so both use the same time base.

### Signalling discipline

The shared state is always modified **first**, and the broadcast is issued
**afterwards**, while still holding the mutex that guards the predicate. A
signal carries no information and only wakes threads that are already
waiting; if the state were not updated first, the wakeup would be lost and
the waiter would sleep forever. `pthread_cond_broadcast()` is used rather
than `pthread_cond_signal()` because several coders may be waiting on the
same dongle and the releaser cannot know whose predicate it has satisfied.

### Coder / monitor communication

Communication is one-way and flag-based rather than message-based. The
monitor reads `last_start` and `compiles` under `data`; to stop the run it
takes `log`, sets `is_stopped` under `data`, prints the burnout line, and
releases `log`, then takes `table` and broadcasts so that every coder
blocked on the condition variable wakes up, observes the flag and exits.
`pthread_join()` on all threads then guarantees that no thread is still
running when the mutexes are destroyed and the arrays freed.

### Scheduling

`heap.c` implements a small binary min-heap ordered by the active policy:
arrival order for `fifo`, burnout deadline with arrival order as
tie-breaker for `edf`. Each dongle owns one heap. Its capacity is 2, which
is not a shortcut but the exact bound: a dongle lies between exactly two
coders, so no third coder can ever queue for it. Both keys are frozen when
the request is queued - a waiting coder's deadline cannot change, since
only starting a compile moves `last_start` - so no re-heapifying is ever
needed.

## Resources

* `man 3 pthread_create`, `pthread_mutex_lock`, `pthread_cond_wait`,
  `pthread_cond_timedwait`, `gettimeofday`
* *The Little Book of Semaphores*, Allen B. Downey - chapter on the dining
  philosophers.
* Dijkstra, E. W., *Hierarchical Ordering of Sequential Processes* (1971) -
  the original dining philosophers problem.
* Coffman, Elphick, Shoshani, *System Deadlocks* (1971) - the four
  necessary conditions for deadlock.
* Liu & Layland, *Scheduling Algorithms for Multiprogramming in a Hard
  Real-Time Environment* (1973) - earliest deadline first.
* Butenhof, D. R., *Programming with POSIX Threads* - condition variable
  usage and the predicate loop.
* `valgrind` documentation for `memcheck`, `helgrind` and `drd`.

### Use of AI

AI was used as a reviewer and a discussion partner, not as a code
generator. Every line of C in this repository was written by hand.
Specifically, AI was used to:

* discuss the overall architecture before implementation - the choice of a
  single table mutex plus one condition variable over one mutex per
  dongle, and why acquiring both dongles atomically removes the hold-and-wait
  condition;
* review each file after writing it, which surfaced real bugs: an
  uninitialized pointer in the request builder, an `int` holding a
  millisecond timestamp, a mutex left locked on one return path, a
  broadcast issued before the flag it was meant to announce, and a queue
  removal that popped the head instead of the caller's own request;
* run and interpret stress tests and the analysis tools (`valgrind`,
  `helgrind`, `drd`, `-fsanitize=thread`), and investigate two scheduling
  problems found that way: a configuration where strict queue order left
  the dongles idle, and the starvation that appears if the resulting
  bypass rule is made too permissive;
* explain the reasoning behind `pthread_cond_timedwait`, the predicate
  loop and lock ordering.

Suggestions were checked against the subject and the code before being
accepted; several were tested, measured and rejected.
