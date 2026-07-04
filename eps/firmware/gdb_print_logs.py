import struct
import time
import gdb

def print_logs():
    # Disable pagination
    gdb.execute("set pagination off")
    gdb.execute("set target-async on")
    
    # Connect to target
    try:
        gdb.execute("target remote localhost:3333")
    except Exception as e:
        print("Failed to connect to GDB server: {}".format(e))
        return

    # Let the target run for 2.0 seconds to boot up and initialize logs
    print("Running target for 2.0s to let it initialize...")
    gdb.execute("continue &")
    time.sleep(2.0)
    
    print("Interrupting target...")
    gdb.execute("interrupt")
    # Wait for target to halt
    while gdb.selected_thread().is_running():
        time.sleep(0.05)
    print("Target halted successfully.")

    # Read log_ring_buffer address
    try:
        rb_addr = int(gdb.parse_and_eval("&log_ring_buffer"))
    except Exception as e:
        print("Failed to get address of log_ring_buffer: {}".format(e))
        return

    # Read 17 bytes of the struct from target memory
    try:
        inferior = gdb.selected_inferior()
        struct_bytes = bytes(inferior.read_memory(rb_addr, 17))
        buffer_ptr, capacity, head, tail, overwrite = struct.unpack("<IIIIB", struct_bytes)
    except Exception as e:
        print("Failed to read log_ring_buffer struct memory: {}".format(e))
        return

    print("Ring buffer status: head={}, tail={}, capacity={}, buffer_addr=0x{:08X}".format(head, tail, capacity, buffer_ptr))

    if head == tail:
        print("No log entries in ring buffer.")
        return

    # Read buffer memory
    try:
        buf = bytes(inferior.read_memory(buffer_ptr, capacity))
    except Exception as e:
        print("Failed to read target buffer memory: {}".format(e))
        return

    # Helper to read from ring buffer
    def read_bytes(offset, length):
        res = bytearray()
        for i in range(length):
            res.append(buf[(offset + i) % capacity])
        return bytes(res)

    # Parse slog entries
    offset = tail
    level_names = {0: "DEBUG", 1: "INFO", 2: "WARN", 3: "ERROR", 4: "CRITICAL"}
    
    entries_printed = 0
    while offset != head:
        bytes_avail = (head - offset + capacity) % capacity
        if bytes_avail < 10:
            break
            
        hdr_bytes = read_bytes(offset, 10)
        timestamp_ms, level, component_id, line, message_len = struct.unpack("<IBBHH", hdr_bytes)
        
        if bytes_avail < 10 + message_len + 1:
            break
            
        msg_bytes = read_bytes(offset + 10, message_len + 1)
        message = msg_bytes[:-1].decode('utf-8', errors='replace').replace('\x00', '')
        
        level_str = level_names.get(level, "UNKNOWN")
        print("[{:8d} ms] [Comp {:02X}] [{:8s}] Line {:4d}: {}".format(
            timestamp_ms, component_id, level_str, line, message
        ))
        
        offset = (offset + 10 + message_len + 1) % capacity
        entries_printed += 1

    print("Total logs printed: {}".format(entries_printed))

print_logs()
gdb.execute("quit")
