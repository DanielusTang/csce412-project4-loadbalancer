# AggieForge Traffic Cloud - CSCE 412 Project 4 load balancer simulation
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
SRCS     = $(wildcard *.cpp)
OBJS     = $(SRCS:.cpp=.o)
TARGET   = loadbalancer

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

docs:
	doxygen Doxyfile

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all docs clean
