#include "../src/repsheet.h"
#include "test_suite.h"

#define TTL 200
#define REDIS_MAX_LENGTH 10

redisContext *context;
redisReply *reply;

void setup(void)
{
  context = redisConnect("localhost", 6379);

  if (context == NULL || context->err) {
    ck_abort_msg("Could not connect to Redis");
  }
}

void teardown(void)
{
  freeReplyObject(redisCommand(context, "flushdb"));
  if (reply) {
    freeReplyObject(reply);
  }
  redisFree(context);
}

START_TEST(handles_null_values)
{
  repsheet_record(context, NULL, NULL, NULL, NULL, NULL, "1.1.1.1", REDIS_MAX_LENGTH, TTL);

  reply = redisCommand(context, "LRANGE 1.1.1.1:requests 0 0");
  ck_assert_str_eq(reply->element[0]->str, "-, -, -, -, -");
}
END_TEST

START_TEST(properly_records_timestamp)
{
  repsheet_record(context, "12345", NULL, NULL, NULL, NULL, "1.1.1.1", REDIS_MAX_LENGTH, TTL);

  reply = redisCommand(context, "LRANGE 1.1.1.1:requests 0 0");
  ck_assert_str_eq(reply->element[0]->str, "12345, -, -, -, -");
}
END_TEST

START_TEST(properly_records_user_agent)
{
  repsheet_record(context, NULL, "A User Agent", NULL, NULL, NULL, "1.1.1.1", REDIS_MAX_LENGTH, TTL);

  reply = redisCommand(context, "LRANGE 1.1.1.1:requests 0 0");
  ck_assert_str_eq(reply->element[0]->str, "-, A User Agent, -, -, -");
}
END_TEST

START_TEST(properly_records_http_method)
{
  repsheet_record(context, NULL, NULL, "GET", NULL, NULL, "1.1.1.1", REDIS_MAX_LENGTH, TTL);

  reply = redisCommand(context, "LRANGE 1.1.1.1:requests 0 0");
  ck_assert_str_eq(reply->element[0]->str, "-, -, GET, -, -");
}
END_TEST

START_TEST(properly_records_uri)
{
  repsheet_record(context, NULL, NULL, NULL, "/", NULL, "1.1.1.1", REDIS_MAX_LENGTH, TTL);

  reply = redisCommand(context, "LRANGE 1.1.1.1:requests 0 0");
  ck_assert_str_eq(reply->element[0]->str, "-, -, -, /, -");
}
END_TEST

START_TEST(properly_records_arguments)
{
  repsheet_record(context, NULL, NULL, NULL, NULL, "foo=bar", "1.1.1.1", REDIS_MAX_LENGTH, TTL);

  reply = redisCommand(context, "LRANGE 1.1.1.1:requests 0 0");
  ck_assert_str_eq(reply->element[0]->str, "-, -, -, -, foo=bar");
}
END_TEST

START_TEST(properly_sets_expiry_on_records)
{
  repsheet_record(context, NULL, NULL, NULL, NULL, NULL, "1.1.1.1", REDIS_MAX_LENGTH, TTL);

  reply = redisCommand(context, "TTL 1.1.1.1:requests");
  ck_assert(reply->integer > 0);
}
END_TEST

START_TEST(does_not_set_expiry_on_records_when_expiry_is_zero)
{
  repsheet_record(context, NULL, NULL, NULL, NULL, NULL, "1.1.1.1", REDIS_MAX_LENGTH, 0);

  reply = redisCommand(context, "TTL 1.1.1.1:requests");
  ck_assert_int_eq(reply->integer, -1);

  reply = redisCommand(context, "LRANGE 1.1.1.1:requests 0 0");
  ck_assert_str_eq(reply->element[0]->str, "-, -, -, -, -");
}
END_TEST

START_TEST(properly_trims_length_of_stored_records)
{
  repsheet_record(context, NULL, NULL, NULL, NULL, NULL, "1.1.1.1", 2, TTL);
  repsheet_record(context, NULL, NULL, NULL, NULL, NULL, "1.1.1.1", 2, TTL);
  repsheet_record(context, NULL, NULL, NULL, NULL, NULL, "1.1.1.1", 2, TTL);

  reply = redisCommand(context, "LRANGE 1.1.1.1:requests 0 -1");
  ck_assert_int_eq(reply->elements, 2);
}
END_TEST

START_TEST(repsheet_init_actor_initializes_with_the_proper_defaults)
{
  actor_t actor;
  repsheet_init_actor(&actor);
  ck_assert_int_eq(actor.whitelisted, 0);
  ck_assert_int_eq(actor.blacklisted, 0);
  ck_assert_int_eq(actor.offender, 0);
  ck_assert(actor.address == NULL);
}
END_TEST

START_TEST(repsheet_score_actor_properly_sets_whitelist_and_blacklist)
{
  freeReplyObject(redisCommand(context, "SET 1.1.1.1:repsheet:blacklist true"));
  freeReplyObject(redisCommand(context, "SET 1.1.1.1:repsheet:whitelist true"));
  actor_t actor;
  repsheet_init_actor(&actor);
  actor.address = "1.1.1.1";
  repsheet_score_actor(context, &actor);

  ck_assert_int_eq(actor.whitelisted, 1);
  ck_assert_int_eq(actor.blacklisted, 1);
}
END_TEST

START_TEST(repsheet_score_actor_properly_sets_offender_bit_on_actor)
{
  freeReplyObject(redisCommand(context, "SET 1.1.1.1:repsheet true"));
  actor_t actor;
  repsheet_init_actor(&actor);
  actor.address = "1.1.1.1";
  repsheet_score_actor(context, &actor);

  ck_assert_int_eq(actor.offender, 1);
}
END_TEST

Suite *make_repsheet_suite(void) {
  Suite *suite = suite_create("librepsheet");

  TCase *tc_record = tcase_create("repsheet_record");
  tcase_add_checked_fixture(tc_record, setup, teardown);
  tcase_add_test(tc_record, handles_null_values);
  tcase_add_test(tc_record, properly_records_timestamp);
  tcase_add_test(tc_record, properly_records_user_agent);
  tcase_add_test(tc_record, properly_records_http_method);
  tcase_add_test(tc_record, properly_records_arguments);
  tcase_add_test(tc_record, properly_records_uri);
  tcase_add_test(tc_record, properly_sets_expiry_on_records);
  tcase_add_test(tc_record, does_not_set_expiry_on_records_when_expiry_is_zero);
  tcase_add_test(tc_record, properly_trims_length_of_stored_records);
  suite_add_tcase(suite, tc_record);

  TCase *tc_score_actor = tcase_create("repsheet_score_actor");
  tcase_add_checked_fixture(tc_score_actor, setup, teardown);
  tcase_add_test(tc_score_actor, repsheet_init_actor_initializes_with_the_proper_defaults);
  tcase_add_test(tc_score_actor, repsheet_score_actor_properly_sets_whitelist_and_blacklist);
  tcase_add_test(tc_score_actor, repsheet_score_actor_properly_sets_offender_bit_on_actor);
  suite_add_tcase(suite, tc_score_actor);

  return suite;
}
