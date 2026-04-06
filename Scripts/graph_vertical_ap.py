import matplotlib.pyplot as plt
import csv

timestamps: list[float] = []
desiredAltitudes: list[float] = []
actualAltitudes: list[float] = []

with open('./inputs/vertical_autopilot_log.csv', 'r') as f:
    reader = csv.DictReader(f)
    for row in reader:
        timestamps.append(float(row['timestamp']))
        desiredAltitudes.append(float(row['targetAltitude']))
        actualAltitudes.append(float(row['altitude']))

plt.figure(figsize=(12, 6))
plt.plot(timestamps, desiredAltitudes, label='Desired Altitude', linestyle='--')
plt.plot(timestamps, actualAltitudes, label='Actual Altitude', linestyle='-')
plt.xlabel('Timestamp (s)')
plt.ylabel('Altitude (feet)')
plt.title('Vertical Autopilot Response')
plt.legend()
plt.grid(True)
plt.show()
