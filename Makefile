# Compiler and flags
CC = gcc
CFLAGS = -Wall -g        # Compiler flags
LDFLAGS = -lm            # Linker flags (Math library)

# Output executables
TARGET_SERVER = server
TARGET_CLIENT = client

# Source files
SRCS_SERVER = server.c common.c
SRCS_CLIENT = client.c common.c

# Object files
OBJS_SERVER = $(SRCS_SERVER:.c=.o)
OBJS_CLIENT = $(SRCS_CLIENT:.c=.o)

# Default target - build both server and client
all: $(TARGET_SERVER) $(TARGET_CLIENT)

# Rule to build the server executable
$(TARGET_SERVER): $(OBJS_SERVER)
	$(CC) $(OBJS_SERVER) $(LDFLAGS) -o $(TARGET_SERVER)

# Rule to build the client executable
$(TARGET_CLIENT): $(OBJS_CLIENT)
	$(CC) $(OBJS_CLIENT) $(LDFLAGS) -o $(TARGET_CLIENT)

# Rule to compile .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule to remove object files and executables
clean:
	rm -f $(OBJS_SERVER) $(OBJS_CLIENT) $(TARGET_SERVER) $(TARGET_CLIENT)
