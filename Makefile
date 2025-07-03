CXX = g++
CXXFLAGS = -std=c++20 -Wall -O2 -Iinclude -Iproto_output
LDFLAGS = -lprotobuf

# Исходные файлы с путями
SRCS = src/rsb_parser.cpp src/byteswap.cpp proto_output/rsb_event.pb.cc
OBJS = $(SRCS:.cpp=.o)
OBJS := $(OBJS:.cc=.o)

# Заголовочные файлы (для зависимостей)
DEPS = include/byteswap.h proto_output/rsb_event.pb.h

# Имя исполняемого файла
TARGET = rsb_parser

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

src/%.o: src/%.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

proto_output/%.o: proto_output/%.cc $(DEPS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f src/*.o proto_output/*.o $(TARGET)

.PHONY: all clean 