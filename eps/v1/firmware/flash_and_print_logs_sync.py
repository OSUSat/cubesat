import struct
import time
import gdb

def run_sync_flash():
    # Disable pagination
    gdb.execute("set pagination off")
    
    # Connect to target
    try:
        gdb.execute("target remote localhost:3333")
    except Exception as e:
        print("Failed to connect to GDB: {}".format(e))
        return

    # Flash target
    print("Flashing target...")
    try:
        gdb.execute("load")
    except Exception as e:
        print("Failed to load: {}".format(e))
        return
    
    # Reset target
    print("Resetting target...")
    try:
        gdb.execute("monitor reset halt")
    except Exception as e:
        print("Failed to reset: {}".format(e))
        return
    
    # Set breakpoint on telemetry_update
    print("Setting breakpoint on telemetry_update...")
    try:
        gdb.execute("break telemetry_update")
    except Exception as e:
        print("Failed to set breakpoint: {}".format(e))
        return
    
    # Run until first hit (initialization / first update)
    print("Continuing to first telemetry update...")
    try:
        gdb.execute("continue")
    except Exception as e:
        print("Stopped at: {}".format(e))
    
    # Let it run 4 more times to accumulate logs over 4 update periods (~2.4 seconds)
    for i in range(4):
        print("Continuing to telemetry update {}...".format(i+2))
        try:
            gdb.execute("continue")
        except Exception as e:
            print("Stopped at: {}".format(e))
            break
        
    print("Target stopped at telemetry update. Reading log ring buffer...")
    
    # Read log_ring_buffer
    try:
        rb_addr = int(gdb.parse_and_eval("&log_ring_buffer"))
        inferior = gdb.selected_inferior()
        struct_bytes = bytes(inferior.read_memory(rb_addr, 17))
        buffer_ptr, capacity, head, tail, overwrite = struct.unpack("<IIIIB", struct_bytes)
        print("Ring buffer status: head={}, tail={}, capacity=0x{:x}".format(head, tail, capacity))
        
        if head == tail:
            print("No log entries in ring buffer.")
            return
            
        buf = bytes(inferior.read_memory(buffer_ptr, capacity))
        
        def read_bytes(offset, length):
            res = bytearray()
            for i in range(length):
                res.append(buf[(offset + i) % capacity])
            return bytes(res)
            
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
    except Exception as e:
        print("Error reading logs: {}".format(e))

run_sync_flash()
gdb.execute("quit")
