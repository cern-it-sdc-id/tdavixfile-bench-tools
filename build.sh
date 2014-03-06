g++ -Wall -Wshadow -std=gnu++11 main.cpp -o test `root-config --libs --cflags` -lTreePlayer -g

g++ -Wall -Wshadow -std=gnu++11 inflateTree_main.cpp -o testInflate `root-config --libs --cflags` -lTreePlayer -g


g++ -Wall -Wshadow -std=gnu++11 h2fastnew_main.cpp -o testEvent `root-config --libs --cflags` -lTreePlayer -g 

