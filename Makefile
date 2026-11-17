# Compiler settings
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -pthread

# Target executable name
TARGET = aggregator

# Build rules
all: $(TARGET)

$(TARGET): aggregator.cpp
	$(CXX) $(CXXFLAGS) aggregator.cpp -o $(TARGET)

clean:
	rm -f $(TARGET)