import hashlib
import os
import sys

def compute_sha256(file_path):
    sha256_hash = hashlib.sha256()
    with open(file_path, "rb") as f:
        for byte_block in iter(lambda: f.read(4096), b""):
            sha256_hash.update(byte_block)
    return sha256_hash.hexdigest()

def replace_placeholder(descriptor_path, sha256):
    with open(descriptor_path, "r") as file:
        data = file.read()
    
    data = data.replace("SHA256_PLACEHOLDER", sha256)
    
    with open(descriptor_path, "w") as file:
        file.write(data)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python replace_sha256.py <elf_file> <descriptor_file>")
        sys.exit(1)
    
    elf_file = sys.argv[1]
    descriptor_file = sys.argv[2]
    
    sha256 = compute_sha256(elf_file)
    replace_placeholder(descriptor_file, sha256)
    print(f"Replaced SHA256_PLACEHOLDER with {sha256}")
