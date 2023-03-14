main : ./out/main.o ./out/TaskQueue.o ./out/ThreadPool.o
	g++ -std=c++11 $^ -o $@ -lpthread
./out/TaskQueue.o : ./ThreadPool/TaskQueue.cpp
	g++ -std=c++11 -c $^ -o $@ -lpthread
./out/ThreadPool.o : ./ThreadPool/ThreadPool.cpp
	g++ -std=c++11 -c $^ -o $@ -lpthread
./out/main.o : main.cpp
	g++ -std=c++11 -c $^ -o $@ -lpthread

clean:
	rm ./out/*.o main