# the compiler: gcc for C program, define as g++ for C++
  CC = g++

  CFLAGS  = -g -Wall -pthread

  # The build target
  TARGET = thr_test

  all: $(TARGET)

  $(TARGET): $(TARGET).cpp
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).cpp

  clean:
	$(RM) $(TARGET)
