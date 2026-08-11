*This project has been created as part of the 42 curriculum by asaqi.*

# Codexion

## Description

Codexion is a concurrency simulation written in C with POSIX threads.

A number of **coders** sit around a circular table in a shared co-working hub. Between
each pair of neighbours lies a single **USB dongle**, so there are exactly as many
dongles as coders. To compile their quantum code, a coder must hold **two** dongles at
the same time — the one on their left and the one on their right. Once they finish
compiling they release both dongles, then debug, then refactor, and immediately try to
compile again.

Two rules make this hard:

- A dongle enters a **cooldown** after being released, during which nobody may take it.
- A coder **burns out** if they do not *start* a new compile within `time_to_burnout`
  milliseconds of the start of their previous one.

The simulation ends either when a coder burns out, or when every coder has compiled at
least `number_of_compiles_required` times.

This is the classic Dining Philosophers problem with two additions that change the
character of the solution considerably: a resource cooldown, and a configurable
arbitration policy (`fifo` or `edf`) that decides who wins a contested dongle.

## Instructions

### Build

```sh
make          # build ./codexion
make clean    # remove object files
make fclean   # remove object files and the binary
make re       # fclean + build
```

Compiled with `cc -Wall -Wextra -Werror` and linked with `-pthread`. No relinking.

### Run

```sh
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| argument | meaning |
| --- | --- |
| `number_of_coders` | number of coders, and of dongles |
| `time_to_burnout` | ms allowed between two compile *starts* before burnout |
| `time_to_compile` | ms spent compiling, holding both dongles |
| `time_to_debug` | ms spent debugging |
| `time_to_refactor` | ms spent refactoring |
| `number_of_compiles_required` | stop once every coder reaches this count |
| `dongle_cooldown` | ms a dongle stays unavailable after being released |
| `scheduler` | `fifo` or `edf` |

All arguments are mandatory. Negative values, non-integers, values above `INT_MAX`, a
coder count below 1, and any scheduler other than `fifo` or `edf` are rejected with a
usage message and exit status 1.

### Examples

```sh
# one coder, one dongle: can never hold two, must burn out around 800 ms
./codexion 1 800 200 200 200 10 0 fifo

# comfortable parameters: nobody burns out, stops after 10 compiles each
./codexion 5 2000 200 200 200 10 0 fifo

# infeasible by design: a 600 ms cycle cannot fit in a 500 ms deadline
./codexion 5 500 200 200 200 10 0 fifo

# heavy cooldown: coders must queue for the dongles
./codexion 5 3000 200 200 200 10 800 fifo
./codexion 5 3000 200 200 200 10 800 edf
```

### Output format

```
timestamp_in_ms X has taken a dongle
timestamp_in_ms X is compiling
timestamp_in_ms X is debugging
timestamp_in_ms X is refactoring
timestamp_in_ms X burned out
```

Timestamps are milliseconds since the start of the simulation. Every `is compiling`
line is immediately preceded by exactly two `has taken a dongle` lines for the same
coder, and `burned out` is always the last line printed.

## Blocking cases handled

### Deadlock — Coffman's four conditions

A deadlock requires all four of Coffman's conditions to hold simultaneously. Two of them
cannot be removed here, so the design breaks the other two.

| condition | status |
| --- | --- |
| **Mutual exclusion** | Not removable. A dongle is genuinely exclusive — that is the whole point of the resource. |
| **No preemption** | Not removable. A dongle is never taken back from a coder mid-compile. |
| **Hold and wait** | **Broken.** A coder never holds one dongle while blocking on the other. |
| **Circular wait** | **Broken.** Since nothing is held while waiting, no cycle of waiting-for relations can form. |

Both are broken by the same mechanism: **atomic acquisition**. `take_dongles()` claims
both dongles inside a single critical section, or claims neither. A coder that cannot
have both simply sleeps holding nothing.

This was not the first design. The original version acquired the two dongles one at a
time, in ascending index order — which does break circular wait and does prevent
deadlock. But it leaves hold-and-wait intact, and with a large `dongle_cooldown` that
turned out to be almost as damaging: a coder could sit on one dongle for two full
seconds while waiting for the second, starving the neighbour who needed only the dongle
being squatted on. Atomic acquisition removes the problem at its root, and has the extra
benefit that the two `has taken a dongle` lines are always genuinely simultaneous.

### Starvation

Fairness is enforced by a **priority queue (binary heap)** attached to every dongle.

- A coder building a request receives a monotonically increasing `order` ticket and a
  `deadline` equal to `last_compile_start + time_to_burnout`.
- The request is pushed into the heap of **both** wanted dongles and **stays there while
  the coder sleeps**. Keeping your place in the queue while blocked is what guarantees
  you are eventually served — a coder that left the queue and re-entered would take a
  fresh, worse ticket and could be overtaken indefinitely.
- Under `fifo` the heap orders by `order`; under `edf` it orders by `deadline`, falling
  back to `order` when deadlines tie, so the policy is fully deterministic.
- Because a request carries the same `order` and `deadline` into both heaps, a coder has
  a consistent priority on both of its dongles rather than winning two independent races.

**Head-of-line blocking** is handled explicitly. Requiring a coder to be the head of both
queues is too strong: a coder sitting at the front of one queue may itself be unable to
move because its *other* dongle is busy, and it would then block a neighbour from a
dongle that is completely free. `can_take()` therefore yields to the queue head only
when that head can genuinely proceed — that is, when both of *its* dongles are free and
past cooldown. A head that is stuck is bypassed.

This does not reintroduce starvation. The bypassed coder keeps its older, better ticket,
while the coder that overtook it must re-queue with a fresh, worse one after compiling.
As soon as the blocked coder's other dongle frees up, it wins.

### Cooldown

Each dongle carries an `available_at` timestamp, set to `now + dongle_cooldown` on
release. A coder may take a dongle only when `now >= available_at`.

Cooldown creates a wait with no event behind it: the dongle is free, nobody holds it, and
no thread will do anything at the moment the cooldown expires — so no broadcast is
coming. Waiters therefore use `pthread_cond_timedwait()` with an absolute deadline
computed from the later of the two dongles' `available_at`, waking themselves at exactly
the right time instead of relying on a signal that never arrives.

### Precise burnout detection

A dedicated **monitor thread** polls every 500 µs, comfortably inside the 10 ms window
the subject requires. For each coder it reads `last_compile_start` and `compiles` under
that coder's own mutex, and declares a burnout when the elapsed time strictly exceeds
`time_to_burnout` — a coder that starts compiling exactly on its deadline has started
*within* the allowed window and survives.

Burnout detection cannot be left to the coders themselves: a coder that is about to burn
out is, by definition, asleep waiting for a dongle and cannot check its own clock or
print anything. An outside observer is required.

On stop, the monitor sets the `stop` flag and then **broadcasts on the table condition
variable**. Without that broadcast, coders sleeping in `pthread_cond_wait()` would never
wake, and `pthread_join()` would hang forever — a flag alone cannot reach a sleeping
thread.

### Log serialization

All output passes through a single `log_lock` mutex, so two lines can never interleave.

Three further details matter:

- The **timestamp is read inside the lock**. Reading it before acquiring the lock allows a
  thread to compute an earlier time, get preempted, and print after a thread with a later
  time — producing timestamps that appear to go backwards.
- The two `has taken a dongle` lines and the following `is compiling` line are printed in
  **one single critical section**, so another coder can never slip a line between them.
- Once `stop` is set, `log_state()` suppresses further output, checking the flag *inside*
  the lock so no thread can pass the check and print afterwards. The burnout message is
  the one exception and is always printed, which guarantees it is the final line.

### Single-coder edge case

With `number_of_coders 1` there is one coder and one dongle, and the coder's left and
right dongle are the same object. This is detected by pointer comparison. The coder takes
the single dongle, logs it, and can never obtain a second one — so it waits and burns out
at `time_to_burnout`, as the subject requires. Without this special case the coder would
attempt to acquire the same dongle twice and wait on itself.

## Thread synchronization mechanisms

### Threads

- **N coder threads**, one per coder, created with `pthread_create()` and each given a
  pointer to its own `t_coder`. That struct holds a back-pointer to the shared `t_sim`,
  so a single `void *` gives a thread both its identity and the shared world.
- **One monitor thread**, created last so it never observes a half-initialised array, and
  joined after all coders.

`start_ms` and every coder's `last_compile_start` are initialised in `main` — from a
single shared timestamp — *before* any thread is created. Setting them inside the thread
routine would let the monitor read uninitialised memory, and would also give later-
starting coders extra life relative to earlier ones.

### Mutexes

| primitive | protects |
| --- | --- |
| `sim->table` | all dongle state: `held`, `available_at`, and the `waiters` heaps |
| `sim->table_cond` | condition variable paired with `sim->table` |
| `sim->state_lock` | the `stop` flag and the `ticket` counter |
| `sim->log_lock` | serialises all output |
| `coder->state` | that coder's `last_compile_start` and `compiles` |

`t_args` is deliberately not protected by anything: it is written once during parsing and
is strictly read-only afterwards, so it needs no synchronization at all.

### Why one table mutex rather than one mutex per dongle

Per-dongle mutexes were the first design and had to be abandoned, for a concrete reason.

The wait condition here spans **two** dongles — "both are free and both are past
cooldown". `pthread_cond_wait()` atomically releases exactly **one** mutex. With separate
locks, a coder would have to unlock the second dongle manually and then sleep on the
first dongle's condition variable, leaving a window in which another thread releases the
second dongle and broadcasts on a condition variable nobody is listening to. The wakeup
is lost and the coder sleeps forever. POSIX also states that using more than one mutex
with the same condition variable is undefined behaviour, so a shared condvar is not a way
out either.

Since the predicate covers both dongles, the mutex guarding it must cover both dongles.
Each dongle's state is still protected by a mutex, as the subject requires — it is simply
the same mutex.

### Lock ordering

Two nesting patterns exist, and both keep `state_lock` innermost:

```
table    ->  state_lock      (a waiter calling stopped())
log_lock ->  state_lock      (log_state() checking whether to stay silent)
```

`state_lock` is never held while acquiring `table` or `log_lock`, so no cycle can form.
The monitor respects this: it sets `stop` under `state_lock`, **releases it**, and only
then takes `table` to broadcast — sequential acquisition, never nested.

### How race conditions are prevented

- **`stop` flag.** Written by the monitor, read constantly by every coder. Every access
  goes through `state_lock` via `stopped()`. An unsynchronized read would be undefined
  behaviour, and in practice the compiler may cache the value in a register so a coder
  never observes the change at all.
- **`last_compile_start` and `compiles`.** Written by their coder, read by the monitor,
  both under that coder's `state` mutex. These are exactly the fields burnout detection
  depends on.
- **Dongle state.** `held`, `available_at`, and the heaps are only ever touched while
  holding `table`. The check "can I take both?" and the act of taking them happen in the
  same critical section, so two coders can never both conclude that the same dongle is
  free.
- **`ticket` counter.** Incremented under `state_lock`, so no two requests can receive the
  same arrival order.
- **Output.** Timestamp and `printf` both inside `log_lock`.

### Thread-safe communication between coders and the monitor

Coders and the monitor never call each other; they communicate only through shared state
guarded by the mutexes above.

- Coder → monitor: a coder publishes `last_compile_start` and `compiles` under its own
  mutex; the monitor samples them on its 500 µs tick.
- Monitor → coders: the monitor raises `stop` under `state_lock`, then broadcasts on
  `table_cond`. Sleeping coders wake, re-test `stopped()`, remove their pending requests
  from the heaps, release the table mutex and return, allowing `pthread_join()` to
  complete.

Every wait is written as a `while` loop that re-tests its predicate after waking, never
an `if` — condition variables permit spurious wakeups, and a broadcast wakes every waiter
even though only one can proceed.

### Verification

```sh
cc -Wall -Wextra -Werror -pthread -fsanitize=thread   *.c -o codexion_tsan
cc -Wall -Wextra -Werror -pthread -fsanitize=address  *.c -o codexion_asan
valgrind --tool=helgrind ./codexion 5 2000 200 200 200 5 0 fifo
valgrind --leak-check=full ./codexion 5 2000 200 200 200 5 0 fifo
```

ThreadSanitizer reports no data races under both `fifo` and `edf`, and AddressSanitizer
reports no leaks or invalid accesses, including the single-coder case.

## Resources

### Concurrency and the underlying problem

- E. W. Dijkstra, *Hierarchical Ordering of Sequential Processes* (1971) — the original
  Dining Philosophers formulation.
- E. G. Coffman, M. Elphick, A. Shoshani, *System Deadlocks* (1971) — the four conditions
  used in the deadlock analysis above.
- A. B. Downey, *The Little Book of Semaphores* — accessible treatment of the classic
  synchronization problems.
- C. L. Liu and J. W. Layland, *Scheduling Algorithms for Multiprogramming in a Hard
  Real-Time Environment* (1973) — the origin of Earliest Deadline First.

### POSIX threads

- `man pthread_create`, `pthread_join`, `pthread_mutex_init`, `pthread_mutex_lock`
- `man pthread_cond_wait`, `pthread_cond_timedwait`, `pthread_cond_broadcast`
- The Open Group Base Specifications, *pthread_cond_wait* — in particular the rule that a
  condition variable must be used with only one mutex, and the requirement to re-test the
  predicate after waking.
- `man gettimeofday`, `man clock_gettime`, `man usleep`

### Data structures

- Standard binary heap / priority queue material (sift-up, sift-down, array
  representation) — the heap here is bounded at two entries, since each dongle lies
  between exactly two coders and can therefore never have more than two pending requests.
