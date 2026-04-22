# BOOST_INC: path to Boost headers.
# Leave empty on Linux (apt-installed Boost is on the system path).
# On macOS: make BOOST_INC=$(brew --prefix boost)/include
BOOST_INC ?=

CXXFLAGS := -O3 -march=native -std=c++11
ifneq ($(BOOST_INC),)
	CXXFLAGS += -I$(BOOST_INC)
endif

SRCS := Transition.cpp Game.cpp Graph.cpp main.cpp

all: $(SRCS)
	g++ $(CXXFLAGS) $(SRCS) -w -o game

clean:
	rm -f game
