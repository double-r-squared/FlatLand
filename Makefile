# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3
INCLUDES = -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib -lSDL2 -lSDL2_ttf -lSDL2_image

# Directories
MAP_DIR = map
NPC_DIR = npc
SHAPES_DIR = $(NPC_DIR)/Shapes
PLAYER_DIR = player
VIEWS_DIR = views
DIALOGUE_DIR = $(VIEWS_DIR)/dialogue-box
WORLDVIEW_DIR = $(VIEWS_DIR)/world-view
PLAYERVIEW_DIR = $(VIEWS_DIR)/player-view
MENU_DIR = $(VIEWS_DIR)/menu

# Output
TARGET = game
MAP_BUILDER = map-builder

# Source files
SOURCES = game.cpp \
          $(MAP_DIR)/map.cpp \
          $(NPC_DIR)/npc.cpp \
          $(SHAPES_DIR)/Rectangle.cpp \
          $(SHAPES_DIR)/Triangle.cpp \
          $(SHAPES_DIR)/Circle.cpp \
          $(SHAPES_DIR)/Line.cpp \
          $(PLAYER_DIR)/player.cpp \
		  $(MENU_DIR)/start-menu.cpp\
          $(DIALOGUE_DIR)/dialogue-box.cpp \
          $(WORLDVIEW_DIR)/world-view.cpp \
          $(PLAYERVIEW_DIR)/player-view.cpp

# Map builder sources
BUILDER_SOURCES = map-builder.cpp \
                  $(NPC_DIR)/npc.cpp \
                  $(SHAPES_DIR)/Rectangle.cpp \
                  $(SHAPES_DIR)/Triangle.cpp \
                  $(SHAPES_DIR)/Circle.cpp \
                  $(SHAPES_DIR)/Line.cpp \
                  $(PLAYER_DIR)/player.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)
BUILDER_OBJECTS = $(BUILDER_SOURCES:.cpp=.o)

# Default target
all: $(TARGET)

# Build map builder
builder: $(MAP_BUILDER)

# Link the game executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

# Link the map builder executable
$(MAP_BUILDER): $(BUILDER_OBJECTS)
	$(CXX) $(BUILDER_OBJECTS) -o $(MAP_BUILDER) $(LDFLAGS)
	@echo "Map builder complete: $(MAP_BUILDER)"

# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(BUILDER_OBJECTS) $(TARGET) $(MAP_BUILDER)
	@echo "Clean complete"

# Rebuild everything
rebuild: clean all

# Run the game
run: $(TARGET)
	./$(TARGET)

# Debug build
debug: CXXFLAGS += -g -DDEBUG
debug: rebuild

# Install dependencies (macOS Homebrew / Linux)
install-deps:
	@echo "For macOS (Homebrew):"
	@echo "  brew install sdl2 sdl2_ttf sdl2_image"
	@echo ""
	@echo "For Ubuntu/Debian:"
	@echo "  sudo apt-get install libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev"
	@echo ""
	@echo "Note: Update INCLUDES and LDFLAGS if SDL2 is in a different location"

# Show variables (for debugging the Makefile)
show:
	@echo "Sources: $(SOURCES)"
	@echo "Objects: $(OBJECTS)"
	@echo "Target: $(TARGET)"

.PHONY: all clean rebuild run debug install-deps show