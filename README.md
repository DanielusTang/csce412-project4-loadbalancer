# AggieForge Traffic Cloud — CSCE 412 Project 4 (Load Balancer)

Daniel Tang · CSCE 412 Cloud Computing · Texas A&M University · Fall 2026

A C++ simulation of a company load balancer: a request queue, a pool of web servers that
scales between 50x and 80x the active server count, random request arrivals, an IP-range
firewall, colour-coded event logging and a configuration file. Part of the AggieForge brand
family (Dev Cloud, Container Cloud, Cluster Cloud, Traffic Cloud).

**Tagline:** Everything you need to stay balanced — every request, the right server.

## Repository layout

| Path | Contents |
|---|---|
| `design/` | **Pre-deliverable (due Oct 18, 2026):** `index.html` + `style.css` design document with the UML class diagram and workflow diagram. Hosted at `https://people.tamu.edu/~<netid>/csce412/project4/design/`. |
| `branding/` | Logo lockups (light + maroon), icon, palette and event-colour table. |
| `tools/` | `make-diagrams.py` regenerates `design/uml.svg` and `design/flow.svg`; `render-png.sh` rasterises an SVG with headless Chrome. |
| `runbooks/` | `upload-people-tamu.md` — how to publish the design folder and verify it. |
| *(Part 2, due Oct 25, 2026)* | `Makefile`, `config.txt`, `Doxyfile`, `main.cpp`, `Request.h`, `RequestQueue.*`, `WebServer.*`, `LoadBalancer.*`, `IPBlocker.*`, `Logger.*`, `Config.*`, `Switch.*` (bonus) — built to the design in `design/index.html`. |

## Design summary

- `Request` struct (ipIn, ipOut, time, jobType, arrivedCycle); `RequestQueue` FIFO wrapper.
- `WebServer` processes one request at a time: `assign()`, `tick()` returns true when done.
- `LoadBalancer` owns its queue and server pool, references a shared `IPBlocker`, `Logger`
  and `Config`; `stepCycle()` = arrival -> firewall -> servers tick -> dispatch -> autoscale
  (cooldown `n`) -> status heartbeat.
- `Logger` quiet vs verbose, ANSI colour on console, plain text in the log file.
- Bonus `Switch` routes 'S' and 'P' jobs to separate load balancers with separate statistics.

Regenerate diagrams: `python3 tools/make-diagrams.py design`
