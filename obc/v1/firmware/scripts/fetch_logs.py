#!/usr/bin/env python3
import socket
import struct
import os
import sys
import re
import time
import subprocess

# Default symbol addresses if dynamic lookup fails
DEFAULT_ADDR_FRAM_MOCK_BUFFER = 0x20001cd0
DEFAULT_ADDR_WRITE_PTR = 0x20005cd0

def get_symbol_address(elf_path, symbol_name):
    try:
        if os.path.exists(elf_path):
            out = subprocess.check_output(["arm-none-eabi-nm", elf_path], stderr=subprocess.DEVNULL).decode('utf-8')
            for line in out.splitlines():
                parts = line.split()
                if len(parts) >= 3 and parts[2] == symbol_name:
                    return int(parts[0], 16)
    except Exception:
        pass
    return None

# Level string helper
LEVEL_STRS = {
    0: "DEBUG",
    1: "INFO",
    2: "WARN",
    3: "ERROR",
    4: "CRITICAL"
}

def print_log(timestamp_ms, level, component_id, line, message):
    lvl_str = LEVEL_STRS.get(level, f"LEVEL_{level}")
    print(f"[{timestamp_ms:6d}] [{lvl_str}] [Comp: 0x{component_id:02X}] [Line: {line}] {message}")

def send_openocd_cmd(cmd):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(2.0)
        s.connect(("localhost", 4444))
        
        # Read initial greeting
        data = b""
        while b"> " not in data:
            chunk = s.recv(1024)
            if not chunk:
                break
            data += chunk
            
        # Send command
        s.sendall(cmd.encode('utf-8') + b"\n")
        
        # Read response
        response = b""
        while b"> " not in response:
            chunk = s.recv(1024)
            if not chunk:
                break
            response += chunk
            
        s.close()
        return response.decode('utf-8', errors='ignore')
    except Exception as e:
        print(f"Error communicating with OpenOCD: {e}")
        sys.exit(1)

def main():
    dump_path = "/tmp/fram_mock_buffer.bin"
    
    # Try to find obc_firmware ELF to get symbol addresses dynamically
    elf_path = "build/obc_firmware"
    if not os.path.exists(elf_path):
        # try search relative to script location
        script_dir = os.path.dirname(os.path.realpath(__file__))
        elf_path = os.path.join(script_dir, "../build/obc_firmware")
        
    addr_fram_mock_buffer = get_symbol_address(elf_path, "fram_mock_buffer")
    addr_write_ptr = get_symbol_address(elf_path, "g_fram_log_write_ptr")
    
    if addr_fram_mock_buffer is None:
        addr_fram_mock_buffer = DEFAULT_ADDR_FRAM_MOCK_BUFFER
        print(f"Using default address for fram_mock_buffer: 0x{addr_fram_mock_buffer:08X}")
    else:
        print(f"Found fram_mock_buffer address dynamically: 0x{addr_fram_mock_buffer:08X}")
        
    if addr_write_ptr is None:
        addr_write_ptr = DEFAULT_ADDR_WRITE_PTR
        print(f"Using default address for g_fram_log_write_ptr: 0x{addr_write_ptr:08X}")
    else:
        print(f"Found g_fram_log_write_ptr address dynamically: 0x{addr_write_ptr:08X}")

    print("Connecting to OpenOCD via Telnet...")
    
    # 1. Halt CPU
    print("Halting CPU...")
    send_openocd_cmd("halt")
    time.sleep(0.1)
    
    # 2. Read write pointer value
    print("Reading write pointer...")
    resp = send_openocd_cmd(f"mdw {addr_write_ptr}")
    match = re.search(f'0x{addr_write_ptr:08x}:\\s+([0-9a-fA-F]+)', resp, re.IGNORECASE)
    write_ptr = 0
    if match:
        write_ptr = int(match.group(1), 16)
    else:
        # fallback regex in case formatting is slightly different
        match = re.search(r':\s+([0-9a-fA-F]+)', resp)
        if match:
            write_ptr = int(match.group(1), 16)
            
    print(f"Write pointer: {write_ptr} (0x{write_ptr:X})")
    
    # 3. Dump memory
    print("Dumping FRAM mock buffer...")
    send_openocd_cmd(f"dump_image {dump_path} {addr_fram_mock_buffer} 16384")
    
    # 4. Resume CPU
    print("Resuming CPU...")
    send_openocd_cmd("resume")
    
    if not os.path.exists(dump_path):
        print("Error: Dump file was not created.")
        sys.exit(1)
        
    print("\nDecoding logs...")
    
    with open(dump_path, "rb") as f:
        data = f.read()
        
    offset = 0
    parsed_count = 0
    
    while offset + 10 <= len(data):
        # Header: timestamp_ms (4B), level (1B), component_id (1B), line (2B), message_len (2B)
        header = data[offset:offset+10]
        timestamp_ms, level, component_id, line, message_len = struct.unpack('<IBBHH', header)
        
        # If the header is completely empty (zeroes), we've hit unwritten memory
        if timestamp_ms == 0 and level == 0 and component_id == 0 and line == 0 and message_len == 0:
            break
            
        # Extract message
        msg_start = offset + 10
        msg_end = msg_start + message_len + 1  # include null terminator
        
        if msg_end > len(data):
            print(f"Warning: Truncated log message at offset {offset}")
            break
            
        # Decode message string (exclude trailing null char)
        message = data[msg_start:msg_end-1].decode('utf-8', errors='ignore')
        
        print_log(timestamp_ms, level, component_id, line, message)
        parsed_count += 1
        
        # Move to the next log entry
        offset = msg_end
        
    print(f"\nFinished. Decoded {parsed_count} log entries from mock FRAM.")

if __name__ == "__main__":
    main()
