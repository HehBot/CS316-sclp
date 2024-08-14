#ifndef ERROR_H
#define ERROR_H

#ifdef __cplusplus
#include <string>
void sclp_error(size_t line, std::string s);
extern std::string aux_error_msg;
#else
#include <stddef.h>
void sclp_error(size_t line, char const* s, ...);
#endif

#endif // ERROR_H
