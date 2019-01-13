CXXFLAGS += -std=c++11 -I googletest/include -I googletest -DTESTING
COMMON = board.o r2d2bot.o
TESTS = board_test.o r2d2bot_test.o

test: $(COMMON) $(TESTS) googletest/src/gtest-all.o googletest/src/gtest_main.o
	$(CXX) -o tests $(COMMON) $(TESTS) googletest/src/gtest-all.o googletest/src/gtest_main.o -pthread
	./tests

board_test.o: board.h

clean:
	rm -f $(COMMON) $(TESTS) test
