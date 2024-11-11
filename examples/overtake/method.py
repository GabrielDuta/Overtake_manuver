import os
import glob
import numpy as np
import matplotlib.pyplot as plt

# Define the base directory paths
dir1 = './results_rallenta/'

# Initialize empty dictionaries to store aggregated data
distance_data = {}
speed_data = {}

# Function to extract speed, distance, and method from filename
def extract_params(filename):
    params = os.path.basename(filename).split('_')
    return float(params[1]), float(params[2])

# Function to parse method from .sca file
def parse_method_from_sca(filename):
    with open(filename, 'r') as f:
        content = f.read()
    
    lines = content.split('\n')
    method = None
    
    for line in lines:
        if "Overtake_method" in line:
            parts = line.split()
            method = int(parts[-1])
    
    return method

# Read .sca files and aggregate data from dir1
for file in glob.glob(os.path.join(dir1, 'OvertakeNoGui_*.sca')):
    speed, distance = extract_params(file)
    
    method = parse_method_from_sca(file)
    
    if method is not None:
        # Add distance to distance_data1 if it doesn't exist
        if distance not in distance_data:
            distance_data[distance] = {1: 0, 2: 0, 3: 0}
        distance_data[distance][method] += 1

        # Add speed to speed_data1 if it doesn't exist
        if speed not in speed_data:
            speed_data[speed] = {1: 0, 2: 0, 3: 0}
        speed_data[speed][method] += 1
    else:
        print("Erro: ", speed, " ", distance)


for d in distance_data:
    print(distance_data[d])

# Prepare data for plotting
distances = sorted(set(list(distance_data.keys())))
speeds = sorted(set(list(speed_data.keys())))

methods = [1, 2, 3]

# Create figure with two subplots
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(16, 6), gridspec_kw={'width_ratios': [1, 1]})

#for method in methods:
ax1.bar(distances, [distance_data[d].get(1, 0) for d in distances], label=f'Method {1}')
ax1.bar(distances, [distance_data[d].get(2, 0) for d in distances], label=f'Method {2}')
ax1.bar(distances, [distance_data[d].get(3, 0) for d in distances], label=f'Method {3}')
ax2.bar(speeds, [speed_data[s].get(1, 0) for s in speeds], label=f'Method {1}')
ax2.bar(speeds, [speed_data[s].get(2, 0) for s in speeds], label=f'Method {2}')
ax2.bar(speeds, [speed_data[s].get(3, 0) for s in speeds], label=f'Method {3}')

ax1.set_title('Method Usage vs Distance')
ax1.set_xlabel('Distance')
ax1.set_ylabel('Count')
ax1.legend()

ax2.set_title('Method Usage vs Speed')
ax2.set_xlabel('Speed')
ax2.set_ylabel('Count')
ax2.legend()

# Layout so plots do not overlap
fig.tight_layout()

# Show the plot
plt.show()

# Optionally save the plot
plt.savefig('overtake_no_gui_method_usage_comparison.png')

plt.close(fig)

