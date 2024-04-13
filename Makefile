# Makefile

CC = g++
CFLAGS = -Iinclude -I/home/pi/websocketpp -I/usr/include/jsoncpp/ -lwiringPi -lboost_system -lpthread -ljsoncpp
SRCDIR = src
OBJDIR = build
TARGET = $(OBJDIR)/projectName

SRC_FILES = $(wildcard $(SRCDIR)/*.cpp)
OBJ_FILES = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRC_FILES))

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(CC) $^ -o $@ $(CFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f $(OBJDIR)/*.o $(TARGET)

.PHONY: clean
