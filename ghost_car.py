import can
import time
import random

bus = can.interface.Bus(channel='vcan0', interface='socketcan')

print("Starting the IEM Fault Injector... (Press Ctrl+C to stop)")
print("Sending normal telemetry. Catastrophic faults will inject in 10 seconds.")

# Record the exact time the script started
start_time = time.time()

try:
    while True:
        elapsed = time.time() - start_time
        
        # regular state:
        if elapsed < 10:
            batt_v = random.randint(100, 140)
            rpm = random.randint(4000, 5000)
            
        # faulty state
        else:
            batt_v = 255 # overvoltage
            rpm = random.randint(14000, 15000) # Mechanical Over-rev
        
        # Pack 8-bit Battery Voltage 
        battery_data = [batt_v, 0, 0, 0, 0, 0, 0, 0]
        msg_batt = can.Message(arbitration_id=0x10, data=battery_data, is_extended_id=False)
    
        # 2. Pack 16-bit Motor RPM 
        rpm_high_byte = (rpm >> 8) & 0xFF
        rpm_low_byte = rpm & 0xFF
        motor_data = [rpm_high_byte, rpm_low_byte, 0, 0, 0, 0, 0, 0]
        msg_motor = can.Message(arbitration_id=0x20, data=motor_data, is_extended_id=False)

        bus.send(msg_batt)
        bus.send(msg_motor)
        time.sleep(0.1)

except KeyboardInterrupt:
    print("\nFault Injector turned off.")