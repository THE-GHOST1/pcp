# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/prajwal/CLionProjects/pcp/src/pmproxy2

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/prajwal/CLionProjects/pcp/src/pmproxy2/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/pmproxy2-temp.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/pmproxy2-temp.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/pmproxy2-temp.dir/flags.make

CMakeFiles/pmproxy2-temp.dir/temp2.c.o: CMakeFiles/pmproxy2-temp.dir/flags.make
CMakeFiles/pmproxy2-temp.dir/temp2.c.o: ../temp2.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/prajwal/CLionProjects/pcp/src/pmproxy2/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/pmproxy2-temp.dir/temp2.c.o"
	/usr/bin/gcc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/pmproxy2-temp.dir/temp2.c.o   -c /home/prajwal/CLionProjects/pcp/src/pmproxy2/temp2.c

CMakeFiles/pmproxy2-temp.dir/temp2.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/pmproxy2-temp.dir/temp2.c.i"
	/usr/bin/gcc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/prajwal/CLionProjects/pcp/src/pmproxy2/temp2.c > CMakeFiles/pmproxy2-temp.dir/temp2.c.i

CMakeFiles/pmproxy2-temp.dir/temp2.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/pmproxy2-temp.dir/temp2.c.s"
	/usr/bin/gcc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/prajwal/CLionProjects/pcp/src/pmproxy2/temp2.c -o CMakeFiles/pmproxy2-temp.dir/temp2.c.s

CMakeFiles/pmproxy2-temp.dir/temp2.c.o.requires:

.PHONY : CMakeFiles/pmproxy2-temp.dir/temp2.c.o.requires

CMakeFiles/pmproxy2-temp.dir/temp2.c.o.provides: CMakeFiles/pmproxy2-temp.dir/temp2.c.o.requires
	$(MAKE) -f CMakeFiles/pmproxy2-temp.dir/build.make CMakeFiles/pmproxy2-temp.dir/temp2.c.o.provides.build
.PHONY : CMakeFiles/pmproxy2-temp.dir/temp2.c.o.provides

CMakeFiles/pmproxy2-temp.dir/temp2.c.o.provides.build: CMakeFiles/pmproxy2-temp.dir/temp2.c.o


# Object files for target pmproxy2-temp
pmproxy2__temp_OBJECTS = \
"CMakeFiles/pmproxy2-temp.dir/temp2.c.o"

# External object files for target pmproxy2-temp
pmproxy2__temp_EXTERNAL_OBJECTS =

pmproxy2-temp: CMakeFiles/pmproxy2-temp.dir/temp2.c.o
pmproxy2-temp: CMakeFiles/pmproxy2-temp.dir/build.make
pmproxy2-temp: CMakeFiles/pmproxy2-temp.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/prajwal/CLionProjects/pcp/src/pmproxy2/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable pmproxy2-temp"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/pmproxy2-temp.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/pmproxy2-temp.dir/build: pmproxy2-temp

.PHONY : CMakeFiles/pmproxy2-temp.dir/build

CMakeFiles/pmproxy2-temp.dir/requires: CMakeFiles/pmproxy2-temp.dir/temp2.c.o.requires

.PHONY : CMakeFiles/pmproxy2-temp.dir/requires

CMakeFiles/pmproxy2-temp.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/pmproxy2-temp.dir/cmake_clean.cmake
.PHONY : CMakeFiles/pmproxy2-temp.dir/clean

CMakeFiles/pmproxy2-temp.dir/depend:
	cd /home/prajwal/CLionProjects/pcp/src/pmproxy2/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/prajwal/CLionProjects/pcp/src/pmproxy2 /home/prajwal/CLionProjects/pcp/src/pmproxy2 /home/prajwal/CLionProjects/pcp/src/pmproxy2/cmake-build-debug /home/prajwal/CLionProjects/pcp/src/pmproxy2/cmake-build-debug /home/prajwal/CLionProjects/pcp/src/pmproxy2/cmake-build-debug/CMakeFiles/pmproxy2-temp.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/pmproxy2-temp.dir/depend

