JSON_INC = /opt/homebrew/Cellar/nlohmann-json/3.11.3/include
CXXFLAGS = -std=c++17 -I$(JSON_INC)
LIBS = -lcurl

all: server

server:
	g++ $(CXXFLAGS) main.cpp tideTest.cpp -o server $(LIBS)

clean:
	rm -f server

.PHONY: all clean