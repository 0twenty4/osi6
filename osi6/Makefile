CXX := g++

CXXFLAGS := -std=c++2a -Wall -pthread

# ���������
SRCS := main.cpp

# ��� ������������ �����
TARGET := server

# ������
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

# �������
clean:
	rm -f $(TARGET)