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
