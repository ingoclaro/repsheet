#ifndef __REPSHEET_H
#define __REPSHEET_H

#include <stdlib.h>
#include <string.h>
#include "hiredis/hiredis.h"

#define ALLOW 0
#define NOTIFY 1
#define BLOCK 2

typedef struct actor_t {
  char *address;
  int offender;
  int blacklisted;
  int whitelisted;
} actor_t;

void repsheet_init_actor(actor_t *actor);
void repsheet_score_actor(redisContext *context, actor_t *actor);
void repsheet_record(redisContext *context, char *timestamp, const char *user_agent, const char *http_method, char* uri, char *arguments, char *ip, int max_length, int expiry);

#endif
