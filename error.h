#ifndef ERROR_H
#define ERROR_H


void parse_error(const char* fmt, ...);
void parse_error2(const char* fmt, ...);
void memory_error(int error_num, size_t mem_size);


#endif
