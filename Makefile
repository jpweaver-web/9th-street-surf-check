JSON_INC = /opt/homebrew/Cellar/nlohmann-json/3.11.3/include
CXXFLAGS = -std=c++17 -I$(JSON_INC)
LIBS = -lcurl

all: server conditionsTest

server:
	g++ $(CXXFLAGS) main.cpp tideTest.cpp -o server $(LIBS)

conditionsTest:
	g++ $(CXXFLAGS) conditionsTest.cpp tideTest.cpp -o conditionsTest $(LIBS)

clean:
	rm -f server conditionsTest

.PHONY: all clean