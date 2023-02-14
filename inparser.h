#pragma once

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "constants.h"

class InParser {
private:
    FILE *fin;
    char *buff;
    char fileName[FILE_NAME_SIZE];
    int index;
    bool isReady;

    char read_ch();

public:
    InParser(const char *file);
    InParser& operator >> (int &n) {
        if (!isReady) {
            isReady = true;
            fin = fopen(fileName, "r");
            buff = new char[BUFF_SIZE];
        }
        char c = '0';
        while (!isdigit(c = read_ch()));
        n = c - '0';
        while (isdigit(c = read_ch())) {
            n = 10 * n + c - '0';
        }
        return *this;
    }
    void close_parser_and_free();
    //InParser& operator >> (long long &n);

};