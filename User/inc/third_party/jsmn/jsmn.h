/*
 * Minimalistic JSON parser jsmn
 * Source: https://github.com/zserge/jsmn (MIT License)
 */
#ifndef __JSMN_H_
#define __JSMN_H_

#include <stddef.h>

typedef enum {
  JSMN_UNDEFINED = 0,
  JSMN_OBJECT = 1,
  JSMN_ARRAY = 2,
  JSMN_STRING = 3,
  JSMN_PRIMITIVE = 4
} jsmntype_t;

typedef enum {
  JSMN_ERROR_NOMEM = -1,
  JSMN_ERROR_INVAL = -2,
  JSMN_ERROR_PART = -3
} jsmnerr_t;

typedef struct {
  jsmntype_t type;
  int start;
  int end;
  int size;
} jsmntok_t;

typedef struct {
  unsigned int pos;
  unsigned int toknext;
  int toksuper;
} jsmn_parser;

void jsmn_init(jsmn_parser *parser);
int jsmn_parse(jsmn_parser *parser, const char *js, size_t len,
               jsmntok_t *tokens, unsigned int num_tokens);

#endif /* __JSMN_H_ */