import matplotlib.pyplot as plt
import csv

timestamps: list[float] = []
desiredBanks: list[float] = []
actualBanks: list[float] = []

with open('./inputs/lateral_autopilot_log.csv', 'r') as f:
    reader = csv.DictReader(f)
    for row in reader:
        timestamps.append(float(row['timestamp']))
        desiredBanks.append(float(row['desiredBank']))
        actualBanks.append(float(row['bankAngle']))

plt.figure(figsize=(12, 6))
plt.plot(timestamps, desiredBanks, label='Desired Bank', linestyle='--')
plt.plot(timestamps, actualBanks, label='Actual Bank', linestyle='-')
plt.xlabel('Timestamp (s)')
plt.ylabel('Bank Angle (degrees)')
plt.title('Lateral Autopilot Response')
plt.legend()
plt.grid(True)
plt.show()
