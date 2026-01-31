# Navigation

- Leg switching assumes 30°
  - Configurable?

# Other

- BAT operation behaviour
  - BAT CDU Lamp when on bat
  - BAT MSU lamp when auto shutdown due to low bat
- TAXI in ALIGN error
- SB Import (ADEU)

# Updating

- Entry to #4
- DME receiver connections
- DME Update
  - AI trickle down to 1 (0 on dual DME) if ALT INF correct, down to 2 otherwise
  - Freq compare entered with tuned
    - Not a thing IRL
  - Unit 1 connected to NAV 1
  - Unit 2 connected to NAV 2
  - Unit 3 gets data form Units 1 and 2
  - Offside DME Update
    - Simply reduces AI in offside units
    - If onside update is running in conjunction to offside (Unit 3 -> double offside)
      - if stations not equal -> increase error correction rate

# Multi Unit

- Unit intercom
  - Remote key
- Tripple Mix
  - TESTING REQUIRED
    - Display
    - Navigation function
    - Dropout if any unit is in AI !0 4

# Sim specific

- To approximate drift and updating:
  - NAV equations use current pos of slaved unit / tripple mix pos if avail
  - Current pos is the only one that has drift errors getting reduced
  - Initial position stays w/o any updates (Eradication behaviour highlight)
  - In order to "reduce" drift, SIM position must be used to determine original "offset" of current/initial pos
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
    - Ensure that in 3sigma, the error rates at AI X create X nmi radial error
    - Error per dTime is dependant on GS (technically accel but meh)
      - Stationary still incurs penalty, but very little (integrator errors)

# MSFS Unit specific

# Documentation
