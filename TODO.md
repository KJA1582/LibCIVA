# WPT Entry

# Other Displays

- XTK/TKE
- DIS/TIME
- DSRTK/STS

# DME entry
 
 - Zero not settable, shows DME 1

# Navigation

- Export of desired track true
- Export of desired heading true
- Export of distance to target
- Leg switching assumes 30°
- WPT CHG
  - Copntray to FSX, WPT CHG INSERT stays illuminated when data pos is changed. CLEAR clears it 
- AUTO/MAN
  - 25.6s min time, only change after that in man
  - ALERT LAMP

# Other

- TEST mode
- MALF clearing
- BAT operation behaviour
  - BAT CDU Lamp when on bat
  - BAT MSU lamp when auto shutdown due to low bat
- TAXI in ALIGN error
- HOLD button behaviour
  - No effect in any but POS/WPT
  - POS -> freeze
  - WPT -> show INS pos (the one w/o updated)

# Updating

- Entry to #4
- Eradication (#1)
- Manual Update
  - AI to 1 immediate
  - Error when update results in 33nmi shift
- DME Update
  - AI trickle down to 0 if ALT INF correct, down to 2 otherwise
  - Freq compare entered with tuned
  - Unit 1 connected to NAV 1
  - Unit 2 connected to NAV 2
  - Unit 3 gets data form Units 1 and 2
  - Offside DME Update
    - Figure out how that works with thest flight

# Multi Unit

- Unit intercom
- Tripple Mix

# Sim specific

- To approximate drift and updating:
  - NAV equations use display pos of slaved unit / tripple mix pos if avail
  - Display pos is the only one that has drift errors getting reduced
  - INS position stays w/o any updates (Eradication behaviour highlight)
  - In order to "reduce" drift, SIM position must be used to determin original "offset" of display/INS pos
    - Updates reduce both error rate and delta of actual SIM pos and display pos
  - Entering NAV at 5 starts with higher error rate
  - Running NAV on ground will increase error, but very slowly

Notes on PI:

- AI indicates 3sigma radial error in nmi
- AI9 is max displayable and reached after 3h w/o updates
  - After 3 hours, 3sigma deviation is 9nmi
  - After 6 hours, 3sigma deviation is 18nmi
    - Display still only reads 9, so yikes
  - sigma is 1/3 nmi
    - 68.3% of the time, AI1 -> <=1/3nmi deviation
    - 27.2% of the time, AI1 -> > 1/3 and <= 2/3nmi deviation
    - 4.2% of the time, AI1 -> > 2/3nmi and <= 1nmi deviation
    - 0.3% of the time, AI1 -> > 1nmi deviation
  - Impl: https://en.cppreference.com/w/cpp/numeric/random/normal_distribution.html
    - At unit power up, pick random number for error rate in lat/lon
    - Ensure that in 3sigma, the error rates at AIX create Xnmi radial error
    - Error per dTime is dependant on GS (technially accel but meh)
      - Stationary still incurs penalty, but very little (integrator errors)

# MSFS Unit specific

- VarManager export all flag, if not, only minimum data is exported
- Controllable via LVAR+EVENT