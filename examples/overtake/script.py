def filter_info_lines(input_file, output_file):
    try:
        # Open the input file in read mode
        with open(input_file, 'r') as infile:
            # Open the output file in write mode
            with open(output_file, 'w') as outfile:
                # Iterate through each line in the input file
                for line in infile:
                    # Check if the line starts with "[INFO]"
                    if "consumo" in line:
                        # Write the line to the output file
                        outfile.write(line)

        print(f"Filtered lines have been written to {output_file}")
    except FileNotFoundError:
        print(f"The file {input_file} does not exist.")
    except Exception as e:
        print(f"An error occurred: {e}")

# Example usage
input_file = "log.txt"  # Replace with your input file name
output_file = "out.txt"  # Replace with your desired output file name

filter_info_lines(input_file, output_file)

