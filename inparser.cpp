#include "inparser.h"

char InParser::read_ch() {
    index++;
    if (index == BUFF_SIZE) {
        index = 0;
        fread(buff, 1, BUFF_SIZE, fin);
    }
    return buff[index];
}


InParser::InParser(const char *name) {
    isReady = false;
    strcpy(fileName, name);
    index = BUFF_SIZE-1;
    buff = NULL;
}

void InParser::close_parser_and_free() {
    fclose(fin);
    delete []buff;
}

