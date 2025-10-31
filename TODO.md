# Navigation

- Export of desired track true
- Export of desired heading true
- Leg switching assumes 30°
  - Configurable?
- AUTO/MAN
  - 25.6s min time, only change after that in auto
  - ALERT LAMP
    - AUTO/MAN 2min
    - AUTO blink if leg time < 25.6s and past point
    - MAN blink if past point

# Other

- BAT operation behaviour
  - BAT CDU Lamp when on bat
  - BAT MSU lamp when auto shutdown due to low bat
- TAXI in ALIGN error

# Updating

- Entry to #4
- DME Select
  - WP CHG button  
- DME Update
  - AI trickle down to 1 (0 on dual DME) if ALT INF correct, down to 2 otherwise
  - Freq compare entered with tuned
  - Unit 1 connected to NAV 1
  - Unit 2 connected to NAV 2
  - Unit 3 gets data form Units 1 and 2
  - Offside DME Update
    - Simply reduces AI in offside units
    - If onside update is running in conjunction to offside (Unit 3 -> double offside)
      - if stations not equal -> increase error correction rate

# Multi Unit

- Unit intercom
- Tripple Mix

# Sim specific

- To approximate drift and updating:
  - NAV equations use current pos of slaved unit / tripple mix pos if avail
  - Current pos is the only one that has drift errors getting reduced
  - Initial position stays w/o any updates (Eradication behaviour highlight)
  - In order to "reduce" drift, SIM position must be used to determin original "offset" of current/initial pos
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
    - Ensure that in 3sigma, the error rates at AI X create Xnmi radial error
    - Error per dTime is dependant on GS (technially accel but meh)
      - Stationary still incurs penalty, but very little (integrator errors)

# MSFS Unit specific

- Setup via VarManager (no of units, export options, heading steering)
- Controllable via LVAR+EVENT