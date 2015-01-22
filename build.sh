g++ -Wall -Wshadow -std=gnu++11 main.cpp -o test `root-config --libs --cflags` -lTreePlayer -g

g++ -Wall -Wshadow -std=gnu++11 inflateTree_main.cpp -o testInflate `root-config --libs --cflags` -lTreePlayer -g

g++ -Wall -Wshadow -std=gnu++11 h2fastnew_main.cpp -o testEvent `root-config --libs --cflags` -lTreePlayer -g 

g++ -Wall -Wshadow -std=gnu++11 h2fastnew_loop.cpp -o testEvent_loop `root-config --libs --cflags` -lTreePlayer -g 

g++ -Wall -Wshadow -std=gnu++11 readDPMWebDav_main.cpp -o testHammercloud `root-config --libs --cflags` -lTreePlayer -g

g++ -o testReadOK read_OK_main.cpp -g -O2 `root-config --cflags --libs` -lTreePlayer

