import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import os

def parse_file(filename):
    with open(filename, 'r') as f:
        content = f.read()
    
    lines = content.split('\n')
    for line in lines:
        if "Overtake_stop" in line:
            parts = line.split()
            return float(parts[-1])
    return None

def main():
    data = []
    speeds = []
    distances = []

    # Get all files matching the pattern
    files = [f for f in os.listdir() if f.startswith("OvertakeNoGui_") and f.endswith(".sca")]

    for file in files:
        speed, distance = file.split('_')[1:3]
        time_taken = parse_file(file)
        
        if time_taken is not None:
            speeds.append(float(speed))
            distances.append(float(distance))
            data.append(time_taken)

    # Create a 3D plot
    fig = plt.figure(figsize=(12, 8))
    ax = fig.add_subplot(111, projection='3d')

    # Plot the data
    ax.scatter(speeds, distances, data, c=data, cmap='viridis')

    # Set labels and title
    ax.set_xlabel('Speed')
    ax.set_ylabel('Distance')
    ax.set_zlabel('Time Taken (seconds)')
    ax.set_title('Time Taken vs Speed and Distance')

    # Add colorbar
    fig.colorbar(ax.collections[0], label='Time Taken (seconds)')

    # Show the plot
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    main()

