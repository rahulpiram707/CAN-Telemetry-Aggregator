
 
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <unordered_map>
#include <cstdint>
#include <thread>
#include <mutex>
#include <chrono>
#include <cstdlib> 

std::unordered_map<uint32_t, int> car_data;
std::mutex data_mutex;


// Second thread: display dashboard

void draw_dashboard() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        system("clear"); 
        
        data_mutex.lock();
        //take the mutex
        std::cout << "=================================\n";
        std::cout << "      IEM GHOST CAR DASHBOARD    \n";
        std::cout << "=================================\n";
        
        // Check if Battery is under under 150V)
        if (car_data.count(0x10)) {
            if (car_data[0x10] >= 150) {
                std::cout << "[0x10] Battery Voltage : " << car_data[0x10] << " V   [OVERVOLTAGE FAULT]\n";
            } else {
                std::cout << "[0x10] Battery Voltage : " << car_data[0x10] << " V\n";
            }
        } else {
            std::cout << "[0x10] Battery Voltage : WAITING...\n";
        }
        
        // Check if Motor under 6000 RPM
        if (car_data.count(0x20)) {
            if (car_data[0x20] >= 6000) {
                std::cout << "[0x20] Motor RPM       : " << car_data[0x20] << " RPM [OVERSPEED FAULT]\n";
            } else {
                std::cout << "[0x20] Motor RPM       : " << car_data[0x20] << " RPM\n";
            }
        } else {
            std::cout << "[0x20] Motor RPM       : WAITING...\n";
        }
        
        std::cout << "=================================\n";
        std::cout << "(Press Ctrl+C to exit)\n";
        //give back the mutex
        data_mutex.unlock();
    }
}


// First thread: receiver
int main() {
    int socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    
    // set up kernal filters to only take in appropriate messages
    struct can_filter rfilter[2];
    rfilter[0].can_id   = 0x10;
    rfilter[0].can_mask = CAN_SFF_MASK; // battery
    rfilter[1].can_id   = 0x20;
    rfilter[1].can_mask = CAN_SFF_MASK; // motor
    setsockopt(socket_fd, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));

    // bind to vcan
    struct ifreq ifr;
    strcpy(ifr.ifr_name, "vcan0");
    ioctl(socket_fd, SIOCGIFINDEX, &ifr);
    
    struct sockaddr_can addr;
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr));

    std::thread display(draw_dashboard);

    // receiver loop
    while (true) {
        struct can_frame frame;
        int bytes_read = read(socket_fd, &frame, sizeof(struct can_frame));

        if (bytes_read > 0) {
            //take the mutex
            data_mutex.lock();
            
            if (frame.can_id == 0x10) {
                // if battery id is just 1 byte receive data normally
                car_data[0x10] = (int)frame.data[0];
            } 
            else if (frame.can_id == 0x20) {
                // if 16 bits then combine 2 bytes
                // cast to a 16 bit number
                uint16_t raw_rpm = ((uint16_t)frame.data[0] << 8) | (uint16_t)frame.data[1];
                car_data[0x20] = (int)raw_rpm;
            }
            //hand back mutex
            data_mutex.unlock();
        }
    }

    close(socket_fd);
    return 0;
}
