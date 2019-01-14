CXXFLAGS += -std=c++11 -I googletest/include -I googletest -DTESTING
COMMON = out/board.o out/r2d2bot.o
TESTS = out/board_test.o out/r2d2bot_test.o

.PHONY: directories

all: directories out/tests

directories: out

out:
	mkdir -p out
	mkdir -p out/googletest/src

out/%.o: %.cc
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $< -o $@

out/tests: $(COMMON) $(TESTS) out/googletest/src/gtest-all.o out/googletest/src/gtest_main.o
	$(CXX) -o out/tests $(COMMON) $(TESTS) out/googletest/src/gtest-all.o out/googletest/src/gtest_main.o -pthread
	./out/tests

out/board_test.o: board.h

clean:
	rm -rf out
