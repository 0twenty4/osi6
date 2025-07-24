CXX := g++

CXXFLAGS := -std=c++2a -Wall -pthread

# Исходники
SRCS := main.cpp

# Имя исполняемого файла
TARGET := server

# Сборка
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

# Очистка
clean:
	rm -f $(TARGET)