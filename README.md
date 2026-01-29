# Build

LibCIVA uses CMake as its build tool.

```sh
mkdir .\out
cd .\out
cmake ..
msbuild .\libciva.sln
```

| Option | Remark                             |
| ------ | ---------------------------------- |
| SHARED | Build shared library (default OFF) |

# Usage

## Setup

See examples

## Data output

Data from the units is output into variables via the variable manager.  
`x` is one of `1`, `2`, or `3`, indicating the respective unit

### LIBCIVA_DISPLAY_UNIT_x

Bit field, 64bits

| 63  | 62  | 61  | 60  | 59          | 58          | 57          | 56          | 55 - 52 | 51 - 48 | 47 - 44 | 43 - 40 | 39 - 36 | 35 - 32 | 31 - 28 | 27 - 24 | 23  | 22         | 21         | 20         | 19 - 16    | 15 - 12 | 11 - 08 | 07 - 04 | 03 - 00 |
| --- | --- | --- | --- | ----------- | ----------- | ----------- | ----------- | ------- | ------- | ------- | ------- | ------- | ------- | ------- | ------- | --- | ---------- | ---------- | ---------- | ---------- | ------- | ------- | ------- | ------- |
| W   | E   | S   | N   | Right 2nd . | Right 1st . | Right 2nd ° | Right 1st ° | TO      | FROM    | Right 6 | Right 5 | Right 4 | Right 3 | Right 2 | Right 1 |     | Left 2nd . | Left 1st . | Left 2nd ° | Left 1st ° | Left 5  | Left 4  | Left 3  | Left 2  | Left 1 |

Single bit fields are boolean and should illuminate/show respective symbol.  
4bit fields are characters. Convert using following:

```
if 0 <= x <= 9: 48 + c // ASCII symbol, shifted by 48
if x == 10: 'R' // ASCII R
if x == 11: 'L' // ASCII L
if x == 12: ' ' // blank space
```

### LIBCIVA_INDICATORS_UNIT_x

Bit field, 64bits

| 63 - 13 | 12                          | 11                          | 10             | 09               | 08            | 07         | 06            | 05          | 04           | 03           | 02         | 01              | 00            |
| ------- | --------------------------- | --------------------------- | -------------- | ---------------- | ------------- | ---------- | ------------- | ----------- | ------------ | ------------ | ---------- | --------------- | ------------- |
| Unset   | DME2 update indicator light | DME1 update indicator light | TO field blink | FROM field blink | WPT CHG light | WARN light | CDU BAT light | ALERT light | INSERT light | REMOTE light | HOLD light | READY NAV light | MSU BAT light |

### LIBCIVA__MODE_SELECTOR_POS_UNIT_x

| Value | Mode  |
| ----- | ----- |
| 0     | OFF   |
| 1     | STBY  |
| 2     | ALIGN |
| 3     | NAV   |
| 4     | ATT   |

### LIBCIVA_WAYPOINT_SELECTOR_POS_UNIT_x

Values 0 through 9, corresponding with the thumb wheel on the unit

### LIBCIVA_AUTO_MAN_POS_UNIT_x

| Value | Mode      |
| ----- | --------- |
| 0     | TK/GS     |
| 1     | HDG/DA    |
| 2     | XTK/TKE   |
| 3     | POS       |
| 4     | WAY PT    |
| 5     | DIS/TIME  |
| 6     | WIND      |
| 7     | DSRTK/STS |

### LIBCIVA_CROSS_TRACK_ERROR_VAR_UNIT_x

Calculated cross track error in nmi.  
Negative values indicate INS is left of course (right turn to get back).

### LIBCIVA_DESIRED_TRACK_VAR_UNIT_x

Calculated desired track in degrees.