# pivec application Makefile

# Makefile Targets
#          all:	compiles the source code
#        clean: removes all .hex, .elf, and .o files in the source code and 
#              	library directories
#      install:	installs the application and documentation
#    uninstall:	uninstalls the application and documentation

# Build Variables +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# Target executable name
TARGET = DnD
# Source files
SOURCES = DnD.c
# Object files
OBJECTS = $(SOURCES:.c=.o)
# Linux usr binaries directory
BINDIR =	/usr/local/bin
# Linux manual directory for the man command
MANDIR =	/usr/local/man/man1
# Build directory
BUILD_DIR = ./build
# Systemd services directory
SYSDDIR = /etc/systemd/system
# Udev rules directory
UDEVDIR = /etc/udev/rules.d
# C compiler command
CC =		gcc
# Build flags for c files
CFLAGS = -g -I/usr/local/include -pedantic -Wall -Wpointer-arith -Wshadow -Wcast-qual -Wcast-align -Wstrict-prototypes -Wredundant-decls -Wno-long-long -DNCURSES_WIDECHAR=1
# Debug flags for c files
DFLAGS = -g -I/usr/local/include -pedantic -Wall -Wpointer-arith -Wshadow -Wcast-qual -Wcast-align -Wstrict-prototypes -Wredundant-decls -Wno-long-long -DNCURSES_WIDECHAR=1
# Linker Flags
LDFLAGS = -lncursesw -lpanel
# Application command line options
OPTIONS =

# Build Targets +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# Build all target files
all:		build $(BUILD_DIR)/$(TARGET)
# Create the build directory
build:
	mkdir -p $(BUILD_DIR)
# Build the app from the .c source
$(BUILD_DIR)/$(TARGET): $(addprefix $(BUILD_DIR)/,$(OBJECTS))
	@echo "Linking $(TARGET)..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
# Compile the .c files to .o files in the build directory
$(BUILD_DIR)/%.o: %.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@
# Build the app with debug symbols
debug: build $(addprefix $(BUILD_DIR)/,$(OBJECTS))
	@echo "Building $(TARGET) with debug symbols..."
	$(CC) $(DFLAGS) -o $(BUILD_DIR)/$(TARGET) $(addprefix $(BUILD_DIR)/,$(OBJECTS)) $(LDFLAGS)
#$(TARGET):		$(TARGET).c
#	$(CC) $(CFLAGS) $(TARGET).c -o $(BUILD_DIR)/$(TARGET) $(LFLAGS)

# Run TARGET with the provided command line options
run:		all
	$(BUILD_DIR)/$(TARGET) $(OPTIONS)
# Install the application
install:	all
	sudo cp $(BUILD_DIR)/$(TARGET) $(BINDIR)
	sudo cp $(TARGET).1 $(MANDIR)
# Uninstall the application
uninstall:
	sudo rm -f $(BINDIR)/$(TARGET)
	sudo rm -f $(MANDIR)/$(TARGET).1
# Install prereqeuisites
prereqs:
	sudo apt update
	sudo apt install libncurses5-dev libncursesw5-dev
# Clean up all generated files
clean:
	rm -rf $(BUILD_DIR)
