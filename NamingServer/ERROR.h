#ifndef ERRORS_H
#define ERRORS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *error_message(int error_code)
{
    switch (error_code)
    {
        case 0: return "SUCCESS";
        case 1: return "INVALID COMMAND";
        case 2: return "PATH NOT FOUND";
        case 3: return "PERMISSION DENIED";
        case 4: return "FILE IN USE";
        case 5: return "ALREADY EXISTS";
        case 6: return "DELETION FAILED";
        case 7: return "CREATION FAILED";
        case 8: return "READ FAILED";
        case 9: return "WRITE FAILED";
        case 10:return "STREAM FAILED";
        case 11:return "COPY FAILED";
        case 12:return "GET_INFO FAILED";
        case 13:return "TIMEOUT";
        case 14:return "BACKUP FAILED";
        case 15:return "CONNECTION FAILED";
    }
    
}

#endif