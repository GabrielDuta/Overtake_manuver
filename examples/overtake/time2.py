import os
import glob
import numpy as np
import matplotlib.pyplot as plt

# Define the base directory paths
dir1 = './results/'
dir2 = './results_rallenta/'

# Initialize empty dictionaries to store aggregated data
distance_data1 = {}
speed_data1 = {}
distance_data2 = {}
speed_data2 = {}

# Function to extract speed and distance from filename
def extract_params(filename):
    params = os.path.basename(filename).split('_')
    return float(params[1]), float(params[2])

# Function to parse time from .sca file
def parse_time_from_sca(filename):
    with open(filename, 'r') as f:
        content = f.read()
    
    lines = content.split('\n')
    for line in lines:
        if "Overtake_stop" in line:
            parts = line.split()
            return float(parts[-1])
    return None

# Read .sca files and aggregate data from dir1
for file in glob.glob(os.path.join(dir1, 'OvertakeNoGui_*.sca')):
    speed, distance = extract_params(file)
    
    time_taken = parse_time_from_sca(file)
    
    if time_taken is not None:
        # Add distance to distance_data1 if it doesn't exist
        if distance not in distance_data1:
            distance_data1[distance] = []
        distance_data1[distance].append(time_taken)

        # Add speed to speed_data1 if it doesn't exist
        if speed not in speed_data1:
            speed_data1[speed] = []
        speed_data1[speed].append(time_taken)

# Read .sca files and aggregate data from dir2
for file in glob.glob(os.path.join(dir2, 'OvertakeNoGui_*.sca')):
    speed, distance = extract_params(file)
    
    time_taken = parse_time_from_sca(file)
    
    if time_taken is not None:
        # Add distance to distance_data2 if it doesn't exist
        if distance not in distance_data2:
            distance_data2[distance] = []
        distance_data2[distance].append(time_taken)

        # Add speed to speed_data2 if it doesn't exist
        if speed not in speed_data2:
            speed_data2[speed] = []
        speed_data2[speed].append(time_taken)

# Prepare data for plotting
distances = sorted(set(list(distance_data1.keys()) + list(distance_data2.keys())))
speeds = sorted(set(list(speed_data1.keys()) + list(speed_data2.keys())))

# Calculate average time for each distance and speed
avg_distance_times1 = np.array([np.mean(distance_data1.get(d, [])) for d in distances])
avg_speed_times1 = np.array([np.mean(speed_data1.get(s, [])) for s in speeds])

avg_distance_times2 = np.array([np.mean(distance_data2.get(d, [])) for d in distances])
avg_speed_times2 = np.array([np.mean(speed_data2.get(s, [])) for s in speeds])

# Create figure with two subplots
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(16, 6), gridspec_kw={'width_ratios': [1, 1]})

# Plot time vs distance
ax1.plot(distances, avg_distance_times1, marker='o', color='blue', label='Sorpasso senza rallentamento')
ax1.plot(distances, avg_distance_times2, marker='o', color='red', label='Sorpasso con rallentamento')
ax1.set_xlabel('Distanza (m)')
ax1.set_ylabel('Media tempo impiegato (s)')
ax1.set_ylim([10, 27])  # Set y-axis range from 10 to 26
ax1.legend()

# Plot time vs speed
ax2.plot(speeds, avg_speed_times1, marker='o', color='blue', label='Sorpasso senza rallentamento')
ax2.plot(speeds, avg_speed_times2, marker='o', color='red', label='Sorpasso con rallentamento')
ax2.set_xlabel('Velocità (km/h)')
ax1.set_ylabel('Media tempo impiegato (s)')
ax2.set_ylim([10, 27])  # Set y-axis range from 10 to 26
ax2.legend()

# Layout so plots do not overlap
fig.tight_layout()

# Show the plot
plt.show()

# Optionally save the plot
plt.savefig('overtake_no_gui_avg_time_plots_comparison.png')

