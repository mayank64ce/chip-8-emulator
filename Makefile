CXX = g++
CXXFLAGS = -std=c++17 -Wall -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib -lSDL2

TARGET = chip8
SOURCES = main.cpp chip8.cpp
OBJECTS = $(SOURCES:.cpp=.o)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: clean
