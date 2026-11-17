# SIL CAN Telemetry Aggregator

A Linux-based C++ telemetry aggregator and fault injector for embedded vehicle networks. 

I built this to run Software-in-the-Loop (SIL) verification automotive firmware. It allows us to simulate CAN buses and test supervisory safety logic against hardware faults without needing the physical microcontrollers or risking actual hardware.

## Architecture & Tech Stack

* **POSIX SocketCAN:** Uses `<linux/can.h>` to bind directly to a virtual Linux network interface (`vcan0`) and read raw asynchronous frames.
* **Kernel-Level Filtering:** Implements `setsockopt` hardware-style filters to drop untracked CAN IDs at the OS layer, keeping listener CPU usage negligible.
* **Multithreading & Concurrency:** Separates the high-frequency network listener from the 2Hz UI rendering thread. Both share an $O(1)$ Hash Map protected by a `std::mutex` to prevent memory race conditions.
* **Bitwise Data Parsing:** Mimics standard embedded C++ HAL practices by applying bit-shifting (`<<`) and masking (`&`) to reconstruct multiplexed 16-bit sensor data (e.g., Motor RPM) from isolated 8-bit network frames in real-time.

## Fault Injection (`ghost_car.py`)

To verify the C++ safety limits, the Python script acts as a virtual ECU broadcasting standard duty-cycle telemetry. After 10 seconds, it intentionally injects hardware-level faults onto the bus:
1. **ADC Railing (Short-to-Power):** Pins the battery voltage to a static `255` (8-bit max) to simulate a high-voltage sensor short.
2. **Mechanical Freewheeling:** Spikes the motor to 14,000+ RPM to simulate a drivetrain failure and trigger the overspeed logic.
