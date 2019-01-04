CXXFLAGS += -std=c++11 -I googletest/include -I googletest

test: board_test.o googletest/src/gtest-all.o googletest/src/gtest_main.o
	$(CXX) -o board_test board_test.o googletest/src/gtest-all.o googletest/src/gtest_main.o -pthread
	./board_test

board_test.o: board.h

