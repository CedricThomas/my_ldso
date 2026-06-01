# ⏱️ Timing & Guide de Présentation
**Total Slot:** 30 Minutes | **Target Speaking Time:** ~24 Minutes | **Buffer / Q&A:** ~6 Minutes

---

## 📊 Global Presentation Tracking

| Section | Slides | Est. Time | Complexity | Progress |
| :--- | :--- | :--- | :--- | :--- |
| **Intro** | 1–2 | **1 min** | Low (Setup) | `[██░░░░░░░░░░░░░░░░]` |
| **Partie 1 · Qui fait quoi ?** | 3–8 | **9 mins** | Medium (Concepts) | `[█████░░░░░░░░░░░░░]` |
| **Partie 2 · Comment lire un ELF** | 9–11 | **3.5 mins** | High (Diagrams) | `[█████████░░░░░░░░░]` |
| **Partie 3 · Comment passer à l'exécution** | 12–17 | **8.5 mins** | Very High (Core Logic) | `[███████████████░░░]` |
| **Partie 4 · Apprentissages & Teaser** | 18–20 | **3.5 mins** | Low (Wrap-up) | `[██████████████████]` |

---

## 🎢 Slide-by-Slide Breakdown

| Slide | Title | Duration | Cumulative | 💡 Presenter Tips |
| :--- | :--- | :--- | :--- | :--- |
| **1** | Title | **0:30** | 00:30 | Smile, welcome. No reading. Just set the context. |
| **2** | Route | **0:30** | 01:00 | Quick overview. "We go from the file on disk to the text on screen." |
| **3** | Why Shared Libs? | **1:00** | 02:00 | Don't over-explain memory pages. Focus on "Why do we do this?" |
| **4** | Example in Mind | **1:30** | 03:30 | Point to the `puts` call. This is our North Star. |
| **5** | ⚡ Premier piège | **2:00** | 05:30 | **Key Slide.** Emphasize the "No C Standard Library" constraint. This is the hardest part. |
| **6** | `main()` is a lie | **1:00** | 06:30 | "You think `main` is first? Nope." Pivot to the actors. |
| **7** | Superpower of `ld.so` | **1:30** | 08:00 | Show the `./ld.so ./hello` command. Keep the explanation simple. |
| **8** | Binary vs `.so` | **1:00** | 09:00 | Quick comparison. "Same format, different job." |
| **9** | ELF Map | **2:00** | 11:00 | Point to the Header, PHT, and Segments. Don't read every label. |
| **10** | Navigation | **1:30** | 12:30 | The 3-step pointer chase. It's a "treasure hunt" for the loader. |
| **11** | Two Families | **1:30** | 14:00 | `PT_LOAD` (Memory) vs `PT_DYNAMIC` (Metadata). This sets up the next part. |
| **12** | `mmap()` Visual | **2:30** | 16:30 | **Heavy Slide.** Watch the clock. Focus on the flow: Disk → `mmap` → RAM. |
| **13** | Relocation | **2:00** | 18:30 | Explain the "hole" in the GOT. This is the technical "Aha!" moment. |
| **14** | Link Map | **1:00** | 19:30 | Don't read the C struct. Just say: "Our internal database of loaded libraries." |
| **15** | Symbol Lookup | **1:30** | 21:00 | How the loader finds `puts` inside `libc.so`. Keep it concrete. |
| **16** | Full Pipeline | **2:00** | 23:00 | **The Summary.** Walk through the 5 steps to wrap up the theory. |
| **17** | Lessons Learned | **1:00** | 24:00 | Quick bullet points. Emotional takeaway: "It's less magic than you think." |
| **18** | Linux vs Windows | **1:30** | 25:30 | "Same problem, different tools." Keep it short. |
| **19** | ASM is ASM | **1:00** | 26:30 | "Under the hood, it's just bytes." Transition to the next project. |
| **20** | Teaser | **1:30** | 28:00 | **High Energy.** Sell the `doom95.exe` hook. Mention "Homemade Wine". |

---

## ⚠️ Risk Areas (Where you might lose time)

1.  **Slide 12 (`mmap`):** It's a complex visual. If you start explaining every `mprotect` call, you will lose 3 minutes.
    *   **Fix:** Focus only on the flow `open → mmap → memory`.
2.  **Slide 13 (Relocation):** The GOT/PLT concept is notoriously hard to explain quickly.
    *   **Fix:** Use the "phonebook with blank pages" analogy and move on as soon as they get the idea.
3.  **Slide 5 (Premier piège):** It's easy to get lost in the "no malloc, no printf" list.
    *   **Fix:** Group them as "No standard C at all" and focus on the ASM code snippet at the bottom.

---

## 🚀 Global Presentation Tips

### 1. Pacing & Flow
*   **Aim for ~1 min 15s per slide** on average.
*   **The "Why" before the "How":** In Partie 3 (the heavy technical part), always remind *why* we are doing the step before showing the technical implementation.
*   **Watch the Clock:** Keep a timer on your side. If you hit **15:00** and you are not at Slide 11, you need to speed up.

### 2. Delivery
*   **Point, Don't Read:** On the diagram slides (9, 12, 16), use your hand to trace the flow on the screen. Do not stand still and read the bullet points.
*   **The "Why" First:** Before showing *how* `ld.so` does something (e.g., `mmap`), spend 5 seconds reminding them *why* it needs to happen (e.g., "The code is on the disk, but the CPU can't execute files. It needs RAM.").

### 3. Handling Q&A
*   **The "Parking Lot":** If someone asks a deep technical question during the core slides (Partie 2 or 3), say: *"Great question, that's a bit off our main path. Let's park it and I'll answer after the pipeline summary so we don't lose the thread."*
*   **Demo Buffer:** If you run a live demo, keep it to a single terminal window pre-opened. Type only the `./hello` execution. Do not live-type `gcc` commands unless you are 100% sure of the flags.

### 4. Ending Strong
*   **The Teaser (Slide 20):** This is your "movie trailer." Increase your energy here. Make eye contact. The goal is to make them curious enough to come back for the `mon_wine` presentation.
