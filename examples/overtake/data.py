import os
import glob
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

# Define the base directory path
base_dir = './'

# Initialize dictionary to store aggregated data
data = {}

# Function to extract speed and distance from filename
def extract_params(filename):
    params = os.path.basename(filename).split('_')
    return int(params[1]), int(params[2])

# Read CSV files and aggregate data
for file in glob.glob(os.path.join(base_dir, 'OvertakeNoGui_*.csv')):
    df = pd.read_csv(file)
    
    # Filter out rows containing 'node[0]' or 'node[1]'
    filtered_df = df[~df['node'].str.contains('|'.join(['node\[0\]', 'node\[1\]']), na=False)]
    
    speed, distance = extract_params(file)
    
    if speed not in data:
        data[speed] = {}
    
    # Calculate total CO2 emissions only for the filtered data
    total_co2 = filtered_df['co2emission'].sum()
    
    data[speed][distance] = total_co2

# Prepare data for 3D plotting
speeds = sorted(data.keys())
distances = sorted(set(distance for d in data.values() for distance in d.keys()))
X, Y = np.meshgrid(distances, speeds)
Z = np.array([[data[y][x] for x in distances] for y in speeds])

# Create 3D plot
fig = plt.figure(figsize=(12, 8))
ax = fig.add_subplot(111, projection='3d')

surf = ax.plot_surface(X, Y, Z, cmap='viridis', edgecolor='none')

ax.set_xlabel('Distance (m)')
ax.set_ylabel('Speed (km/h)')
ax.set_zlabel('Total CO2 Emissions')

fig.colorbar(surf, shrink=0.5, aspect=5)

plt.title('CO2 Emissions vs Speed and Distance (excluding node[0] and node[1])')
plt.tight_layout()

# Show the plot
plt.show()

# Optionally save the plot
plt.savefig('overtake_no_gui_3d_plot_excluding_nodes.png')

