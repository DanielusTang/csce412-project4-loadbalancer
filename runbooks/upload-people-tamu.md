# Runbook — publish the design folder on people.tamu.edu (due Sun Oct 18, 2026, 11:59 pm)

**DONE Oct 18, 2026 11:27 PM** via the Virtual Open Access Lab (Omnissa Horizon in Chrome): Horizon sidebar ->
Transfer Files -> upload `design.zip` (lands in H:\), then in PowerShell on the virtual desktop:
`Expand-Archive -LiteralPath H:\design.zip -DestinationPath U:\csce412\project4 -Force`.
Typing into the Horizon canvas drops 'c'/'v' and shifted characters, so paste commands via the clipboard instead.
Verified logged out: all 8 files return HTTP 200 at https://people.tamu.edu/~leyuantangcollege/csce412/project4/design/.
The Mac SMB mount (Step 3 below) was never completed; the VOAL route is the working one.


people.tamu.edu is NOT an SFTP host. Your web space is the Open Access Labs "U: drive"
(a network share). Anything placed in it is served at https://people.tamu.edu/~leyuantangcollege/.
Sources: TAMU KB articles 1145 (Setting Up a TAMU Personal Web Site), 681 (U: drive), 1146 (Creating a Web Page).

What goes up: the whole `design/` folder (8 files, ~820 KB):

    index.html  style.css  logo.svg  favicon.svg  uml.svg  uml.png  flow.svg  flow.png

Target folder on the U: drive:   `csce412/project4/design/`
URL to submit on Canvas:         `https://people.tamu.edu/~leyuantangcollege/csce412/project4/design/`
(the Doxygen site for Part 2 goes beside it in `csce412/project4/docs/`)

## Step 1 — be on the TAMU network

On campus: TAMU_WiFi. Off campus: connect the TAMU VPN (connect.tamu.edu / Cisco Secure Client) first.
The share is not reachable from the open internet.

## Step 2 — enable your web space (one time)

1. Go to https://oalinfo.tamu.edu -> Manage OAL Account -> log in with NetID.
2. Click "Set up my web page" -> Enable. Wait a couple of minutes for "Enable Web Success".
3. https://people.tamu.edu/~leyuantangcollege/ should now show a placeholder page.

## Step 3 — mount the U: drive on the Mac

Finder -> Go -> Connect to Server (Cmd+K) -> enter:

    smb://storage.it.tamu.edu/tamu/oal/web/~leyuantangcollege

Connect as Registered User: name `AUTH\leyuantangcollege`, your NetID password.
(Windows equivalent: map drive U: to `\\storage.it.tamu.edu\tamu\oal\web\~leyuantangcollege`, user `AUTH\leyuantangcollege`.)
The share opens in Finder as your web root (it already contains the placeholder index.html; leave it).

## Step 4 — copy the folder

Either drag `design/` into the share and create the parents, or from Terminal (share mounts under /Volumes):

    ls /Volumes/                      # find the mounted share name, e.g. "~leyuantangcollege"
    DEST="/Volumes/~leyuantangcollege/csce412/project4/design"
    mkdir -p "$DEST"
    cp "/Users/danielus/Fall 2026/CSCE 412/projects/project_4/csce412-project4/design/"* "$DEST/"
    ls -la "$DEST"                    # expect the 8 files

File names must match exactly (the server is case-sensitive); `index.html` lower case makes the folder URL render.

## Step 5 — verify logged out

1. Private/incognito window -> https://people.tamu.edu/~leyuantangcollege/csce412/project4/design/
2. Check: maroon AggieForge header, both diagrams, styled tables. Plain black text = style.css missing;
   broken image = file name/case mismatch.
3. Click "Open full size (SVG)" under each figure.
4. If the page 403s, re-check Step 2 (web space enabled) and wait a couple of minutes.

## Step 6 — submit

Canvas -> CSCE 412 -> Assignments -> "Project 4 - Pre Deliverable" -> Website URL -> paste the folder URL
(https://canvas.tamu.edu/courses/473182/assignments/3244828).

## Fallbacks if the share will not mount

- Use a campus Open Access Lab machine or the Virtual Open Access Lab (voal.tamu.edu): the U: drive is
  pre-mapped there; copy the folder from Google Drive / a USB stick / email.
- Engineering students also have people.engr.tamu.edu (SFTP to people-edit.engr.tamu.edu); the assignment
  asks for people.tamu.edu, so use that unless the instructor allows the engineering host.

## Re-publishing after an edit

Edit `design/index.html` (or rerun `python3 tools/make-diagrams.py design` and
`tools/render-png.sh design/uml.svg design/uml.png 1620 1170` / `... flow.svg flow.png 1500 2334`
if the diagrams change), then copy only the changed files to the same folder on the share. Commit in git.
