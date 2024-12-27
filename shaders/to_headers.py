import os
import subprocess
import binascii

def compile_shader(shader_path, output_path):
    subprocess.run(['glslc', shader_path, '-o', output_path], check=True)

def create_header(shader_name, spv_path, header_path):
    with open(spv_path, 'rb') as spv_file:
        spv_content = spv_file.read()

    hex_content = binascii.hexlify(spv_content).decode('ascii')

    with open(header_path, 'w') as header_file:
        header_file.write(f'// This file was generated by shaders/to_headers.py\n\n')
        header_file.write(f'#pragma once\n\n')
        header_file.write(f'constexpr unsigned char {shader_name}_spv[] = {{\n')
        for i in range(0, len(hex_content), 2):
            header_file.write(f'0x{hex_content[i:i+2]}, ')
            if (i + 2) % 32 == 0:
                header_file.write('\n')
        header_file.write(f'}};\n\n')
        header_file.write(f'constexpr unsigned int {shader_name}_spv_len = sizeof({shader_name}_spv);\n')

def process_shaders():
    for filename in os.listdir('.'):
        if filename.endswith('.comp'):
            shader_path = os.path.join('.', filename)
            shader_name = os.path.splitext(filename)[0]
            spv_path = os.path.join('.', f'{shader_name}.spv')
            header_path = os.path.join('.', f'../lib_includes/{shader_name}_comp.h')

            compile_shader(shader_path, spv_path)
            create_header(shader_name, spv_path, header_path)

            print(f'Processed: {filename}')

if __name__ == '__main__':
    process_shaders()