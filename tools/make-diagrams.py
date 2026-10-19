#!/usr/bin/env python3
"""Generate the UML class diagram and the workflow flow diagram for the
AggieForge Traffic Cloud design document as SVG files.

Usage:  python3 tools/make-diagrams.py  [outdir]
Writes  uml.svg  and  flow.svg  into outdir (default: design/).
"""
import sys, os, html

OUT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(__file__), "..", "design")

MAROON, EMBER, CHAR, STEEL, PARCH = "#500000", "#E0731D", "#232323", "#8A929B", "#F5F2EC"
GREEN, CYAN, RED, YELLOW, MAGENTA = "#2E7D32", "#00838F", "#B3261E", "#B98A00", "#8E24AA"
FONT = "'Segoe UI','Helvetica Neue',Helvetica,Arial,sans-serif"
MONO = "Menlo,Consolas,'DejaVu Sans Mono',monospace"


def esc(s):
    return html.escape(s, quote=False)


# ----------------------------------------------------------------------------
# UML class diagram
# ----------------------------------------------------------------------------
HDR_H, LINE_H, PAD = 48, 21, 9
CHAR_W = 8.45  # px per character at 14px Menlo


class Box:
    def __init__(self, name, stereotype, attrs, methods, x, y, w=None, dashed=False, note=None):
        self.name, self.stereo, self.attrs, self.methods = name, stereotype, attrs, methods
        self.x, self.y, self.dashed, self.note = x, y, dashed, note
        longest = max([len(s) for s in attrs + methods] + [len(name) + 4])
        self.w = w or int(longest * CHAR_W + 2 * PAD + 6)
        a_h = (len(attrs) * LINE_H + 2 * PAD) if attrs else 0
        m_h = (len(methods) * LINE_H + 2 * PAD) if methods else 0
        self.h = HDR_H + a_h + m_h
        self.a_h, self.m_h = a_h, m_h

    # anchor points
    @property
    def cx(self): return self.x + self.w / 2
    @property
    def cy(self): return self.y + self.h / 2
    def top(self, f=0.5): return (self.x + self.w * f, self.y)
    def bottom(self, f=0.5): return (self.x + self.w * f, self.y + self.h)
    def left(self, f=0.5): return (self.x, self.y + self.h * f)
    def right(self, f=0.5): return (self.x + self.w, self.y + self.h * f)

    def svg(self):
        o = []
        dash = ' stroke-dasharray="9 6"' if self.dashed else ""
        o.append(f'<rect x="{self.x}" y="{self.y}" width="{self.w}" height="{self.h}" rx="6" ry="6" '
                 f'fill="#ffffff" stroke="{CHAR}" stroke-width="1.6"{dash}/>')
        # header
        o.append(f'<path d="M{self.x},{self.y+6} a6,6 0 0 1 6,-6 h{self.w-12} a6,6 0 0 1 6,6 v{HDR_H-6} h-{self.w} z" fill="{MAROON}"/>')
        ty = self.y + (31 if self.stereo else 31)
        if self.stereo:
            o.append(f'<text x="{self.cx}" y="{self.y+17}" text-anchor="middle" font-family="{FONT}" '
                     f'font-size="11" fill="{PARCH}" opacity=".8">{esc(self.stereo)}</text>')
            ty = self.y + 37
        o.append(f'<text x="{self.cx}" y="{ty}" text-anchor="middle" font-family="{FONT}" '
                 f'font-size="17" font-weight="600" fill="{PARCH}">{esc(self.name)}</text>')
        y = self.y + HDR_H
        if self.attrs:
            yy = y + PAD + 15
            for s in self.attrs:
                o.append(f'<text x="{self.x+PAD}" y="{yy}" font-family="{MONO}" font-size="14" fill="{CHAR}">{esc(s)}</text>')
                yy += LINE_H
            y += self.a_h
            if self.methods:
                o.append(f'<line x1="{self.x}" y1="{y}" x2="{self.x+self.w}" y2="{y}" stroke="{CHAR}" stroke-width="1.2"/>')
        if self.methods:
            yy = y + PAD + 15
            for s in self.methods:
                color = STEEL if s.startswith("//") else CHAR
                o.append(f'<text x="{self.x+PAD}" y="{yy}" font-family="{MONO}" font-size="14" fill="{color}">{esc(s)}</text>')
                yy += LINE_H
        return "\n".join(o)


def line(p1, p2, color=CHAR, dashed=False, width=1.8, marker_end=None, marker_start=None, pts=None):
    pts = pts or [p1, p2]
    d = " ".join(f"{x},{y}" for x, y in pts)
    extra = f' stroke-dasharray="7 5"' if dashed else ""
    me = f' marker-end="url(#{marker_end})"' if marker_end else ""
    ms = f' marker-start="url(#{marker_start})"' if marker_start else ""
    return f'<polyline points="{d}" fill="none" stroke="{color}" stroke-width="{width}"{extra}{me}{ms}/>'


def label(x, y, s, size=12, color=CHAR, anchor="start", weight="normal", italic=False, mono=False):
    fs = f' font-style="italic"' if italic else ""
    fam = MONO if mono else FONT
    return (f'<text x="{x}" y="{y}" text-anchor="{anchor}" font-family="{fam}" font-size="{size}" '
            f'font-weight="{weight}" fill="{color}"{fs}>{esc(s)}</text>')


def defs():
    return f'''<defs>
  <marker id="arrow" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="9" markerHeight="9" orient="auto-start-reverse">
    <path d="M0,0 L10,5 L0,10 z" fill="{CHAR}"/>
  </marker>
  <marker id="arrowOpen" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="10" markerHeight="10" orient="auto-start-reverse">
    <path d="M0,0 L10,5 L0,10" fill="none" stroke="{CHAR}" stroke-width="1.6"/>
  </marker>
  <marker id="diamondFilled" viewBox="0 0 20 12" refX="1" refY="6" markerWidth="18" markerHeight="11" orient="auto-start-reverse">
    <path d="M1,6 L10,1 L19,6 L10,11 z" fill="{CHAR}" stroke="{CHAR}"/>
  </marker>
  <marker id="diamondOpen" viewBox="0 0 20 12" refX="1" refY="6" markerWidth="18" markerHeight="11" orient="auto-start-reverse">
    <path d="M1,6 L10,1 L19,6 L10,11 z" fill="#fff" stroke="{CHAR}" stroke-width="1.4"/>
  </marker>
  <marker id="arrowG" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="9" markerHeight="9" orient="auto-start-reverse">
    <path d="M0,0 L10,5 L0,10 z" fill="{CHAR}"/>
  </marker>
</defs>'''


def brand_strip(W, title, subtitle, y=0):
    icon = ('<g transform="translate(18,10) scale(0.19)">'
            f'<path d="M62,182 h118 a34,34 0 0 0 10,-66 a46,46 0 0 0 -88,-14 a36,36 0 0 0 -40,80 z" fill="{PARCH}" stroke="{PARCH}" stroke-width="7" stroke-linejoin="round"/>'
            f'<path d="M64,124 q0,-8 13,-10 l22,-2 h74 v22 h-20 l-9,11 v9 l16,10 v8 h-56 v-8 l16,-10 v-9 l-9,-11 h-19 l-12,-2 q-13,-2 -13,-8 z" fill="{MAROON}"/>'
            f'<path d="M196,42 l6,14 14,6 -14,6 -6,14 -6,-14 -14,-6 14,-6 z" fill="{EMBER}"/></g>')
    return (f'<rect x="0" y="{y}" width="{W}" height="58" fill="{MAROON}"/>'
            f'<rect x="0" y="{y+58}" width="{W}" height="3" fill="{EMBER}"/>'
            + icon +
            label(72, y + 26, "AggieForge", 18, PARCH, weight="600") +
            f'<text x="172" y="{y+26}" font-family="{FONT}" font-size="18" font-weight="600" fill="{EMBER}">Traffic Cloud</text>' +
            label(72, y + 46, subtitle, 12, PARCH) +
            label(W - 20, y + 36, title, 20, PARCH, anchor="end", weight="600"))


def uml():
    W, H = 1620, 1170
    TOP = 100
    XA, XB, XC = 40, 560, 1150
    boxes = {}
    # column A
    boxes["Request"] = Box("Request", "<<struct>>",
        ["+ ipIn    : std::string   // requester IP",
         "+ ipOut   : std::string   // destination IP",
         "+ time    : int           // cycles to process",
         "+ jobType : char          // 'P' | 'S'",
         "+ arrivedCycle : int      // for wait stats"],
        ["// plain data - no behaviour"], XA, TOP)
    boxes["IPBlocker"] = Box("IPBlocker", "firewall / DOS prevention",
        ["- ranges_       : vector<Range{base,maskBits}>",
         "- blockedCount_ : int"],
        ["+ addRange(cidr : string) : bool",
         "+ isBlocked(ip : string) : bool",
         "+ blockedCount() : int",
         "+ ranges() : const vector<Range>&"], XA, TOP + 240)
    boxes["Logger"] = Box("Logger", "console (color) + plain-text file",
        ["- level_ : Level { Quiet, Verbose }",
         "- file_  : std::ofstream",
         "- color_ : bool            // ANSI on console"],
        ["+ Logger(path, level : Level, color : bool)",
         "+ event(cat : Category, msg, cycle : int)",
         "+ status(msg : string, cycle : int) // verbose",
         "+ summary(msg : string)             // always",
         "+ setLevel(level : Level)"], XA, TOP + 470)
    boxes["Config"] = Box("Config", "config.txt parser (requirement 13)",
        ["- values_ : map<string,string>"],
        ["+ load(path : string) : bool",
         "+ getInt(key, def : int) : int",
         "+ getDouble(key, def : double) : double",
         "+ getBool(key, def : bool) : bool",
         "+ getString(key, def : string) : string",
         "+ getList(key) : vector<string>"], XA, TOP + 750)

    # column B
    boxes["RequestQueue"] = Box("RequestQueue", "FIFO",
        ["- q_        : std::queue<Request>",
         "- peakSize_ : size_t"],
        ["+ push(r : Request) : void",
         "+ pop() : Request          // pre: !empty()",
         "+ front() : const Request&",
         "+ empty() : bool",
         "+ size() : size_t",
         "+ peakSize() : size_t"], XB, TOP)
    boxes["LoadBalancer"] = Box("LoadBalancer", "one instance per server pool",
        ["- name_          : std::string   // 'AGF-LB-P' ...",
         "- queue_         : RequestQueue",
         "- servers_       : vector<WebServer>",
         "- blocker_       : IPBlocker&",
         "- log_           : Logger&",
         "- cfg_           : const Config&",
         "- clock_         : int",
         "- cooldownUntil_ : int   // autoscale may act again",
         "- stats_ : Stats { generated, processed, rejected,",
         "      scaleUps, scaleDowns, minServers, maxServers,",
         "      minQueue, maxQueue, totalWait, busyCycles }"],
        ["+ LoadBalancer(cfg, blocker&, log&, name : string)",
         "+ seedQueue(count : int) : void   // servers * 100",
         "+ addRequest(r : Request) : bool  // false => blocked",
         "+ stepCycle() : void              // one clock tick",
         "+ run(cycles : int) : void        // calls stepCycle",
         "+ queueSize() : size_t",
         "+ activeServers() : int   + inactiveServers() : int",
         "+ stats() : const Stats&",
         "+ printSummary() : void           // START / END block",
         "- dispatch() : void               // queue -> idle srv",
         "- maybeGenerateRequest() : void   // random arrivals",
         "- autoscale() : void              // 50x/80x + cooldown",
         "- addServer() : void      - removeServer() : void"], XB, TOP + 310)

    # column C
    boxes["WebServer"] = Box("WebServer", "",
        ["- id_          : int",
         "- busy_        : bool",
         "- current_     : Request",
         "- finishCycle_ : int",
         "- handled_     : int",
         "- busyCycles_  : int"],
        ["+ WebServer(id : int)",
         "+ assign(r : Request, cycle : int) : void",
         "+ tick(cycle : int) : bool   // true when done",
         "+ isBusy() : bool",
         "+ id() : int",
         "+ requestsHandled() : int",
         "+ busyCycles() : int"], XC, TOP)
    boxes["Random"] = Box("RequestFactory", "<<free functions>> Random.h",
        [],
        ["randomIPv4() : string",
         "randomTaskTime(min, max : int) : int",
         "randomJobType() : char   // 'P' or 'S'",
         "makeRequest(cfg : Config&, cycle) : Request"], XC, TOP + 390)
    boxes["Switch"] = Box("Switch", "<<bonus>> routes by jobType",
        ["- lbs_ : map<char, LoadBalancer*>"],
        ["+ attach(jobType : char, lb : LoadBalancer&)",
         "+ route(r : Request) : bool",
         "+ stepAll() : void",
         "+ summaryAll() : void"], XC, TOP + 600, dashed=True)
    boxes["Main"] = Box("main.cpp", "<<driver>> written by hand (rule A)",
        [],
        ["read config.txt -> prompt servers, cycles",
         "build Logger, IPBlocker, Config",
         "build LoadBalancer (bonus: Switch + 2 LBs)",
         "seedQueue(servers * 100); START summary",
         "loop: stepCycle() x cycles; END summary"], XC, TOP + 840)

    o = [f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {W} {H}" width="{W}" height="{H}" font-family="{FONT}">',
         defs(),
         f'<rect width="{W}" height="{H}" fill="{PARCH}"/>',
         brand_strip(W, "Class Diagram (UML)", "CSCE 412 - Project 4 Load Balancer - Design Document AGF-DOC-005")]

    B = boxes
    LB, RQ, WS = B["LoadBalancer"], B["RequestQueue"], B["WebServer"]

    def hline(src, dst, y, **kw):
        """horizontal connector from src.right to dst.left (or reverse) at absolute y"""
        if src.x < dst.x:
            p1, p2 = (src.x + src.w, y), (dst.x, y)
        else:
            p1, p2 = (src.x, y), (dst.x + dst.w, y)
        o.append(line(p1, p2, **kw))
        return p1, p2

    def midlabel(p1, p2, s, dy=-7, size=12, color=STEEL):
        o.append(label((p1[0] + p2[0]) / 2, p1[1] + dy, s, size, color, anchor="middle", italic=True))

    # RequestQueue <>-- Request (aggregation 0..*)
    y = B["Request"].y + B["Request"].h * 0.5
    p1, p2 = hline(RQ, B["Request"], y, marker_start="diamondOpen")
    midlabel(p1, p2, "holds 0..*")

    # LoadBalancer *-- RequestQueue (composition 1)
    x = RQ.cx
    p1, p2 = (x, LB.y), (x, RQ.y + RQ.h)
    o.append(line(p1, p2, marker_start="diamondFilled"))
    o.append(label(x + 10, (p1[1] + p2[1]) / 2 + 4, "queue_  1", 12, STEEL, italic=True))

    # LoadBalancer *-- WebServer (composition 1..*)
    y = LB.y + 24
    p1, p2 = hline(LB, WS, y, marker_start="diamondFilled")
    midlabel(p1, p2, "servers_ 1..*", dy=-7, size=11)

    # WebServer ..> Request (current_, 0..1) over the top
    yy = TOP - 22
    p1, p2 = WS.top(0.5), B["Request"].top(0.5)
    o.append(line(p1, p2, dashed=True, marker_end="arrowOpen", pts=[p1, (p1[0], yy), (p2[0], yy), p2]))
    o.append(label((p1[0] + p2[0]) / 2, yy - 6, "current_ : Request (0..1)  -  a WebServer processes one Request at a time", 12, STEEL, anchor="middle", italic=True))

    # LoadBalancer --> IPBlocker / Logger / Config (references, straight lines)
    for key, txt in (("IPBlocker", "blocker_ &"), ("Logger", "log_ &"), ("Config", "cfg_ const&")):
        t = B[key]
        y = t.cy
        p1, p2 = hline(LB, t, y, marker_end="arrowOpen")
        midlabel(p1, p2, txt, size=11)

    # RequestFactory ..> used by LoadBalancer
    y = B["Random"].cy
    p1, p2 = hline(LB, B["Random"], y, dashed=True, marker_start="arrowOpen")
    midlabel(p1, p2, "<<uses>>", size=11)

    # Switch --> LoadBalancer (1..*)
    y = B["Switch"].cy
    p1, p2 = hline(B["Switch"], LB, y, marker_end="arrowOpen")
    midlabel(p1, p2, "lbs_ 1..*", size=11)

    # Main ..> LoadBalancer
    m = B["Main"]
    p1 = (m.x, m.cy); px = LB.x + LB.w - 60
    p2 = (px, LB.y + LB.h)
    o.append(line(p1, p2, dashed=True, marker_end="arrowOpen", pts=[p1, (px, p1[1]), p2]))
    o.append(label((p1[0] + px) / 2, p1[1] + 18, "<<creates, drives>>", 11, STEEL, anchor="middle", italic=True))

    for b in boxes.values():
        o.append(b.svg())

    # legend
    lx, ly = XB, H - 118
    o.append(f'<rect x="{lx}" y="{ly}" width="520" height="96" rx="6" fill="#fff" stroke="{STEEL}" stroke-width="1"/>')
    o.append(label(lx + 12, ly + 20, "Legend", 13, MAROON, weight="600"))
    o.append(line((lx + 12, ly + 42), (lx + 70, ly + 42), marker_start="diamondFilled"))
    o.append(label(lx + 80, ly + 46, "composition (owns, same lifetime)", 12))
    o.append(line((lx + 12, ly + 66), (lx + 70, ly + 66), marker_start="diamondOpen"))
    o.append(label(lx + 80, ly + 70, "aggregation (contains copies)", 12))
    o.append(line((lx + 290, ly + 42), (lx + 348, ly + 42), marker_end="arrowOpen"))
    o.append(label(lx + 358, ly + 46, "reference / association", 12))
    o.append(line((lx + 290, ly + 66), (lx + 348, ly + 66), dashed=True, marker_end="arrowOpen"))
    o.append(label(lx + 358, ly + 70, "dependency (uses / creates)", 12))
    o.append(label(lx + 12, ly + 88, "dashed box = bonus component   |   '+' public  '-' private   |   name : type", 11, STEEL))

    o.append(label(W - 20, H - 12, "AggieForge Traffic Cloud  -  Daniel Tang  -  CSCE 412 Cloud Computing, Texas A&M University  -  Oct 2026", 11, STEEL, anchor="end"))
    o.append("</svg>")
    return "\n".join(o)


# ----------------------------------------------------------------------------
# Workflow / flow diagram
# ----------------------------------------------------------------------------
def flow():
    W = 1500
    o = []   # header prepended at the end once the height is known

    CX = 560          # spine centre
    BW = 520          # process box width

    def text_block(cx, y, lines, size=14, color=CHAR, weight="normal", anchor="middle", mono=False):
        out = []
        for i, s in enumerate(lines):
            w = "600" if (i == 0 and weight == "title") else ("600" if weight == "600" else "normal")
            sz = size + (1 if (i == 0 and weight == "title") else 0)
            col = color if (i == 0 or color != CHAR) else "#444"
            out.append(label(cx, y + i * (size + 6), s, sz, col, anchor=anchor, weight=w, mono=mono))
        return "\n".join(out)

    def process(cx, y, lines, w=BW, accent=MAROON, tag=None):
        h = 22 + len(lines) * 20 + 10
        x = cx - w / 2
        o.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="7" fill="#fff" stroke="{CHAR}" stroke-width="1.6"/>')
        o.append(f'<rect x="{x}" y="{y}" width="8" height="{h}" rx="3" fill="{accent}"/>')
        if tag:
            o.append(f'<rect x="{x + w - 118}" y="{y - 11}" width="110" height="22" rx="11" fill="{accent}"/>')
            o.append(label(x + w - 63, y + 4, tag, 11, "#fff", anchor="middle", weight="600"))
        o.append(text_block(cx + 4, y + 26, lines, 14, weight="title"))
        return (x, y, w, h)

    def terminal(cx, y, s, w=220, h=48):
        x = cx - w / 2
        o.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="24" fill="{MAROON}" stroke="{MAROON}"/>')
        o.append(label(cx, y + h / 2 + 6, s, 16, PARCH, anchor="middle", weight="600"))
        return (x, y, w, h)

    def decision(cx, y, lines, w=360, h=104, accent=MAROON):
        pts = f"{cx},{y} {cx + w/2},{y + h/2} {cx},{y + h} {cx - w/2},{y + h/2}"
        o.append(f'<polygon points="{pts}" fill="#fff8ee" stroke="{accent}" stroke-width="2"/>')
        start = y + h / 2 - (len(lines) - 1) * 10 + 5
        for i, s in enumerate(lines):
            o.append(label(cx, start + i * 20, s, 14, CHAR, anchor="middle", weight="600" if i == 0 else "normal"))
        return (cx - w / 2, y, w, h)

    def arrow(pts, lbl=None, color=CHAR, lbl_dx=8, lbl_dy=-6, dashed=False):
        o.append(line(None, None, color=color, dashed=dashed, marker_end="arrow", pts=pts))
        if lbl:
            o.append(label(pts[0][0] + lbl_dx, pts[0][1] + lbl_dy + 18, lbl, 13, MAROON, weight="600"))

    def side_note(x, y, w, lines, accent=STEEL):
        h = 14 + len(lines) * 18 + 8
        o.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="4" fill="#fffdf8" stroke="{accent}" stroke-width="1" stroke-dasharray="4 3"/>')
        for i, s in enumerate(lines):
            o.append(label(x + 10, y + 20 + i * 18, s, 12, "#444"))
        return h

    y = 100
    t = terminal(CX, y, "START")
    y += 48 + 28
    arrow([(CX, y - 28), (CX, y)])

    b1 = process(CX, y, ["1. Read config.txt",
                         "servers, cycles, task_time_min/max, arrival_probability,",
                         "cooldown_n, scale_low (50) / scale_high (80), min/max servers,",
                         "blocked_ranges, mode (quiet | verbose), color, log_file"])
    y += b1[3] + 30
    arrow([(CX, y - 30), (CX, y)])

    b2 = process(CX, y, ["2. Prompt the user  (requirement 4)",
                         "number of web servers N   ->  overrides config",
                         "run time in clock cycles  ->  overrides config"])
    y += b2[3] + 30
    arrow([(CX, y - 30), (CX, y)])

    b3 = process(CX, y, ["3. Build the components",
                         "Logger(log_file, mode, color)   IPBlocker.addRange() for each blocked range",
                         "LoadBalancer(cfg, blocker, logger)  ->  creates N WebServer(id)",
                         "bonus: Switch + LoadBalancer 'AGF-LB-P' + LoadBalancer 'AGF-LB-S'"])
    y += b3[3] + 30
    arrow([(CX, y - 30), (CX, y)])

    b4 = process(CX, y, ["4. Seed the starting queue",
                         "push N x 100 random Requests  (makeRequest: ipIn, ipOut, time, jobType)",
                         "log START summary: servers, starting queue size, task-time range,",
                         "thresholds 50xN / 80xN, cooldown n, blocked ranges"])
    y += b4[3] + 34
    arrow([(CX, y - 34), (CX, y)])

    # ---- loop head decision ----
    LOOP_TOP = y
    d1 = decision(CX, y, ["cycle < runCycles ?"], w=330, h=96)
    # NO branch -> right -> END summary
    nx = CX + 165
    end_x = 1160
    arrow([(nx, y + 48), (end_x - 200, y + 48)], lbl="no", lbl_dx=14, lbl_dy=-24)
    e1 = process(end_x, y - 40, ["9. Log END summary  (quiet + verbose)",
                                "ending queue size, generated / processed / rejected,",
                                "active vs inactive (removed) servers, scale-ups / downs,",
                                "min / max servers, min / max queue, avg wait, utilization,",
                                "P / S split (bonus: one block per load balancer)"], w=400, accent=MAGENTA)
    ey = y - 40 + e1[3] + 28
    arrow([(end_x, ey - 28), (end_x, ey)])
    terminal(end_x, ey, "END", w=180)
    y += 96 + 30
    arrow([(CX, y - 30), (CX, y)], lbl="yes", lbl_dx=10, lbl_dy=-24)

    # ---- 5. arrivals ----
    b5 = process(CX, y, ["5. Random arrival  (requirement 9)",
                         "with probability arrival_probability this cycle:",
                         "r = makeRequest(cfg, cycle)   ->   ipIn, ipOut, time in [min,max], 'P'/'S'"],
                 accent=GREEN, tag="ARRIVE")
    y += b5[3] + 30
    arrow([(CX, y - 30), (CX, y)])

    d2 = decision(CX, y, ["IPBlocker.isBlocked(r.ipIn) ?"], w=380, h=96, accent=RED)
    # yes -> right: reject
    arrow([(CX + 190, y + 48), (CX + 190 + 110, y + 48)], lbl="yes", lbl_dx=14, lbl_dy=-24)
    rj = process(CX + 190 + 110 + 165, y + 48 - 38, ["Reject the request",
                                                   "stats.rejected++ ; log BLOCK (red):",
                                                   "'<ip> rejected (range <cidr>)'"], w=330, accent=RED, tag="BLOCK")
    y += 96 + 30
    arrow([(CX, y - 30), (CX, y)], lbl="no", lbl_dx=10, lbl_dy=-24)
    # rejected rejoins below the push box
    b6 = process(CX, y, ["6. Admit the request",
                         "queue_.push(r)   (bonus: Switch.route(r) picks the LB by jobType)",
                         "stats.generated++ ; verbose: log ARRIVE (green)"], accent=GREEN)
    # join line from reject box down to below b6
    rj_bottom = (rj[0] + rj[2] / 2, rj[1] + rj[3])
    join_y = y + b6[3] + 15
    o.append(line(None, None, pts=[rj_bottom, (rj_bottom[0], join_y), (CX, join_y)]))
    y += b6[3] + 30
    arrow([(CX, y - 30), (CX, y)])

    # ---- 7. servers tick + dispatch ----
    b7 = process(CX, y, ["7. Advance every WebServer one cycle",
                         "for each server: done = tick(cycle)",
                         "done -> handled++ ; busy_ = false ; verbose: log DONE (cyan)",
                         "   'srv 03 finished; asking for next'"], accent=CYAN, tag="DONE")
    y += b7[3] + 30
    arrow([(CX, y - 30), (CX, y)])

    b8 = process(CX, y, ["8. Dispatch  queue -> idle servers",
                         "while (idle server exists && !queue_.empty()):",
                         "   r = queue_.pop() ; server.assign(r, cycle) ; totalWait += cycle - r.arrivedCycle",
                         "   verbose: log ASSIGN (green)  'srv 03 <- 172.16.4.9 -> 8.8.8.8 (P, 41 cyc)'"],
                 accent=GREEN, tag="ASSIGN")
    y += b8[3] + 34
    arrow([(CX, y - 34), (CX, y)])

    # ---- autoscale ----
    d3 = decision(CX, y, ["cycle >= cooldownUntil ?", "(rest period n elapsed)"], w=360, h=104, accent=YELLOW)
    # yes -> right: nested decisions
    ax = CX + 180 + 100 + 170  # centre of right column
    arrow([(CX + 180, y + 52), (ax - 170 - 10, y + 52)], lbl="yes", lbl_dx=14, lbl_dy=-24)
    d4 = decision(ax, y, ["queue < 50 x N", "and N > min_servers ?"], w=340, h=104, accent=YELLOW)
    # d4 yes -> down to remove
    ry = y + 104 + 26
    arrow([(ax, y + 104), (ax, ry)], lbl="yes", lbl_dx=10, lbl_dy=-22)
    rm = process(ax, ry, ["Scale DOWN  ->  remove one idle server",
                          "stats.scaleDowns++ ; cooldownUntil = cycle + n",
                          "log SCALE- (yellow); a busy server is never dropped"], w=400, accent=YELLOW, tag="SCALE-")
    # d4 no -> right-side second decision below? Use another decision to the right of d4 is too wide; put below rm
    dy5 = ry + rm[3] + 26
    # d4 "no" path goes around the remove box on the right
    right_edge = ax + 170
    o.append(line(None, None, pts=[(right_edge, y + 52), (right_edge + 60, y + 52), (right_edge + 60, dy5 + 52), (ax + 170 + 8, dy5 + 52)], marker_end="arrow"))
    o.append(label(right_edge + 8, y + 52 - 10, "no", 13, MAROON, weight="600"))
    d5 = decision(ax, dy5, ["queue > 80 x N", "and N < max_servers ?"], w=340, h=104, accent=YELLOW)
    ay = dy5 + 104 + 26
    arrow([(ax, dy5 + 104), (ax, ay)], lbl="yes", lbl_dx=10, lbl_dy=-22)
    ad = process(ax, ay, ["Scale UP  ->  add one WebServer(nextId)",
                          "stats.scaleUps++ ; cooldownUntil = cycle + n",
                          "log SCALE+ (yellow)  'queue 1710 > 80*20=1600 -> add srv 21'"], w=400, accent=YELLOW, tag="SCALE+")
    # d5 no -> down-left to join spine
    y_status = ay + ad[3] + 40
    o.append(line(None, None, pts=[(ax - 170, dy5 + 52), (ax - 170 - 40, dy5 + 52), (ax - 170 - 40, y_status - 20), (CX + 20, y_status - 20)]))
    o.append(label(ax - 170 - 44, dy5 + 52 - 8, "no", 13, MAROON, weight="600", anchor="end"))
    o.append(label(CX + 30, y_status - 28, "in range (50xN <= queue <= 80xN) or no change -> continue", 12, MAROON, weight="600"))
    # remove box bottom joins the same horizontal
    o.append(line(None, None, pts=[(ax, ry + rm[3]), (ax, ry + rm[3] + 12), (ax - 170 - 40 + 1, ry + rm[3] + 12)]))
    # add box bottom joins
    o.append(line(None, None, pts=[(ax, ay + ad[3]), (ax, y_status - 20)]))
    # d3 no -> straight down the spine
    arrow([(CX, y + 104), (CX, y_status)], lbl="no", lbl_dx=10, lbl_dy=-22)

    y = y_status
    b9 = process(CX, y, ["10. Status heartbeat  (verbose only, every status_every cycles)",
                         "log STATUS (magenta): queue, active servers, idle, rejected so far",
                         "then  cycle++  (clock_ = cycle)"], accent=MAGENTA, tag="STATUS")
    y += b9[3]
    # loop back to loop head on the left
    LX = 60
    o.append(line(None, None, pts=[(CX, y), (CX, y + 30), (LX, y + 30), (LX, LOOP_TOP + 48), (CX - 165, LOOP_TOP + 48)], marker_end="arrow"))
    o.append(label(LX + 10, y + 26, "next cycle", 13, MAROON, weight="600"))
    o.append(label(LX - 6, (y + LOOP_TOP) / 2, "", 12))
    # rotated label on the loop line
    mid = (y + LOOP_TOP) / 2
    o.append(f'<text transform="translate({LX - 14},{mid}) rotate(-90)" text-anchor="middle" font-family="{FONT}" font-size="13" font-weight="600" fill="{MAROON}">per-cycle loop (steps 5 - 10 = LoadBalancer::stepCycle)</text>')

    # side notes
    side_note(1010, 100, 440, ["Inputs:  config.txt  +  two interactive values",
                               "Outputs: colored console events (verbose) and a plain-text log",
                               "         file; START and END summaries in both modes",
                               "Deliverable run: N = 20 servers, 100 000 cycles, quiet mode"])
    side_note(1010, 232, 440, ["Event colours (requirement 15) match the box accents here:",
                               "green ARRIVE/ASSIGN   cyan DONE   red BLOCK",
                               "yellow SCALE+/SCALE-   magenta STATUS"])
    side_note(60, y + 60, 430, ["Invariant the autoscaler keeps:",
                                "    50 x N  <=  queue size  <=  80 x N   (N = active servers)",
                                "Cooldown n stops thrashing: after any add/remove the rule is",
                                "not re-evaluated until n cycles have passed."])
    side_note(1010, y + 60, 440, ["Bonus (dashed in the UML): Switch sits in front of two load",
                                  "balancers. Step 6 becomes Switch.route(r): 'S' -> AGF-LB-S,",
                                  "'P' -> AGF-LB-P; steps 7 - 10 run once per load balancer and",
                                  "each prints its own END summary."])

    H = y + 60 + 110 + 50
    o.append(label(W - 20, H - 12, "AggieForge Traffic Cloud  -  Daniel Tang  -  CSCE 412 Cloud Computing, Texas A&M University  -  Oct 2026", 11, STEEL, anchor="end"))
    head = [f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {W} {H}" width="{W}" height="{H}" font-family="{FONT}">',
            defs(),
            f'<rect width="{W}" height="{H}" fill="{PARCH}"/>',
            brand_strip(W, "Simulation Workflow", "CSCE 412 - Project 4 Load Balancer - Design Document AGF-DOC-005")]
    o = head + o
    o.append("</svg>")
    print("flow height", H)
    return "\n".join(o)


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    with open(os.path.join(OUT, "uml.svg"), "w") as f:
        f.write(uml())
    with open(os.path.join(OUT, "flow.svg"), "w") as f:
        f.write(flow())
    print("wrote", os.path.join(OUT, "uml.svg"), "and flow.svg")
