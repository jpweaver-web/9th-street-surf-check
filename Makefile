JSON_INC = /opt/homebrew/Cellar/nlohmann-json/3.11.3/include
FLAGS = -std=c++17 -lcurl -I$(JSON_INC)

all: server conditionsTest

server:
	g++ $(FLAGS) server.cpp tideTest.cpp -o server

conditionsTest:
	g++ $(FLAGS) conditionsTest.cpp tideTest.cpp -o conditionsTest

clean:
	rm -f server conditionsTest