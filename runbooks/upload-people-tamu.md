# Runbook — publish the design folder on people.tamu.edu (due Sun Oct 18, 2026, 11:59 pm)

What goes up: the whole `design/` folder (8 files, ~1.1 MB):

    index.html  style.css  logo.svg  favicon.svg  uml.svg  uml.png  flow.svg  flow.png

Target folder in your web space (create it):   `public_html/csce412/project4/design/`
Resulting URL to submit:                        `https://people.tamu.edu/~<netid>/csce412/project4/design/`
(the Doxygen site for Part 2 goes beside it in `csce412/project4/docs/`)

## Option A — web file manager (no extra tools)

1. Go to https://people.tamu.edu/ -> "Manage your site" (CAS login + Duo).
2. In the file manager open `public_html`, create folder `csce412`, inside it `project4`, inside it `design`.
3. Upload all 8 files from `design/` into that folder (multi-select; keep the exact file names — the
   server is case-sensitive and `index.html` must be lower case so the folder URL renders the page).
4. If the manager shows permissions: folders 755, files 644.

## Option B — SFTP from the Mac

    cd "/Users/danielus/Fall 2026/CSCE 412/projects/project_4/csce412-project4"
    sftp <netid>@people.tamu.edu            # NetID password (+ Duo if prompted)
    sftp> cd public_html
    sftp> mkdir csce412
    sftp> mkdir csce412/project4
    sftp> mkdir csce412/project4/design
    sftp> cd csce412/project4/design
    sftp> put design/*
    sftp> chmod 644 *
    sftp> bye

(If `sftp` is refused, use Option A; the file-manager route always works.)

## Verify (do this logged out)

1. Open a private/incognito window and load
   `https://people.tamu.edu/~<netid>/csce412/project4/design/`
2. Check: maroon header with the AggieForge logo, both diagrams visible, tables styled.
   A missing stylesheet (plain black text) or a broken image means a file name/case mismatch.
3. Click "Open full size (SVG)" under each figure — the SVGs must open in their own tab.

## Submit

Canvas -> CSCE 412 -> Assignments -> "Project 4 - Pre Deliverable" -> Website URL -> paste the folder URL
(https://canvas.tamu.edu/courses/473182/assignments/3244828).

## Re-publishing after an edit

Edit `design/index.html` (or rerun `python3 tools/make-diagrams.py design` and
`tools/render-png.sh design/uml.svg design/uml.png 1620 1170` / `... flow.svg flow.png 1500 2334`
if the diagrams change), then re-upload only the changed files to the same folder. Commit in git.
