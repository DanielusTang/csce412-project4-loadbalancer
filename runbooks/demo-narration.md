# Demo narration - word for word (target 6-8 min)

Read the **SAY** lines aloud. **SHOW** tells you what is on screen. Pause ~1 s between paragraphs.
Windows: Chrome (design page, Doxygen site, GitHub) and the Terminal window in the repo.

---

## 1. Intro - 30 s

**SHOW** Chrome, design page, scrolled to the top (maroon header + logo).

**SAY**
Hi, I'm Daniel Tang, and this is my CSCE 412 Project 4: AggieForge Traffic Cloud.
It's a C++ simulation of the load balancer that sits in front of a large company's web-server farm.
Requests come in from random IP addresses, each needs a random amount of work, and the balancer hands
every request to a free web server while adding and removing servers as the load changes.
Our tagline: everything you need to stay balanced - every request, the right server.
I'll cover the design, the classes, a live run with user input, the dynamic server scaling, and the bonus switch.

---

## 2. Design choice - 1.5 min (20 pts)

**SHOW** Scroll to Figure 1 (UML).

**SAY**
Here's the class design. A Request is a plain struct: source IP, destination IP, processing time in cycles,
and a job type, 'P' for processing or 'S' for streaming. Requests wait in a RequestQueue, which is a
first-in-first-out wrapper, so no request can be starved. A WebServer holds at most one request and counts
down its remaining cycles. The LoadBalancer owns the queue and the pool of servers.

Three helpers are injected into the balancer by reference: Config, which reads config.txt; IPBlocker,
the firewall; and Logger, which writes to the console and the log file. Because they're injected, several
balancers can share them. That's what makes the bonus switch a composition change instead of a rewrite.

**SHOW** Scroll to Figure 2 (workflow).

**SAY**
Everything runs on one integer clock. Each tick does five things in a fixed order: new arrivals, firewall check,
servers tick down, dispatch queued requests to idle servers, then autoscale.

The scaling rule is a band. If the queue is more than 80 times the number of servers, we add one server.
If it drops below 50 times, we remove one idle server. After any change the balancer rests for n cycles,
the cooldown, so it doesn't thrash. There are hard minimum and maximum server bounds, and a busy server is never
removed. Every one of those numbers lives in config.txt, so an experiment is one edit, no recompile.

---

## 3. Classes and structures - 1.5 min (20 pts)

**SHOW** Chrome, Doxygen site. Click **Classes** -> **Class List**.

**SAY**
This is the Doxygen documentation, hosted on people.tamu.edu. Every class and method has a comment.
Let me walk through the main ones.

**SHOW** Click **Request** (struct).

**SAY**
Request: the four fields from the design, plus arrivedCycle so we can measure wait time.

**SHOW** Click **RequestQueue**.

**SAY**
RequestQueue wraps a std::queue and tracks the peak depth for the summary.

**SHOW** Click **WebServer**.

**SAY**
WebServer has assign, which takes a request, and tick, which counts down one cycle and reports when the job is done.

**SHOW** Click **LoadBalancer**. Scroll the member list.

**SAY**
LoadBalancer is the core. seedQueue fills the starting queue. addRequest runs a request through the firewall.
stepCycle advances the clock one tick in that fixed order. The private helpers are dispatch, autoscale,
addServer and removeServer. printSummary writes the START and END blocks you'll see in a moment.

**SHOW** Click **IPBlocker**, then **Logger**, then **Switch** (briefly each).

**SAY**
IPBlocker stores each blocked range as a base and mask, so one bitwise AND decides a hit. It accepts CIDR notation.
Logger colours each event type on the console and writes the same line without escape codes to the file.
And Switch, the bonus, routes P and S jobs to two separate balancers. Config and the Random helpers round it out.

---

## 4. User input and execution - 1.5 min (20 pts)

**SHOW** Terminal. Type the command, then answer the two prompts.

```
./loadbalancer --verbose --set status_every=200
```
Prompts: `5` servers, `1500` cycles.

**SAY**
Now a live run. The program asks for the number of servers and how long to run. I'll say 5 servers and
1500 clock cycles. The queue starts at servers times 100, so 500 requests.

Verbose mode colour-codes every event. Green is an arrival or an assignment to a server. Cyan is a job
finishing. Red is a firewall block - that request came from a blocked range and was rejected. Yellow is a
scale event, plus or minus. And every 200 cycles the magenta STATUS line gives the queue depth, active and
idle servers, servers removed, and the processed and rejected counts.

**SHOW** Scroll to the END block at the bottom.

**SAY**
At the end we get a summary: ending queue size, requests generated split by P and S, processed, rejected
by the firewall, how many servers are active and how many were removed, scale-ups and scale-downs,
the min and max server count and queue depth, the average wait, and server utilization.

**SHOW** `head -40 lb_log.txt` (or open the file).

**SAY**
The same output goes to lb_log.txt with no colour codes, so it's clean text for grading.

---

## 5. Dynamic change in number of servers - 1.5 min (20 pts)

**SHOW** Terminal. Paste the filtered command.

```
./loadbalancer --no-prompt --verbose --servers 4 --cycles 3000 --set cooldown_n=20 \
  --set arrival_probability=0.1 --set task_time_max=20 --set status_every=500 \
  | grep -E "SCALE|STATUS|END|Ending|Active|Scale-ups"
```

**SAY**
This run is tuned so you can see both directions of scaling in a few seconds: 4 servers, a short cooldown
of 20, a low arrival rate, and short tasks. I'm filtering the output to just the scale and status lines.

At cycle zero the queue is 396, which is above 80 times 4, so the balancer adds a fifth server.
Then the backlog drains: 224, 54, zero. Once the queue is below 50 times the server count, it removes
an idle server every 20 cycles - here, at cycles 1174, 1194, 1214 and 1234 - down to a single server,
which is the configured minimum. The END block confirms it: one scale-up, four scale-downs.

**SHOW** Run the deliverable in quiet mode.

```
./loadbalancer --no-prompt --servers 20 --cycles 100000
```

**SAY**
And this is the required deliverable run: 20 servers, 100,000 cycles, quiet mode. Starting queue 2000,
task times 3 to 80 cycles. It finished with a queue of 1385 and 23 servers. During the run it scaled up six
times to a peak of 26 and down three times. 241 requests were rejected by the firewall. The ending queue is
inside the 50 to 80 band for 23 servers, which is 1150 to 1840, so the pool settled exactly where the rule says it should.

---

## 6. Bonus - switch - 1 min

**SHOW** Terminal.

```
./loadbalancer --no-prompt --set switch_enabled=true
```

**SAY**
For the bonus, one config flag turns on the switch. Now main generates the arrivals and the Switch routes
each request by job type to one of two balancers: AGF-LB-P for processing jobs and AGF-LB-S for streaming.
You get two START blocks and two END blocks. Each balancer has its own queue, its own server pool, its own
scale events and its own rejection count. Because the helpers are injected, this was a composition change:
no balancer code was modified.

---

## 7. Conclusion - 30 s

**SHOW** Chrome, GitHub repo page (commit history visible).

**SAY**
From the experiments in the README, the capacity rule of thumb is: the pool settles near arrival rate times
mean task time; keep the thresholds tight and the cooldown moderate, and the balancer stays stable.
The code, Makefile, config and Doxygen setup are on GitHub, and the documentation is on people.tamu.edu.
Thanks for watching - AggieForge Traffic Cloud: every request, the right server.
