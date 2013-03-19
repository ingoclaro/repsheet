#include <stdio.h>

#include "http_core.h"
#include "http_protocol.h"
#include "http_request.h"
#include "http_config.h"
#include "http_log.h"

#include "hiredis/hiredis.h"

#define UNDEFINED 0
#define NOTIFY 1
#define BLOCK 2

typedef struct {
  int enabled;
  int timeout;
  int action;
  const char *prefix;
} repsheet_config;
static repsheet_config config;

const char *repsheet_set_enabled(cmd_parms *cmd, void *cfg, const char *arg)
{
  if (!strcasecmp(arg, "on")) config.enabled = 1; else config.enabled = 0;
  return NULL;
}

const char *repsheet_set_timeout(cmd_parms *cmd, void *cfg, const char *arg)
{
  config.timeout = atoi(arg) * 1000;
  return NULL;
}

const char *repsheet_set_prefix(cmd_parms *cmd, void *cfg, const char *arg)
{
  //TODO: test if directive argument is missing and set a default
  config.prefix = arg;
  return NULL;
}

const char *repsheet_set_action(cmd_parms *cmd, void *cfg, const char *arg)
{
  if (strcmp(arg, "Notify") == 0) {
    config.action = NOTIFY;
  } else if (strcmp(arg, "Block") == 0) {
    config.action = BLOCK;
  } else {
    config.action = UNDEFINED;
  }
  return NULL;
}

static const command_rec repsheet_directives[] =
  {
    AP_INIT_TAKE1("repsheetEnabled", repsheet_set_enabled, NULL, RSRC_CONF, "Enable or disable mod_repsheet"),
    AP_INIT_TAKE1("repsheetRedisTimeout", repsheet_set_timeout, NULL, RSRC_CONF, "Set the Redis timeout"),
    AP_INIT_TAKE1("repsheetAction", repsheet_set_action, NULL, RSRC_CONF, "Set the action"),
    AP_INIT_TAKE1("repsheetPrefix", repsheet_set_prefix, NULL, RSRC_CONF, "Set the log prefix"),
    { NULL }
  };

static int repsheet_handler(request_rec *r)
{
  if (config.enabled == 1) {

    apr_time_exp_t start;
    char           human_time[50];
    char           value[256]; // TODO: potential overflow here
    redisContext   *c;
    redisReply     *reply;

    struct timeval timeout = {0, (config.timeout > 0) ? config.timeout : 10000};

    c = redisConnectWithTimeout((char*) "127.0.0.1", 6379, timeout);
    if (c == NULL || c->err) {
      if (c) {
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server, "%s Redis Connection Error: %s", config.prefix, c->errstr);
        redisFree(c);
      } else {
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server, "%s Connection Error: can't allocate redis context", config.prefix);
      }

      return DECLINED;
    }

    reply = redisCommand(c, "SISMEMBER repsheet %s", r->connection->remote_ip);
    if (reply->integer == 1) {
      if (config.action == BLOCK) {
	return HTTP_FORBIDDEN;
      } else {
	apr_table_set(r->headers_in, "X-Repsheet", "true");
      }
    }
    freeReplyObject(reply);

    apr_time_exp_gmt(&start, r->request_time);
    sprintf(human_time, "%d/%d/%d %d:%d:%d.%d", (start.tm_mon + 1), start.tm_mday, (1900 + start.tm_year), start.tm_hour, start.tm_min, start.tm_sec, start.tm_usec);
    sprintf(value, "%s,%s,%s,%s,%s", human_time, apr_table_get(r->headers_in, "User-Agent"), r->method, r->uri, r->args);

    reply = redisCommand(c, "RPUSH %s %s", r->connection->remote_ip, value);
    ap_log_error(APLOG_MARK, APLOG_INFO, 0, r->server, "%s Added data for %s", config.prefix, r->connection->remote_ip);
    freeReplyObject(reply);

    const char *waf_events = apr_table_get(r->headers_in, "X-WAF-Events");
    const char *waf_score = apr_table_get(r->headers_in, "X-WAF-Score");

    if (waf_events && waf_score ) {
      // X-WAF-Events: TX:960017-POLICY/IP_HOST-REQUEST_HEADERS:Host, TX:0, TX:1
      // X-WAF-Score: Total=2; sqli=; xss=
      
      ap_log_error(APLOG_MARK, APLOG_INFO, 0, r->server, "%s WAF Event Triggered: %s", config.prefix, waf_events);
      ap_log_error(APLOG_MARK, APLOG_INFO, 0, r->server, "%s WAF Score: %s", config.prefix, waf_score);

      reply = redisCommand(c, "SADD waf %s", r->connection->remote_ip);
      freeReplyObject(reply);
    }

    redisFree(c);
  }

  return DECLINED;
}

static void register_hooks(apr_pool_t *pool)
{
  ap_hook_fixups(repsheet_handler, NULL, NULL, APR_HOOK_REALLY_LAST);
}

module AP_MODULE_DECLARE_DATA repsheet_module =
  {
    STANDARD20_MODULE_STUFF,
    NULL,                /* Per-directory configuration handler */
    NULL,                /* Merge handler for per-directory configurations */
    NULL,                /* Per-server configuration handler */
    NULL,                /* Merge handler for per-server configurations */
    repsheet_directives, /* Any directives we may have for httpd */
    register_hooks       /* Our hook registering function */
  };
