import os
import re

def replace_commas_with_newlines(content):
    lines = []
    current_line = ""
    comma_count = 0
    
    for char in content:
        if char == ',':
            comma_count += 1
            if comma_count % 4 == 0:
                lines.append(current_line.strip())
                current_line = ""
            else:
                current_line += char
        else:
            current_line += char
    
    # Add the last line if it's not empty
    if current_line.strip():
        lines.append(current_line.strip())
    
    return "\n".join(lines)

def process_csv_file(file_path):
    with open(file_path, 'r') as file:
        content = file.read()

    modified_content = re.sub(r'(?<!,)\s+(?!")', ',', content)
    final_content = replace_commas_with_newlines(modified_content)

    with open(file_path, 'w') as file:
        file.write(final_content)

    print(f"Processed file: {file_path}")

def main():
    directory_path = './'

    for filename in os.listdir(directory_path):
        if filename.endswith('.csv'):
            file_path = os.path.join(directory_path, filename)
            process_csv_file(file_path)

if __name__ == "__main__":
    main()
