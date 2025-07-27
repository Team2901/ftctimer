import json
import os
import sys

# Path to your package.json file
package_json_path = os.path.join(os.path.dirname(__file__), 'package.json')

# Path to your version.h file
version_h_path = os.path.join(os.path.dirname(__file__), 'main', 'version.h')

# Read version from package.json
with open(package_json_path, 'r') as f:
    data = json.load(f)
    version = data.get('version', '0.0.0')

def get_version(version, output_file):
    header_content = f'set(PROJECT_VER "{version}")\n'
    with open(output_file, 'w') as f:
        f.write(header_content)

if __name__ == "__main__":
    output_file = sys.argv[1]
    get_version(version, output_file)
    # Write version to version.h
    with open(version_h_path, 'w') as f:
        f.write(f'// Automatically generated from package.json\n')
        f.write(f'#pragma once\n\n')
        f.write(f'#define FIRMWARE_VERSION "{version}"\n')
