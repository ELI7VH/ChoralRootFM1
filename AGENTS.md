# ChoralRoot + SHAYPE (WaveLoop fork)

Local Lucian Labs agents also follow ../lucian-utils/AGENTS.md.
Push only origin (ELI7VH/ChoralRootFM1), never upstream.

- Read docs/SHAYPE.md for controls, timing, session migration and verification.
- SHAYPE is Pattern 13. Keep native modes, pattern IDs 1–12 and stored division IDs 0–11 stable.
- The portable donor core/fixtures retain their headers and exact provenance.
- Session v2 is 1156 bytes: its original fields remain at their old offsets, SHAYPE starts at 1024.
- Backup object 69 is the session. Restore settings 1 before session 69; the native settings PUT clears the old session.
- Native global SAVE persists SHAYPE. Do not introduce automatic musical-session saves.
- Generate headers using tools/build.generate(), then run sh tools/test_shape.sh. Keep native build guards enabled.
- ChoralRoot 0.27-wlshape3 + SHAYPE / Scale Keys was tested and archived on 2026-10-31. TEST is now back on CoralFlower 0.6.1-wlshape3.
- Future installation needs a fresh backup and stopped confirmation. SHOW and GHOUL are excluded.
