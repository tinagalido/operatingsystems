# Compiler and flags
CC = gcc

# Source file
SRCS = ph.c
# Output
TARGET = a2.out
THREADS1 = 1
THREADS2 = 2
THREADS4 = 4

# Default target
all: $(TARGET)

# Rule to build the executable
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

# Rule to run the program
run1: $(TARGET)
	./$(TARGET) $(THREADS1)

run2: $(TARGET)
	./$(TARGET) $(THREADS2)

run4: $(TARGET)
	./$(TARGET) $(THREADS4)

# Rule to clean generated files
clean:
	rm -f $(TARGET)
