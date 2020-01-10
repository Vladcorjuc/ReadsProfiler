#!/bin/bash
g++ Server.c -o Server -L/usr/include/mysql -lmysqlclient -I/usr/include/mysql
g++ Client.c -o Client
./Server

