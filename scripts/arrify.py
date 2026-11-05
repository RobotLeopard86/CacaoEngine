#!/usr/bin/env python3

import sys

if len(sys.argv) != 4:
    print(f"Usage: {sys.argv[0]} <input> <output> <array_name>", file=sys.stderr)
    sys.exit(1)

input_path = sys.argv[1]
output_path = sys.argv[2]
array_name = sys.argv[3]

with open(input_path, "rb") as f:
    data = f.read()

bytes_per_line = 16

with open(output_path, "w", encoding="utf-8") as out:
    out.write(
        f"constexpr std::array<unsigned char, {len(data)}> {array_name} = {{\n"
    )

    for i in range(0, len(data), bytes_per_line):
        chunk = data[i:i + bytes_per_line]
        line = ", ".join(f"0x{b:02X}" for b in chunk)

        # Add a trailing comma unless this is the final line.
        if i + bytes_per_line < len(data):
            out.write(f"    {line},\n")
        else:
            out.write(f"    {line}\n")

    out.write("};\n")