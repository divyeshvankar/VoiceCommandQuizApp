# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall

# Project files
SRC = src/server.cpp
TARGET = server

# Boost libraries
LIBS = -lboost_system -lpthread

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LIBS)

clean:
	rm -f $(TARGET)
