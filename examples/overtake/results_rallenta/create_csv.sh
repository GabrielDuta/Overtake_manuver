#!/bin/bash

# Check if the directory is provided as an argument
if [ $# -eq 0 ]; then
    echo "Error: Please provide a directory path as an argument."
    exit 1
fi

# Get the directory path from the first argument
directory="$1"

# Check if the directory exists
if [ ! -d "$directory" ]; then
    echo "Error: Directory '$directory' does not exist."
    exit 1
fi

# Change to the specified directory
cd "$directory"

# Temporary CSV file name
temp_csv="temp.csv"

# Process all .vec files in the directory
for vec_file in *.vec; do
    if [ -f "$vec_file" ]; then
        csv_file="${vec_file%.vec}.csv"
        
        # Apply opp_vec2csv.pl
        echo "Converting $vec_file to CSV..."
        if ! ../opp_vec2csv.pl --merge-by em -F co2emission "$vec_file" > "$temp_csv"; then
            echo "Error converting $vec_file"
            continue
        fi
        
        # Sort the CSV file
        echo "Sorting $csv_file..."
        if ! sort -n "$temp_csv" > "$csv_file"; then
            echo "Error sorting $csv_file"
            rm -f "$temp_csv"
            continue
        fi
        
        echo "Successfully processed $vec_file to $csv_file"
        
        # Clean up temporary file
        rm -f "$temp_csv"
    fi
done

echo "Processing completed."

