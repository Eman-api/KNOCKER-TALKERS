# Start here (no GitHub experience needed)

Think of this repo as a **shared Google Drive folder that keeps every old version**. You only need 3 buttons.

## One-time setup (10 minutes)

1. Make a free account at **github.com** and send your username to E so he can add you.
2. Accept the invite. It's in your email, or at github.com/notifications.
3. Install **GitHub Desktop**: desktop.github.com. Sign in with your GitHub account.
4. In GitHub Desktop: **File → Clone repository → KNOCKER-TALKERS → Clone.**
   The project now lives in a normal folder on your computer (default: `Documents/GitHub/KNOCKER-TALKERS`).
5. Install **Arduino IDE 2** (arduino.cc/en/software) if you're working on the code.

## Every time you work: 3 buttons

| When | Button in GitHub Desktop | What it does |
|---|---|---|
| **Before** you start | **Fetch origin → Pull origin** | gets everyone else's latest changes |
| **After** you finish | type a short summary (bottom left) → **Commit to main** | saves a checkpoint |
| Right after that | **Push origin** | uploads it for the team |

That's it. Edit files normally in between, with Arduino IDE, Word, KiCad or anything else.

## Working on the Arduino code

### One-time Arduino setup
1. Plug in the Uno (Elegoo Uno is the same thing).
2. In Arduino IDE: **Tools → Board → Arduino AVR Boards → Arduino Uno**.
3. **Tools → Port** → pick the one that appears when you plug the Uno in (e.g. `COM5`).
   If no port shows up on an Elegoo board, install the **CH340 driver** (search "CH340 driver Windows").
4. Optional: **File → Preferences → Sketchbook location** → set it to
   `Documents\GitHub\KNOCKER-TALKERS`. The sketch then shows up under **File → Sketchbook**.

Board and port settings live on your own computer, not in the repo, so they never cause conflicts.

### Opening the code (the step that matters most)
**File → Open →** `Documents\GitHub\KNOCKER-TALKERS\firmware\firmware.ino`

- ✅ Always open it **from the repo folder**.
- ❌ **Never "Save As"** somewhere else (like the default Arduino folder). That makes a copy that GitHub Desktop can't see, and your work never reaches the team.
- ❌ Don't rename the `firmware` folder or `firmware.ino`. Arduino requires the folder and the file to have the same name.

### Your Arduino work loop

| # | Where | Do this |
|---|---|---|
| 1 | GitHub Desktop | **Fetch → Pull origin** (with the sketch **closed** in Arduino IDE) |
| 2 | Arduino IDE | Open `firmware.ino`, make your change, **Ctrl+S** |
| 3 | Arduino IDE | Click **✓ Verify**. It checks that the code compiles, with no board needed |
| 4 | Arduino IDE | Click **→ Upload**, only if you have the Uno plugged in |
| 5 | GitHub Desktop | Your file shows up automatically → write a summary → **Commit to main** → **Push origin** |

**Why close the sketch before pulling?** Arduino IDE can keep showing the old version after a pull. If you then press Save, you overwrite your teammate's work. Close it → Pull → reopen. Every time.

### Testing on the real hardware
- **Serial Monitor** (magnifying glass, top right) set to **115200 baud** shows what the Uno is sending: `BIT 1`, `CHAR A`, `HR 72`.
- To hear speech, **close the Serial Monitor first**, then run `python laptop/speak.py`. Only one program can use the USB port at a time.
- On power-up the Uno calibrates: **relax → flex left → flex right**, following the prompts.
- Tuning values (thresholds, timings) are at the **top of `firmware.ino`**. Change one at a time and write what you changed in the commit summary.

### Arduino problems
| Problem | Fix |
|---|---|
| "Port busy" / "Access denied" on upload | Close the Serial Monitor and stop `speak.py`, then upload again |
| No COM port appears | Different USB cable (some are charge-only), or install the CH340 driver |
| Your changes don't show in GitHub Desktop | You saved a copy somewhere else. Reopen the file from `Documents\GitHub\KNOCKER-TALKERS\firmware` and redo the change |
| Red ✗ on GitHub after your push | Your code doesn't compile. Click ✓ Verify in Arduino IDE to see the error |

## House rules (these prevent 99% of problems)

- **Always Pull before you start.**
- **One person per file at a time.** Say it in the group chat: "I'm editing firmware.ino."
- **KiCad files: one person only**, ever, at a time. They can't be merged.
- Write a commit summary a teammate would understand: "change EMG threshold to 0.35", not "stuff".

## Even easier: no install at all

- **Upload a file** (slides, photos, a PDF): on the repo page click **Add file → Upload files**, drag it in, and click **Commit changes**.
- **Small text or code edit:** open the file on github.com, click the **pencil icon**, edit, then click **Commit changes**.

## Something went wrong?

- **"Pull" shows a conflict:** don't panic and don't delete anything. Message E. Nothing is ever lost.
- **Red ✗ next to a commit** on the repo page: the Arduino code no longer compiles. Undo it in GitHub Desktop (**History → right-click your commit → Revert**) or ask for help.
- **Made a mess before committing?** In GitHub Desktop, right-click the file → **Discard changes**, and it goes back to the last saved version.

## Who owns what

| Area | Folder | Owner |
|---|---|---|
| Arduino code | `firmware/` | _name_ |
| Laptop speech script | `laptop/` | _name_ |
| Wiring / KiCad | `hardware/` | _name_ |
| Slides, diagrams, report | `docs/` | _name_ |
