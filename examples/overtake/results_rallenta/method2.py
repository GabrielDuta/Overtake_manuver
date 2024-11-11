import os
import re
import matplotlib.pyplot as plt
from collections import defaultdict

def parse_file(filename):
    with open(filename, 'r') as f:
        content = f.read()
    
    match = re.search(r'Overtake_method\s+(\d)', content)
    if match:
        return int(match.group(1))
    else:
        return None

def plot_results(results):
    fig, axs = plt.subplots(2, figsize=(12, 10))

    # Plot speed-based results
    for distance, methods in results.items():
        speeds = sorted(methods.keys())
        counts = [methods[speed] for speed in speeds]
        
        axs[0].plot(speeds, counts, marker='o', label=f'distance={distance}')
    
    axs[0].set_title('Overtake Methods by Speed')
    axs[0].set_xlabel('Speed')
    axs[0].set_ylabel('Count')
    axs[0].legend()

    # Plot distance-based results
    for speed, methods in results.items():
        distances = sorted(methods.keys())
        counts = [methods[distance] for distance in distances]
        
        axs[1].plot(distances, counts, marker='o', label=f'speed={speed}')
    
    axs[1].set_title('Overtake Methods by Distance')
    axs[1].set_xlabel('Distance')
    axs[1].set_ylabel('Count')
    axs[1].legend()

    plt.tight_layout()
    plt.show()

def main():
    results = defaultdict(lambda: defaultdict(int))

    # Iterate through all files matching the pattern
    for filename in os.listdir('.'):
        match = re.match(r'OvertakeNoGui_(\d+)_(\d+)_0\.sca', filename)
        if match:
            speed, distance = map(int, match.groups())
            
            method = parse_file(filename)
            if method is not None:
                results[speed][distance] += 1
    
    # Count occurrences of each method for each speed and distance
    final_results = defaultdict(lambda: defaultdict(lambda: [0]*3))
    
    for speed, distances in results.items():
        for distance, count in distances.items():
            final_results[speed][distance] = [results[speed][distance]] * 3
    
    plot_results(final_results)

if __name__ == "__main__":
    main()
