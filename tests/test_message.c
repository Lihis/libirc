#include <stddef.h>
#include <setjmp.h>
#include <stdarg.h>
#include <string.h>
#include <cmocka.h>

#include <irc/message.h>

static void test_message_parse_without_tag(void **data)
{
    irc_message_t m = irc_message_new();
    const char *str = ":prefix COMMAND Arg";
    irc_error_t error = irc_error_internal;

    error = irc_message_parse(m, str, strlen(str));
    assert_int_equal(error, irc_error_success);
    assert_string_equal(m->prefix, "prefix");
    assert_string_equal(m->command, "COMMAND");
    assert_int_equal(m->argslen, 1);
    assert_string_equal(m->args[0], "Arg");
    assert_int_equal(m->tagslen, 0);
    assert_ptr_equal(m->tags, NULL);

    irc_message_unref(m);
}

static void test_message_parse_with_single_tag(void **data)
{
    irc_message_t m = irc_message_new();
    const char *str = "@time=0000 :prefix COMMAND Arg";
    irc_error_t error = irc_error_internal;

    error = irc_message_parse(m, str, strlen(str));
    assert_int_equal(error, irc_error_success);
    assert_string_equal(m->prefix, "prefix");
    assert_string_equal(m->command, "COMMAND");
    assert_int_equal(m->argslen, 1);
    assert_string_equal(m->args[0], "Arg");
    assert_int_equal(m->tagslen, 1);
    assert_string_equal(m->tags[0]->key, "time");
    assert_string_equal(m->tags[0]->value, "0000");

    irc_message_unref(m);
}

static void test_message_parse_with_multiple_tag(void **data)
{
    irc_message_t m = irc_message_new();
    const char *str = "@key1=value1;key2;key3=value3 :prefix COMMAND Arg";
    irc_error_t error = irc_error_internal;

    error = irc_message_parse(m, str, strlen(str));
    assert_int_equal(error, irc_error_success);
    assert_string_equal(m->prefix, "prefix");
    assert_string_equal(m->command, "COMMAND");
    assert_int_equal(m->argslen, 1);
    assert_string_equal(m->args[0], "Arg");
    assert_int_equal(m->tagslen, 3);
    assert_string_equal(m->tags[0]->key, "key1");
    assert_string_equal(m->tags[0]->value, "value1");
    assert_string_equal(m->tags[1]->key, "key2");
    assert_ptr_equal(m->tags[1]->value, NULL);
    assert_string_equal(m->tags[2]->key, "key3");
    assert_string_equal(m->tags[2]->value, "value3");

    irc_message_unref(m);
}

static void test_message_string(void **data)
{
    irc_message_t m = irc_message_new();
    const char *expected = "@key1=value1;key2=value2 :prefix COMMAND Arg\r\n";
    char *str = NULL;
    irc_error_t error = irc_error_internal;
    char *actual = NULL;
    size_t len = 0;

    // Allocate string for parsing without the "\r\n"
    str = calloc(sizeof(char), strlen(expected) - 1);
    memcpy(str, expected, strlen(expected) - 2);
    error = irc_message_parse(m, str, strlen(str));
    assert_return_code(error, irc_error_success);

    error = irc_message_string(m, &actual, &len);
    assert_return_code(error, irc_error_success);
    assert_string_equal(actual, expected);

    free(str);
    free(actual);
    irc_message_unref(m);
}

int main(int ac, char **av)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_message_parse_without_tag),
        cmocka_unit_test(test_message_parse_with_single_tag),
        cmocka_unit_test(test_message_parse_with_multiple_tag),
        cmocka_unit_test(test_message_string),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
