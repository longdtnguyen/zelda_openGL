# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

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
CMAKE_SOURCE_DIR = /u9/l34nguye/cs488/shared/glfw-3.1.1

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /u9/l34nguye/cs488/shared/glfw-3.1.1/build

# Include any dependencies generated for this target.
include tests/CMakeFiles/cursoranim.dir/depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/cursoranim.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/cursoranim.dir/flags.make

tests/CMakeFiles/cursoranim.dir/cursoranim.c.o: tests/CMakeFiles/cursoranim.dir/flags.make
tests/CMakeFiles/cursoranim.dir/cursoranim.c.o: ../tests/cursoranim.c
	$(CMAKE_COMMAND) -E cmake_progress_report /u9/l34nguye/cs488/shared/glfw-3.1.1/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object tests/CMakeFiles/cursoranim.dir/cursoranim.c.o"
	cd /u9/l34nguye/cs488/shared/glfw-3.1.1/build/tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/cursoranim.dir/cursoranim.c.o   -c /u9/l34nguye/cs488/shared/glfw-3.1.1/tests/cursoranim.c

tests/CMakeFiles/cursoranim.dir/cursoranim.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/cursoranim.dir/cursoranim.c.i"
	cd /u9/l34nguye/cs488/shared/glfw-3.1.1/build/tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /u9/l34nguye/cs488/shared/glfw-3.1.1/tests/cursoranim.c > CMakeFiles/cursoranim.dir/cursoranim.c.i

tests/CMakeFiles/cursoranim.dir/cursoranim.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/cursoranim.dir/cursoranim.c.s"
	cd /u9/l34nguye/cs488/shared/glfw-3.1.1/build/tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /u9/l34nguye/cs488/shared/glfw-3.1.1/tests/cursoranim.c -o CMakeFiles/cursoranim.dir/cursoranim.c.s

tests/CMakeFiles/cursoranim.dir/cursoranim.c.o.requires:
.PHONY : tests/CMakeFiles/cursoranim.dir/cursoranim.c.o.requires

tests/CMakeFiles/cursoranim.dir/cursoranim.c.o.provides: tests/CMakeFiles/cursoranim.dir/cursoranim.c.o.requires
	$(MAKE) -f tests/CMakeFiles/cursoranim.dir/build.make tests/CMakeFiles/cursoranim.dir/cursoranim.c.o.provides.build
.PHONY : tests/CMakeFiles/cursoranim.dir/cursoranim.c.o.provides

tests/CMakeFiles/cursoranim.dir/cursoranim.c.o.provides.build: tests/CMakeFiles/cursoranim.dir/cursoranim.c.o

# Object files for target cursoranim
cursoranim_OBJECTS = \
"CMakeFiles/cursoranim.dir/cursoranim.c.o"

# External object files for target cursoranim
cursoranim_EXTERNAL_OBJECTS =

tests/cursoranim: tests/CMakeFiles/cursoranim.dir/cursoranim.c.o
tests/cursoranim: tests/CMakeFiles/cursoranim.dir/build.make
tests/cursoranim: src/libglfw3.a
tests/cursoranim: /usr/lib/x86_64-linux-gnu/libX11.so
tests/cursoranim: /usr/lib/x86_64-linux-gnu/libXrandr.so
tests/cursoranim: /usr/lib/x86_64-linux-gnu/libXinerama.so
tests/cursoranim: /usr/lib/x86_64-linux-gnu/libXi.so
tests/cursoranim: /usr/lib/x86_64-linux-gnu/libXxf86vm.so
tests/cursoranim: /usr/lib/x86_64-linux-gnu/librt.so
tests/cursoranim: /usr/lib/x86_64-linux-gnu/libm.so
tests/cursoranim: /usr/lib/x86_64-linux-gnu/libXcursor.so
tests/cursoranim: /usr/lib/x86_64-linux-gnu/libGL.so
tests/cursoranim: tests/CMakeFiles/cursoranim.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable cursoranim"
	cd /u9/l34nguye/cs488/shared/glfw-3.1.1/build/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/cursoranim.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/cursoranim.dir/build: tests/cursoranim
.PHONY : tests/CMakeFiles/cursoranim.dir/build

tests/CMakeFiles/cursoranim.dir/requires: tests/CMakeFiles/cursoranim.dir/cursoranim.c.o.requires
.PHONY : tests/CMakeFiles/cursoranim.dir/requires

tests/CMakeFiles/cursoranim.dir/clean:
	cd /u9/l34nguye/cs488/shared/glfw-3.1.1/build/tests && $(CMAKE_COMMAND) -P CMakeFiles/cursoranim.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/cursoranim.dir/clean

tests/CMakeFiles/cursoranim.dir/depend:
	cd /u9/l34nguye/cs488/shared/glfw-3.1.1/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /u9/l34nguye/cs488/shared/glfw-3.1.1 /u9/l34nguye/cs488/shared/glfw-3.1.1/tests /u9/l34nguye/cs488/shared/glfw-3.1.1/build /u9/l34nguye/cs488/shared/glfw-3.1.1/build/tests /u9/l34nguye/cs488/shared/glfw-3.1.1/build/tests/CMakeFiles/cursoranim.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/cursoranim.dir/depend

