FLAGS = -O3 -Wall -msse2 -m64  -flax-vector-conversions -lpthread -g
./a.out: MemoryDict.o MemoryManager.o test.o hash.o function.o ListHandler.o CacheFrame.o
	g++ $(FLAGS) MemoryDict.o MemoryManager.o test.o hash.o function.o ListHandler.o CacheFrame.o
#TM.o:	TM.cpp TM.h
#	g++ $(FLAGS) -c TM.cpp -o TM.o
MemoryDict.o: MemoryDict.cpp  MemoryDict.h
	g++ $(FLAGS) -c MemoryDict.cpp -o MemoryDict.o
MemoryManager.o: MemoryManager.cpp  MemoryManager.h
	g++ $(FLAGS) -c MemoryManager.cpp -o MemoryManager.o
ListHandler.o: ListHandler.cpp ListHandler.h
	g++ $(FLAGS) -c ListHandler.cpp -o ListHandler.o
CacheFrame.o: CacheFrame.cpp CacheFrame.h
	g++ $(FLAGS) -c CacheFrame.cpp -o CacheFrame.o
hash.o: hash.cpp hash.h
	g++ $(FLAGS) -c hash.cpp -o hash.o
function.o: function.cpp function.h
	g++ $(FLAGS) -c function.cpp -o function.o
test.o: testMM_threads.cpp
	g++ $(FLAGS) -c testMM_threads.cpp -o test.o

clean:
	rm *.o a.out
ar:
	tar cvzf "./bak/`date +"%Y%m%d_%H%M%S"`.tar.gz" *.cpp *.h Makefile
