# Runbook - Part 2 submission (due Sun Oct 25, 2026, 11:59 pm)

Five Canvas assignments. Everything below the "needs you" line requires Daniel's logins.

## 0. Final checks (local)

```
cd "/Users/danielus/Fall 2026/CSCE 412/projects/project_4/csce412-project4"
make clean && make                         # no warnings
./loadbalancer --no-prompt --servers 20 --cycles 100000   # regenerates lb_log.txt
tools/run-experiments.sh                   # refreshes evidence/
make docs                                  # docs/html/
git status                                 # clean
```

## 1. Git repository link (20 pts) - needs you

```
gh auth login                              # GitHub.com, HTTPS, login with a browser
gh repo create csce412-project4-loadbalancer --public --source . --remote origin --push
```
Submit the repo URL that `gh repo view --web` opens:
https://canvas.tamu.edu/courses/473182/assignments/3244825

## 2. All code and make file (50 pts)

`dist/aggieforge-traffic-cloud-code.zip` is rebuilt by `tools/package.sh` and contains only
Makefile, *.cpp, *.h, config.txt, README.md, Doxyfile (no executables, no .o).
Upload at https://canvas.tamu.edu/courses/473182/assignments/3244823

## 3. Code documentation - Doxygen on people.tamu.edu (100 pts) - needs you

`dist/docs.zip` holds `docs/html/` renamed to `docs/`. Same VOAL route as the design page
(`runbooks/upload-people-tamu.md`, "DONE" note at the top):

1. voal.tamu.edu in Chrome -> Omnissa Horizon -> sidebar "Transfer Files" -> upload `dist/docs.zip`
   (lands in `H:\`).
2. PowerShell on the virtual desktop (paste via clipboard; typing drops characters):
   ```
   Expand-Archive -LiteralPath H:\docs.zip -DestinationPath U:\csce412\project4 -Force
   ```
   Result: `U:\csce412\project4\docs\index.html` beside the existing `design\` folder.
3. Verify logged out (private window):
   https://people.tamu.edu/~leyuantangcollege/csce412/project4/docs/
   Check the maroon header with the anvil logo, the main page text, and Classes -> LoadBalancer.
4. Submit that URL: https://canvas.tamu.edu/courses/473182/assignments/3244824

## 4. Load balancer log (80 pts)

Upload `evidence/baseline_20x100000_quiet.txt` (or paste its text) at
https://canvas.tamu.edu/courses/473182/assignments/3244827
It shows: starting queue 2000, ending queue 1385, task-time range 3-80, and nine further items
(generated P/S, processed, rejected, in service, active/idle, removed, scale-ups/downs, min/max
servers and queue, average wait, utilization).

## 5. Demo on video (80 pts) - needs you

Record per `runbooks/demo-video-script.md`, upload (Google Drive link or media upload) at
https://canvas.tamu.edu/courses/473182/assignments/3244826

## After submitting

`git push` once more if anything changed; the repo link must show the final state.
