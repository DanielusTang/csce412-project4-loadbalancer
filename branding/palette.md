# AggieForge Traffic Cloud — Brand Kit (Project 4)

Same company, same logo, same palette as **AggieForge Dev Cloud** (Project 1),
**AggieForge Container Cloud** (Project 2) and **AggieForge Cluster Cloud** (Project 3).
Project 4 is the fourth product line from the same brand: the anvil-in-a-cloud icon and the
wordmark are reused unchanged; only the product descriptor under the wordmark changes.

**Product name:** AggieForge Traffic Cloud
**Tagline:** *Everything you need to stay balanced — every request, the right server.*
**Component naming:** `AGF-LB-P` / `AGF-LB-S` (load balancers, bonus switch), `srv-01 … srv-NN` (web servers)
**Document ID:** AGF-DOC-005 (design document)

## Palette

| Role | Name | Hex | Usage |
|---|---|---|---|
| Primary | Forge Maroon | `#500000` | Logo, headers, UML class headers |
| Accent | Ember | `#E0731D` | Spark accents, links, callout borders |
| Dark | Charcoal | `#232323` | Body text, terminal backgrounds |
| Neutral | Steel | `#8A929B` | Secondary text, rules, captions |
| Light | Parchment | `#F5F2EC` | Page background, light text on maroon |

## Event colors (console output, requirement 15) — reused in the flow diagram

| Event | ANSI | Hex used in diagrams |
|---|---|---|
| ARRIVE / ASSIGN | green | `#2E7D32` |
| DONE | cyan | `#00838F` |
| BLOCK (firewall) | red | `#B3261E` |
| SCALE+ / SCALE- | yellow | `#B98A00` |
| STATUS | magenta | `#8E24AA` |

## Files

- `logo.svg` — full lockup for light backgrounds (icon + "AggieForge" + TRAFFIC CLOUD)
- `logo-on-maroon.svg` — lockup for maroon backgrounds (page header / footer)
- `logo-icon.svg` — icon only (favicon, Doxygen project logo)

## Usage rules

- Same brand on: the design page (pre-deliverable), the Doxygen site, README, log header,
  and the demo video intro/outro.
- Maroon backgrounds always use Parchment text, never white-on-ember.
- Scripts and C++ sources stay pure ASCII.
