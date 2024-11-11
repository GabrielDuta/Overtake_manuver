import os
import re
import matplotlib.pyplot as plt
import numpy as np
from collections import defaultdict

# Directory containing the files
#directory = './results_rallenta'
directory = './results'

# Initialize dictionaries to store counts for each method per speed and distance
distance_counts = defaultdict(lambda: {1: 0, 2: 0, 3: 0})
speed_counts = defaultdict(lambda: {1: 0, 2: 0, 3: 0})

# Regex pattern to extract speed and distance from the filename
filename_pattern = re.compile(r"OvertakeNoGui_(\d+)_(\d+)_0\.sca")

# Iterate through each file in the directory
for filename in os.listdir(directory):
    # Match the filename pattern
    match = filename_pattern.match(filename)
    if match:
        # Extract speed and distance
        speed = int(match.group(1))
        distance = int(match.group(2))
        
        # Open and read the file to find the Overtake_method
        with open(os.path.join(directory, filename), 'r') as file:
            for line in file:
                if "Overtake_method" in line:
                    # Extract the overtake method (integer at the end of the line)
                    overtake_method = int(line.split()[-1])
                    if overtake_method in [1, 2, 3]:  # Ensure the method is within the expected range
                        # Update counts for both speed and distance
                        distance_counts[distance][overtake_method] += 1
                        speed_counts[speed][overtake_method] += 1
                    break

# Define method labels
method_labels = {1: "Sorpasso normale", 2: "C rallenta", 3: "B attende C"}

# Function to create a stacked bar plot on a specified subplot axis
def plot_stacked_counts(ax, data_counts, title, xlabel):
    # Sort the keys for consistent plotting order
    keys = sorted(data_counts.keys())
    methods = [1, 2, 3]  # The three overtake methods

    # Prepare data for plotting
    x = np.arange(len(keys))
    method_counts = {method: [data_counts[key][method] for key in keys] for method in methods}

    # Stacked bar plot
    bottom = np.zeros(len(keys))
    for method in methods:
        ax.bar(x, method_counts[method], bottom=bottom, label=method_labels[method])
        bottom += method_counts[method]  # Update bottom to stack bars

    # Labeling
    ax.set_xlabel(xlabel)
    ax.set_ylabel('Conta')
    ax.set_title(title)
    ax.set_xticks(x)
    ax.set_xticklabels(keys)
    ax.legend()
    
    # Set y-axis ticks from 0 to 21
    ax.set_yticks(np.arange(0, 23, 1))

# Create a single figure with two subplots side by side
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))

# Plot distance vs. overtake method count on the first subplot
plot_stacked_counts(ax1, distance_counts, '', 'Distanze (m)')

# Plot speed vs. overtake method count on the second subplot
plot_stacked_counts(ax2, speed_counts, '', 'Velocità (km/h)')

# Adjust layout for better spacing
plt.tight_layout()
plt.show()
