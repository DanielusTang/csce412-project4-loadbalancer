# Demo video script - AggieForge Traffic Cloud (Part 2, "Demo on Video", 80 pts)

Target length 6-8 minutes. Rubric: design choice 20, classes and structures 20, user input and
execution 20, dynamic change in number of servers 20, bonus attempt (switch) extra.
Record with QuickTime (File > New Screen Recording) or the same flow used for Project 3.

## Before recording

```
cd "/Users/danielus/Fall 2026/CSCE 412/projects/project_4/csce412-project4"
make clean && make
open design/index.html                    # or the live URL
open docs/html/index.html                 # Doxygen site (after `make docs`)
```
Set the terminal to a large font and a dark background; colours are the point of segment 4.

## 1. Intro (30 s) - branding
Show the design page header. "I'm Daniel Tang. AggieForge Traffic Cloud is the fourth AggieForge
product: a C++ simulation of a company load balancer. Everything you need to stay balanced - every
request, the right server."

## 2. Design choice (1.5 min) - 20 pts
Scroll the design page: UML (Figure 1) and workflow (Figure 2). Say:
- Request is a plain struct; the queue is a FIFO wrapper so no request is starved.
- One integer clock; each tick = arrivals -> firewall -> servers tick -> dispatch -> autoscale.
- 50x/80x band keeps the farm neither overloaded nor idle; cooldown n stops thrashing; min/max
  bounds stop scaling to zero or to infinity; a busy server is never removed.
- Config file so every experiment is one edit, no recompile.
- Config, IPBlocker and Logger are injected by reference, so several balancers can share them
  (that is what makes the bonus switch a composition change, not a rewrite).

## 3. Classes and structures (1.5 min) - 20 pts
In the editor or the Doxygen site, click through: Request.h, RequestQueue, WebServer
(assign/tick), LoadBalancer (stepCycle, autoscale, removeServer), IPBlocker (CIDR as base/mask),
Logger (colour on console, plain file), Config, Random.h, Switch. Point at the Doxygen main page
and the Classes list.

## 4. User input and execution (1.5 min) - 20 pts
```
./loadbalancer --verbose --set status_every=200
```
Type `5` servers and `1500` cycles at the prompts. Narrate the colours: green ASSIGN/ARRIVE, cyan
DONE, red BLOCK (firewall hit), yellow SCALE+/SCALE-, magenta STATUS with queue, active, idle,
removed, processed, rejected. Then show the END block: ending queue, generated/processed/rejected,
active and removed servers, scale-ups/downs, min/max, average wait, utilization.
Open `lb_log.txt` to show the same text with no colour codes.

## 5. Dynamic change in number of servers (1.5 min) - 20 pts
Scale-up and scale-down in one short run (short tasks and a low arrival rate, so the 400-request
backlog first forces a SCALE+, then drains and triggers four SCALE- events):
```
./loadbalancer --no-prompt --verbose --servers 4 --cycles 3000 --set cooldown_n=20 \
  --set arrival_probability=0.1 --set task_time_max=20 --set status_every=500 \
  | grep -E "SCALE|STATUS|END|Ending|Active|Scale-ups"
```
Expected: SCALE+ at cycle 0 (queue 396 > 80*4), STATUS lines showing the queue falling
224 -> 54 -> 0, then SCALE- at ~1174/1194/1214/1234 down to 1 server, END with "Scale-ups / downs : 1 / 4".
Then the deliverable run, quiet mode:
```
./loadbalancer --no-prompt --servers 20 --cycles 100000
```
Read the summary: 20 -> 26 -> 23 servers, queue 2000 -> 1385, inside the 50x-80x band.

## 6. Bonus - switch (1 min)
```
./loadbalancer --no-prompt --set switch_enabled=true
```
Two START and two END blocks: AGF-LB-P and AGF-LB-S with separate queues, pools, scale events and
rejections, plus the switch routing totals.

## 7. Conclusion (30 s)
Capacity guide in one sentence (pool settles near arrival rate x mean task time; keep thresholds
tight; moderate cooldown). Show the GitHub repository page with the commit history. Branding outro.
