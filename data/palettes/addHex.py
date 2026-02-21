import os
import re

def convert_palette(input_file, output_file):
    with open(input_file, 'r') as f:
        lines = f.readlines()

    output = []
    for line in lines:
        # Keep header lines as-is
        if line.startswith('GIMP') or line.startswith('#') or line.strip() == '':
            output.append(line)
            continue

        # Match RGB values and name
        match = re.match(r'\s*(\d+)\s+(\d+)\s+(\d+)\s*(.*)', line)
        if match:
            r, g, b, name = match.groups()
            hex_code = f'{int(r):02x}{int(g):02x}{int(b):02x}'
            output.append(f'{int(r):<3} {int(g):<3} {int(b):<3}\t{hex_code} {name}\n')
        else:
            output.append(line)

    with open(output_file, 'w') as f:
        f.writelines(output)

convert_palette('data/palettes/BasePal2.gpl', 'data/palettes/output.gpl')
# ```

# This will turn your palette from:
# ```
#   0   0   0	Untitled
# 238   0   0	Untitled
# ```
# into:
# ```
# 0   0   0	000000 Untitled
# 238   0   0	ee0000 Untitled