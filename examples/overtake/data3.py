import os
import glob
import pandas as pd
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
    return int(params[1]), int(params[2])

# Function to extract CO2 emissions for specific nodes
def extract_node_emissions(df):
    #node2_emission = df.loc[df['node'] == 'Overtake.node[2].mobility', 'co2emission'].iloc[-1]
    #node3_emission = df.loc[df['node'] == 'Overtake.node[3].mobility', 'co2emission'].iloc[-1]
    #return node2_emission + node3_emission
    filtered_df = df[~df['node'].str.contains('|'.join(['node\[0\]', 'node\[1\]']), na=False)]
    return filtered_df['co2emission'].sum()

# Read CSV files and aggregate data from dir1
for file in glob.glob(os.path.join(dir1, 'OvertakeNoGui_*.csv')):
    df = pd.read_csv(file)
    
    speed, distance = extract_params(file)
    
    total_co2 = extract_node_emissions(df)
    
    # Add distance to distance_data1 if it doesn't exist
    if distance not in distance_data1:
        distance_data1[distance] = []
    distance_data1[distance].append(total_co2)

    # Add speed to speed_data1 if it doesn't exist
    if speed not in speed_data1:
        speed_data1[speed] = []
    speed_data1[speed].append(total_co2)

# Read CSV files and aggregate data from dir2
for file in glob.glob(os.path.join(dir2, 'OvertakeNoGui_*.csv')):
    df = pd.read_csv(file)
    
    speed, distance = extract_params(file)
    
    total_co2 = extract_node_emissions(df)
    
    # Add distance to distance_data2 if it doesn't exist
    if distance not in distance_data2:
        distance_data2[distance] = []
    distance_data2[distance].append(total_co2)

    # Add speed to speed_data2 if it doesn't exist
    if speed not in speed_data2:
        speed_data2[speed] = []
    speed_data2[speed].append(total_co2)

# Prepare data for plotting
distances = sorted(set(list(distance_data1.keys()) + list(distance_data2.keys())))
speeds = sorted(set(list(speed_data1.keys()) + list(speed_data2.keys())))

# Calculate average emissions for each distance and speed
avg_distance_emissions1 = np.array([np.mean(distance_data1.get(d, [])) for d in distances])
avg_speed_emissions1 = np.array([np.mean(speed_data1.get(s, [])) for s in speeds])

avg_distance_emissions2 = np.array([np.mean(distance_data2.get(d, [])) for d in distances])
avg_speed_emissions2 = np.array([np.mean(speed_data2.get(s, [])) for s in speeds])

# Create figure with two subplots
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(16, 6), gridspec_kw={'width_ratios': [1, 1]})

# Plot CO2 emissions vs distance
ax1.plot(distances, avg_distance_emissions1, marker='o', color='blue', label='Sorpasso senza rallentamento')
ax1.plot(distances, avg_distance_emissions2, marker='o', color='red', label='Sorpasso con rallentamento')
ax1.set_title('Media emissioni sulle distanze')
ax1.set_xlabel('Distanza (m)')
ax1.set_ylabel('Media emissioni totali di CO2 (mg)')
ax1.set_ylim(25600, 26600)
ax1.legend()

# Plot CO2 emissions vs speed
ax2.plot(speeds, avg_speed_emissions1, marker='o', color='blue', label='Sorpasso senza rallentamento')
ax2.plot(speeds, avg_speed_emissions2, marker='o', color='red', label='Sorpasso con rallentamento')
ax2.set_title('Media emissioni sulle velocità')
ax2.set_xlabel('Velocità (km/h)')
ax2.set_ylabel('Media emissioni totali di CO2 (mg)')
ax2.set_ylim(25600, 26600)
ax2.legend()

# Layout so plots do not overlap
fig.tight_layout()

# Show the plot
plt.show()

# Optionally save the plot
plt.savefig('overtake_no_gui_avg_plots_comparison_nodes.png')

