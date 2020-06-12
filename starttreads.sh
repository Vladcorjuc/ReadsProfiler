#!/bin/bash
g++ Server.c -o Server -L/usr/include/mysql -lmysqlclient -I/usr/include/mysql -lpthread 
g++ Client.c -o Client `pkg-config --libs --cflags -gtkmm-2.4`
valgrind ./Server -ggdb

