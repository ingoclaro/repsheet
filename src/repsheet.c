#include "hiredis/hiredis.h"
#include "repsheet.h"

void repsheet_init_actor(actor_t *actor)
{
  actor->whitelisted = 0;
  actor->blacklisted = 0;
  actor->offender = 0;
  actor->address = NULL;
}

void repsheet_score_actor(redisContext *context, actor_t *actor)
{
  redisReply *reply;

  reply = redisCommand(context, "GET %s:repsheet:whitelist", actor->address);
  if (reply->str && strcmp(reply->str, "true") == 0) {
    freeReplyObject(reply);
    actor->whitelisted = 1;
  }

  reply = redisCommand(context, "GET %s:repsheet:blacklist", actor->address);
  if (reply->str && strcmp(reply->str, "true") == 0) {
    freeReplyObject(reply);
    actor->blacklisted = 1;
  }

  reply = redisCommand(context, "GET %s:repsheet", actor->address);
  if (reply->str && strcmp(reply->str, "true") == 0) {
    freeReplyObject(reply);
    actor->offender = 1;
  }
}

void repsheet_record(redisContext *context, char *timestamp, const char *user_agent, const char *http_method, char *uri, char *arguments, char *ip, int max_length, int expiry)
{
  char *t, *ua, *method, *u, *args, *rec;

  if (timestamp == NULL) {
    t = malloc(2);
    sprintf(t, "%s", "-");
  } else {
    t = malloc(strlen(timestamp) + 1);
    sprintf(t, "%s", timestamp);
  }

  if (user_agent == NULL) {
    ua = malloc(2);
    sprintf(ua, "%s", "-");
  } else {
    ua = malloc(strlen(user_agent) + 1);
    sprintf(ua, "%s", user_agent);
  }

  if (http_method == NULL) {
    method = malloc(2);
    sprintf(method, "%s", "-");
  } else {
    method = malloc(strlen(http_method) + 1);
    sprintf(method, "%s", http_method);
  }

  if (uri == NULL) {
    u = malloc(2);
    sprintf(u, "%s", "-");
  } else {
    u = malloc(strlen(uri) + 1);
    sprintf(u, "%s", uri);
  }

  if (arguments == NULL) {
    args = malloc(2);
    sprintf(args, "%s", "-");
  } else {
    args = malloc(strlen(arguments) + 1);
    sprintf(args, "%s", arguments);
  }

  rec = (char*)malloc(strlen(t) + strlen(ua) + strlen(method) + strlen(u) + strlen(args) + 9);
  sprintf(rec, "%s, %s, %s, %s, %s", t, ua, method, u, args);

  freeReplyObject(redisCommand(context, "LPUSH %s:requests %s", ip, rec));
  freeReplyObject(redisCommand(context, "LTRIM %s:requests 0 %d", ip, (max_length - 1)));
  if (expiry > 0) {
    freeReplyObject(redisCommand(context, "EXPIRE %s:requests %d", ip, expiry));
  }
}

void blacklist_and_expire(redisContext *context, char *actor, int expiry, char *reason)
{
  redisCommand(context, "SET %s:repsheet:blacklist true", actor);
  redisCommand(context, "EXPIRE %s:repsheet:blacklist %d", actor, expiry);
  redisCommand(context, "SETEX %s:repsheet:blacklist:reason %d %s", actor, expiry, reason);
  redisCommand(context, "SADD repsheet:blacklist:history %s", actor);
}
