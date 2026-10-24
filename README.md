# AggieForge Traffic Cloud {#mainpage}

\image html logo.svg width=560px

**AggieForge Traffic Cloud** is a C++ simulation of the load balancer that sits in front of a
large company's web-server farm. Requests arrive from random source IP addresses, each needing a
random number of clock cycles of work; the balancer hands every request to a free web server as
quickly as possible while keeping just enough servers switched on.

*Everything you need to stay balanced - every request, the right server.*

Daniel Tang - CSCE 412 Cloud Computing - Texas A&M University - Fall 2026 - Project 4.
Fourth product line of the AggieForge brand family (Dev Cloud, Container Cloud, Cluster Cloud,
Traffic Cloud). Design document (pre-deliverable, AGF-DOC-005):
<https://people.tamu.edu/~leyuantangcollege/csce412/project4/design/>

---

## What the simulation does

- Takes the **number of web servers** and the **run time in clock cycles** from the user and starts
  with a full queue of `servers * 100` requests.
- Generates requests with a random requester IP, destination IP, processing time and job type
  (`'P'` processing or `'S'` streaming) and keeps injecting new ones at random times.
- Runs web servers that process one request for its full duration and then ask for the next one.
- **Autoscales** the server pool: queue below `50 x servers` removes one idle server, queue above
  `80 x servers` adds one; after either change the balancer rests for `n` cycles so it never
  thrashes. Hard `min_servers` / `max_servers` bounds keep the pool sane.
- **Blocks** requests from configured IP ranges (firewall / DOS prevention) at admission time and
  counts the rejections.
- Writes a **colour-coded event stream** in verbose mode and a **START / END summary** in both
  modes, to the console and to a plain-text log file (no colour codes in the file).
- **Bonus:** a Switch routes streaming jobs to one load balancer and processing jobs to another,
  each with its own server pool, autoscaling and statistics.

## Build and run

```
make                      # builds ./loadbalancer with g++ -std=c++17 -Wall -Wextra -O2
./loadbalancer            # reads config.txt, asks for servers and cycles (Enter keeps the default)
./loadbalancer --verbose  # coloured event stream + STATUS heartbeat, for the demo
./loadbalancer --no-prompt --servers 20 --cycles 100000          # deliverable run, no questions
./loadbalancer --no-prompt --set switch_enabled=true             # bonus: two balancers
./loadbalancer other.txt --set cooldown_n=5 --set log_file=x.txt # any key can be overridden
make docs                 # Doxygen HTML into docs/html/
make clean
```

Command-line flags: `[config.txt] [--verbose|--quiet] [--servers N] [--cycles N] [--set key=value]... [--no-prompt]`.
Flags win over the file; the interactive prompts win over both.

## Configuration (`config.txt`)

Every tunable lives in the configuration file, so experiments never require a recompile.

| Key | Default | Meaning |
|---|---|---|
| `servers` | 20 | Initial web servers (the program also asks interactively) |
| `cycles` | 100000 | Run length in clock cycles (also asked interactively) |
| `initial_multiplier` | 100 | Starting queue = `servers * initial_multiplier` |
| `task_time_min` / `task_time_max` | 3 / 80 | Task-time range in cycles |
| `arrival_probability` | 0.6 | Expected new requests per cycle (values above 1 give several per cycle) |
| `cooldown_n` | 50 | Rest period `n` after adding or removing a server |
| `scale_low` / `scale_high` | 50 / 80 | Per-server queue bounds for the autoscaler |
| `min_servers` / `max_servers` | 1 / 200 | Hard bounds on the pool |
| `blocked_ranges` | `10.0.0.0/8, 192.168.100.0/24` | Firewall ranges (CIDR, bare address or dotted prefix) |
| `mode` | quiet | `quiet` (summaries only) or `verbose` (events + STATUS) |
| `color` | true | ANSI colours on the console; never written to the file |
| `log_file` | lb_log.txt | Plain-text log path |
| `status_every` | 1000 | Verbose: STATUS heartbeat interval in cycles |
| `seed` | 412 | RNG seed; `0` draws a random seed each run |
| `switch_enabled` | false | Bonus: route `'S'` / `'P'` to separate balancers |

## Design

The class contract was fixed in the design document before any code was written; the code builds
exactly that design.

| Class | Responsibility |
|---|---|
| Request | Plain data: `ipIn`, `ipOut`, `time`, `jobType`, plus `arrivedCycle` for wait-time statistics |
| RequestQueue | FIFO wrapper around `std::queue<Request>` with a peak-depth counter |
| WebServer | Processes one request at a time; `tick()` returns true once when the request finishes |
| LoadBalancer | Owns its queue and server pool; references the shared Config, IPBlocker and Logger; `stepCycle()` = arrivals, firewall, servers tick, dispatch, autoscale, heartbeat |
| IPBlocker | Firewall: ranges stored as (base, mask) integers, one bitwise AND per check |
| Logger | Single sink: coloured console lines, plain-text file, quiet vs verbose level |
| Config | `key = value` file reader with typed getters and defaults |
| Random.h | RequestFactory helpers: random IPv4, task time, job type, arrivals |
| Switch | Bonus: routes by job type to several balancers stepped on one clock |

**Design choices.** A cooldown after every scaling action prevents thrashing; hard server bounds stop
the pool from scaling to zero or without limit; a busy server is never removed, so the only
discarded requests are firewall rejections and those are counted; blocking happens before a request
touches the queue, so a flood from a blocked range costs one comparison and no server time;
dependency injection (Config, IPBlocker and Logger passed by reference) is what lets two balancers
share one firewall and one log while keeping separate statistics.

Event colours (console only): ARRIVE / ASSIGN green, DONE cyan, BLOCK red, SCALE+ / SCALE- yellow,
STATUS magenta.

## Deliverable log: 20 servers, 100,000 cycles, quiet mode

`evidence/baseline_20x100000_quiet.txt` (seed 412, defaults above):

```
=== AggieForge Traffic Cloud - AGF-LB START ===
Servers (initial)     : 20
Run length            : 100000 cycles
Starting queue size   : 2000
Task time range       : 3 - 80 cycles
Arrival probability   : 0.60 / cycle
Scale thresholds      : 50 - 80 per server, cooldown n = 50
Blocked IP ranges     : 10.0.0.0/8, 192.168.100.0/24

=== AggieForge Traffic Cloud - AGF-LB END (after 100000 cycles) ===
Ending queue size     : 1385
Requests generated    : 61818   (P: 31157, S: 30661)
Requests processed    : 60169
Requests rejected     : 241   (firewall)
Requests in service   : 23   (busy servers at end)
Active servers (end)  : 23   (busy 23, idle 0)
Inactive (removed)    : 3   of 26 servers ever created
Scale-ups / downs     : 6 / 3
Min / max servers     : 20 / 26
Min / max queue       : 0 / 2008
Avg wait in queue     : 1431.8 cycles
Server utilization    : 100.0 %
```

The pool grew from 20 to 26 servers while the starting backlog drained, shed three servers when the
queue dipped, and finished with 23 servers and a queue of 1385, inside the 50x-80x band (1150-1840).

## Experiments and capacity guide

All runs: 20 servers, 100,000 cycles, one knob changed from the defaults (`tools/run-experiments.sh`,
summaries in `evidence/`).

| Experiment | Setting | Servers end (min/max) | Queue end | Scale +/- | Rejected | Avg wait |
|---|---|---|---|---|---|---|
| Baseline | defaults | 23 (20/26) | 1385 | 6 / 3 | 241 | 1432 |
| Arrival rate | 0.3 / cycle | 13 (11/25) | 40 | 7 / 14 | 135 | 1834 |
| Arrival rate | 0.9 / cycle | 38 (20/38) | 1455 | 18 / 0 | 327 | 2587 |
| Arrival rate | 1.5 / cycle | 62 (20/62) | 4692 | 42 / 0 | 577 | 3136 |
| Task length | max 20 cycles | 7 (6/23) | 472 | 6 / 19 | 245 | 395 |
| Task length | max 300 cycles | 91 (20/91) | 7217 | 71 / 0 | 232 | 11194 |
| Cooldown n | 5 | 24 (20/26) | 1849 | 8 / 4 | 241 | 1616 |
| Cooldown n | 500 | 23 (20/26) | 928 | 6 / 3 | 241 | 1410 |
| Thresholds | 70 - 90 | 25 (20/25) | 1789 | 5 / 0 | 241 | 3271 |
| Thresholds | 20 - 120 | 25 (20/25) | 2523 | 5 / 0 | 241 | 4420 |
| Firewall | block 0.0.0.0/1 | 13 (9/25) | 110 | 9 / 16 | 29903 | 1737 |
| Bonus switch | two pools of 10 | P 12, S 12 | 442 / 358 | 3/1 each | 124 / 117 | - |

**Capacity rule.** The pool settles near `arrival_probability x mean task time` servers: with a
mean task of 41.5 cycles, 0.6 arrivals per cycle needs about 25 servers, 0.9 needs about 37, 1.5
needs about 62, and a 20-cycle task cap (mean 11.5) needs only 7. The autoscaler found each of these
on its own.

**Guide for a balanced farm.**

1. Size the pool for the arrival rate times the mean task time, then let the 50x-80x rule trim it.
2. Keep the thresholds close together (50-80) - widening to 20-120 or raising to 70-90 left
   requests waiting two to three times longer for the same server count.
3. Use a moderate cooldown. `n = 5` scaled more often and ended with a deeper queue; `n = 500`
   reacted slowly but stayed stable; `n = 50` was the sweet spot at this load.
4. Watch the queue's trend, not its level: utilization is 100 % in every overloaded run, so a
   growing queue with the pool at `max_servers` (1.5 arrivals, 300-cycle tasks) is the signal to
   raise the ceiling or shorten tasks.
5. The firewall is free capacity: blocking half the address space cut arrivals in half and the
   pool shrank from 25 to 13 servers with no request lost from the queue.

## Repository layout

| Path | Contents |
|---|---|
| `*.h`, `*.cpp`, `Makefile`, `config.txt` | The simulator (Part 2) |
| `Doxyfile`, `branding/doxygen-agf.css` | Doxygen configuration; `make docs` writes `docs/html/` |
| `design/` | Pre-deliverable design document (HTML/CSS, UML and flow diagrams) |
| `branding/` | Logos, palette and event-colour table |
| `evidence/` | Quiet-mode summaries from the experiment suite, incl. the deliverable log |
| `tools/` | `run-experiments.sh`, diagram generators |
| `runbooks/` | Publishing the design folder and the Doxygen site on people.tamu.edu; demo script |
