# LiteX Project Makefile - Root Level Build System
# This Makefile builds the software from the project root directory

# Define BUILD_DIR for LiteX compatibility
BUILD_DIR ?= $(BUILD_DIR_SIPEED_VEX)

# Check if BUILD_DIR is properly set
ifeq ($(BUILD_DIR),)
$(error BUILD_DIR is not set. Please run this Makefile after building the LiteX SoC with the appropriate environment variables set, or set BUILD_DIR manually)
endif

# Check if required files exist
ifeq (,$(wildcard $(BUILD_DIR)/software/include/generated/variables.mak))
$(warning Warning: $(BUILD_DIR)/software/include/generated/variables.mak not found. You may need to build the LiteX SoC first.)
endif

# Directory structure
SOFTWARE_DIR = software
BUILD_OBJ_DIR = build
OUTPUT_DIR = .

# Include LiteX makefiles (paths adjusted for project root)
include $(BUILD_DIR)/software/include/generated/variables.mak
include $(SOC_DIRECTORY)/software/common.mak

# Source files from software directory
SOURCES = $(wildcard $(SOFTWARE_DIR)/*.c $(SOFTWARE_DIR)/*/*.c $(SOFTWARE_DIR)/*/*/*.c)

# Object files in build directory, preserving directory structure
OBJECTS = $(BUILD_OBJ_DIR)/crt0.o
OBJECTS += $(patsubst $(SOFTWARE_DIR)/%.c,$(BUILD_OBJ_DIR)/%.o,$(SOURCES))

# Default target
all: benchmark.bin

# Create build directory structure
$(BUILD_OBJ_DIR):
	@echo "Creating build directory structure..."
	@mkdir -p $(BUILD_OBJ_DIR)
	@mkdir -p $(BUILD_OBJ_DIR)/drivers

# Binary generation
benchmark.bin: benchmark.elf
	@echo "Generating binary: $@"
	$(OBJCOPY) -O binary $< $@
ifneq ($(OS),Windows_NT)
	chmod -x $@
endif

# ELF linking
benchmark.elf: $(OBJECTS)
	@echo "Linking ELF: $@"
	$(CC) $(LDFLAGS) -T $(SOFTWARE_DIR)/linker.ld -N -o $@ \
		$(OBJECTS) \
		$(PACKAGES:%=-L$(BUILD_DIR)/software/%) \
		-Wl,--whole-archive \
		-Wl,--gc-sections \
		-Wl,-Map,$@.map \
		$(LIBS:lib%=-l%)
ifneq ($(OS),Windows_NT)
	chmod -x $@
endif

# Include dependency files
-include $(OBJECTS:.o=.d)

# Special compiler flags for specific files
$(BUILD_OBJ_DIR)/donut.o: CFLAGS += -w

# VPATH for finding source files (adjusted for project root)
VPATH = $(BIOS_DIRECTORY):$(BIOS_DIRECTORY)/cmds:$(CPU_DIRECTORY):$(SOFTWARE_DIR):$(SOFTWARE_DIR)/drivers
vpath %.a $(PACKAGES:%=../%)

# Special rule for crt0.o (comes from CPU_DIRECTORY via VPATH)
$(BUILD_OBJ_DIR)/crt0.o: crt0.S | $(BUILD_OBJ_DIR)
	@echo "Assembling crt0.S from CPU directory: $<"
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -o $@ $< -MD -MP -MF $(@:.o=.d)

# Compilation rules with build directory output
$(BUILD_OBJ_DIR)/%.o: $(SOFTWARE_DIR)/%.cpp | $(BUILD_OBJ_DIR)
	@echo "Compiling C++ file: $<"
	@mkdir -p $(dir $@)
	$(CC) -c $(CXXFLAGS) -o $@ $< -MD -MP -MF $(@:.o=.d)

$(BUILD_OBJ_DIR)/%.o: $(SOFTWARE_DIR)/%.c | $(BUILD_OBJ_DIR)
	@echo "Compiling C file: $<"
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -o $@ $< -MD -MP -MF $(@:.o=.d)

# Generic rule for other assembly files that might come via VPATH
$(BUILD_OBJ_DIR)/%.o: %.S | $(BUILD_OBJ_DIR)
	@echo "Assembling file: $<"
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -o $@ $< -MD -MP -MF $(@:.o=.d)

# Clean target
clean:
	@echo "Cleaning build artifacts..."
	$(RM) -rf $(BUILD_OBJ_DIR)
	$(RM) benchmark.elf benchmark.elf.map benchmark.bin
	$(RM) *.d

# Clean software directory of old build artifacts
clean-software:
	@echo "Cleaning old software directory artifacts..."
	$(RM) $(SOFTWARE_DIR)/*.o $(SOFTWARE_DIR)/*.d
	$(RM) $(SOFTWARE_DIR)/drivers/*.o $(SOFTWARE_DIR)/drivers/*.d
	$(RM) $(SOFTWARE_DIR)/demo.elf $(SOFTWARE_DIR)/demo.elf.map $(SOFTWARE_DIR)/demo.bin

# Full clean
clean-all: clean clean-software

# Debug targets
print-LDFLAGS:
	@echo "LDFLAGS: $(LDFLAGS)"

print-LIBS:
	@echo "LIBS: $(LIBS)"

print-objects:
	@echo "Object files:"
	@for obj in $(OBJECTS); do echo "  $$obj"; done

print-sources:
	@echo "Source files:"
	@for src in $(SOURCES); do echo "  $$src"; done

print-paths:
	@echo "Build configuration:"
	@echo "  SOFTWARE_DIR: $(SOFTWARE_DIR)"
	@echo "  BUILD_OBJ_DIR: $(BUILD_OBJ_DIR)"
	@echo "  BUILD_DIR: $(BUILD_DIR)"
	@echo "  SOC_DIRECTORY: $(SOC_DIRECTORY)"

print-env:
	@echo "Build environment:"
	@echo "  BUILD_DIR: $(BUILD_DIR)"
	@echo "  BUILD_DIR_SIPEED_VEX: $(BUILD_DIR_SIPEED_VEX)"
	@echo "  SOC_DIRECTORY: $(SOC_DIRECTORY)"
	@echo "  BUILD_OBJ_DIR: $(BUILD_OBJ_DIR)"
	@echo "  CPU_DIRECTORY: $(CPU_DIRECTORY)"
	@echo "  VPATH: $(VPATH)"

# Help target
help:
	@echo "LiteX Project Build System (Project Root)"
	@echo "Available targets:"
	@echo "  all          - Build benchmark.bin (default)"
	@echo "  benchmark.bin - Build the final binary"
	@echo "  benchmark.elf - Build the ELF file"
	@echo "  clean        - Clean build artifacts"
	@echo "  clean-software - Clean old artifacts from software directory"
	@echo "  clean-all    - Full clean of all artifacts"
	@echo "  print-*      - Print various build information"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Usage:"
	@echo "  1. First build the SoC: python3 sipeed_tang_nano_9k.py --build"
	@echo "  2. Then build software: make"
	@echo ""
	@echo "Or set BUILD_DIR manually: make BUILD_DIR=/path/to/build/dir"

.PHONY: all clean clean-software clean-all print-LDFLAGS print-LIBS print-objects print-sources print-paths print-env help 
