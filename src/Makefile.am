%.o: %.cpp %.h
    $(CC) -c $< -o $@

# ...or just grab all the files in the source directory:
SOURCE_FILES = $(wildcard *.cpp)
OBJ_FILES = $(SOURCE_FILES:.cpp=.o)

# It is possible to get this from the environment, but not advisable.
LIBS = -lred -lblue

final-thing: $(OBJ_FILES)
    $(CC) $(LIBS) $^ -o $@