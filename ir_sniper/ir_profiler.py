import serial
import serial.tools.list_ports
import time
import datetime

def find_m5_port():
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        if "USB" in p.description or "UART" in p.description:
            return p.device
    return None

def log_success(message):
    timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    with open("success_codes.txt", "a") as f:
        f.write(f"[{timestamp}] {message}\n")
    print(f"\n[SAVED] {message}")

def blast_range(ser, start, end, delay=0.1):
    """Blasts a range of codes with v4 speed."""
    print(f"\n>>> BLASTING RANGE: {start} to {min(end, 277)}...")
    for i in range(start, min(end + 1, 278)):
        print(f"  Firing #{i}...", end='\r')
        ser.write(f"{i}\n".encode())
        
        # Fast wait for OK
        ser.readline() 
        time.sleep(delay) 
    print(f"\n>>> Range Completed.")

def main():
    print("==================================")
    print("   IR SNIPER - v4 STABLE RECON    ")
    print("==================================")
    
    port_name = find_m5_port()
    if not port_name:
        port_name = input("Enter COM port: ")
    
    try:
        ser = serial.Serial(port_name, 115200, timeout=0.5)
        time.sleep(2) 
        ser.read_all()
        
        total_codes = 278
        base_50 = 0
        
        while base_50 < total_codes:
            # PHASE 1
            blast_range(ser, base_50, base_50 + 49, delay=0.1)
            ans = input(f"\nDid the TV react to block {base_50}-{min(base_50+49, 277)}? (w=YES | [Enter]=NEXT): ").strip().lower()
            
            if ans == 'w':
                log_success(f"SUCCESS: TV reacted to 50-Block {base_50}-{min(base_50+49, 277)}")
                
                base_10 = base_50
                while base_10 < base_50 + 50 and base_10 < total_codes:
                    # PHASE 2
                    blast_range(ser, base_10, base_10 + 9, delay=0.1)
                    ans_10 = input(f"  Did it react to 10-Block {base_10}-{min(base_10+9, 277)}? (w=YES | [Enter]=NEXT): ").strip().lower()
                    
                    if ans_10 == 'w':
                        log_success(f"SUCCESS: TV reacted to 10-Block {base_10}-{min(base_10+9, 277)}")
                        
                        # PHASE 3
                        for i in range(base_10, min(base_10 + 10, 278)):
                            print(f"    [TESTING SINGLE] ID #{i}...")
                            ser.write(f"{i}\n".encode())
                            ser.readline() 
                            time.sleep(0.5) 
                            ans_1 = input(f"    Did ID #{i} work? (w=SAVE | [Enter]=NEXT): ").strip().lower()
                            if ans_1 == 'w':
                                log_success(f"FINAL CONFIRMATION: ID #{i} is a working code!")
                    base_10 += 10
            base_50 += 50

    except KeyboardInterrupt:
        print("\nStopped.")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        if 'ser' in locals():
            ser.close()

if __name__ == "__main__":
    main()
