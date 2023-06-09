﻿#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <iostream>
#include <cstring>
#include <vector>

#include "lept_value.h"
#include "lept_json.h"

using namespace std;

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

template <typename T>
void EXPECT_EQ_BASE(bool equality, T expect, T actual, string format, unsigned _line) {
	test_count++;
	if (equality)
		test_pass++;
	else {
		std::cerr << __FILE__ << ":" << _line << " : expect:" << expect << " actual:" << actual << std::endl;
		main_ret = 1;
	}
}

template <typename T>
void EXPECT_EQ_INT(T expect, T actual, unsigned _line) {
	EXPECT_EQ_BASE(as_integer(expect) == as_integer(actual), expect, actual, "%d", _line);
}

template <>
void EXPECT_EQ_INT(bool expect, bool actual, unsigned _line) {
	EXPECT_EQ_BASE(expect == actual, expect, actual, "%d", _line);
}

void EXPECT_EQ_DOUBLE(double expect, double actual, unsigned _line) {
	EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g", _line);
}

void EXPECT_EQ_STRING(const string& expect, const string& actual, size_t len, unsigned _line) {
	EXPECT_EQ_BASE((expect.size() == len) && memcmp(expect.data(), actual.data(), len) == 0, expect, actual, "%s", _line);
}

#if defined(_MSC_VER)
void EXPECT_EQ_SIZE_T(size_t expect, size_t actual, unsigned _line) {
	EXPECT_EQ_BASE(expect == actual, (size_t)expect, (size_t)actual, "%Iu", _line);
}
#else 
void EXPECT_EQ_SIZE_T(size_t expect, size_t actual, unsigned _line) {
	EXPECT_EQ_BASE(expect == actual, (size_t)expect, (size_t)actual, "%zu", _line);
}
#endif

void EXPECT_TRUE(bool actual, unsigned _line) {
	EXPECT_EQ_BASE(actual != false, "true", "false", "%s", _line);
}

void EXPECT_FALSE(bool actual, unsigned _line) {
	EXPECT_EQ_BASE(actual == false, "false", "true", "%s", _line);
}

void test_parse_null() {
	lept_value lv;
	lept_json lj("null");
	lv.set_type(lept_type::LEPT_FALSE);
	EXPECT_EQ_INT(Expect::LEPT_PARSE_OK, lj.lept_parse(lv), __LINE__);
	EXPECT_EQ_INT(lept_type::LEPT_NULL, lv.lept_get_type(), __LINE__);
}

void test_parse_true() {
	lept_value lv;
	lept_json lj("true");
	lv.set_type(lept_type::LEPT_FALSE);
	EXPECT_EQ_INT(Expect::LEPT_PARSE_OK, lj.lept_parse(lv), __LINE__);
	EXPECT_EQ_INT(lept_type::LEPT_TRUE, lv.lept_get_type(), __LINE__);
}

void test_parse_false() {
	lept_value lv;
	lept_json lj("false");
	lv.lept_set_boolean(false);
	EXPECT_EQ_INT(Expect::LEPT_PARSE_OK, lj.lept_parse(lv), __LINE__);
	EXPECT_EQ_INT(lept_type::LEPT_FALSE, lv.lept_get_type(), __LINE__);
}

void TEST_NUMBER(double expect, const std::string& json) {
	lept_value lv;
	lept_json lj(json.c_str());
	lv.lept_set_boolean(false);
	EXPECT_EQ_INT(Expect::LEPT_PARSE_OK, lj.lept_parse(lv), __LINE__);
	EXPECT_EQ_INT(lept_type::LEPT_NUMBER, lv.lept_get_type(), __LINE__);
	EXPECT_EQ_DOUBLE(expect, lv.lept_get_number(), __LINE__);
}

void TEST_ERROR(Expect error, const string& json, unsigned _line) {
	lept_value lv;
	lept_json lj(json.c_str());
	lv.lept_set_boolean(false);
	EXPECT_EQ_INT(error, lj.lept_parse(lv), __LINE__);
	EXPECT_EQ_INT(lept_type::LEPT_NULL, lv.lept_get_type(), __LINE__);
}

void TEST_STRING(const string& expect, const string& json, unsigned _line) {
	lept_value lv;
	lept_json lj(json.c_str());
	EXPECT_EQ_INT(Expect::LEPT_PARSE_OK, lj.lept_parse(lv), __LINE__);
	EXPECT_EQ_INT(lept_type::LEPT_STRING, lv.lept_get_type(), __LINE__);
	EXPECT_EQ_STRING(expect, lv.lept_get_string(), lv.lept_get_string_length(), __LINE__);
}

void test_parse_number() {
	TEST_NUMBER(0.0, "0");
	TEST_NUMBER(0.0, "-0");
	TEST_NUMBER(0.0, "-0.0");
	TEST_NUMBER(1.0, "1");
	TEST_NUMBER(-1.0, "-1");
	TEST_NUMBER(1.5, "1.5");
	TEST_NUMBER(-1.5, "-1.5");
	TEST_NUMBER(3.1416, "3.1416");
	TEST_NUMBER(1E10, "1E10");
	TEST_NUMBER(1e10, "1e10");
	TEST_NUMBER(1E+10, "1E+10");
	TEST_NUMBER(1E-10, "1E-10");
	TEST_NUMBER(-1E10, "-1E10");
	TEST_NUMBER(-1e10, "-1e10");
	TEST_NUMBER(-1E+10, "-1E+10");
	TEST_NUMBER(-1E-10, "-1E-10");
	TEST_NUMBER(1.234E+10, "1.234E+10");
	TEST_NUMBER(1.234E-10, "1.234E-10");
	TEST_NUMBER(0.0, "1e-10000"); // must underflow

	TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); // the smallest number > 1
	TEST_NUMBER(4.9406564584124654e-324, "4.9406564584124654e-324"); // minimum denormal
	TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
	TEST_NUMBER(2.2250738585072009e-308, "2.2250738585072009e-308"); // Max subnormal double
	TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
	TEST_NUMBER(1.7976931348623157e+308, "1.7976931348623157e+308"); // max double
	TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

void test_parse_string() {
	TEST_STRING("", "\"\"", __LINE__);
	TEST_STRING("Hello", "\"Hello\"", __LINE__);
#if 1
	TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"", __LINE__);
	TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"", __LINE__);
#endif
	const string test_str = string("Hello") + '\0' + string("World");
	TEST_STRING(test_str, "\"Hello\\u0000World\"", __LINE__);
	TEST_STRING("\x24", "\"\\u0024\"", __LINE__); // Dollar sign U+0024
	TEST_STRING("\xC2\xA2", "\"\\u00A2\"", __LINE__); // Cents sign U+00A2
	TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\"", __LINE__); // Euro sign U+20AC
	TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"", __LINE__); // G clef sign U+1D11E
	TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"", __LINE__); // G clef sign U+1D11E
}

void test_parse_array() {
	lept_value lv;
	lept_json lj("[ ]");
	EXPECT_EQ_INT(Expect::LEPT_PARSE_OK, lj.lept_parse(lv), __LINE__);
	EXPECT_EQ_INT(lept_type::LEPT_ARRAY, lv.lept_get_type(), __LINE__);
	EXPECT_EQ_SIZE_T(0, lv.lept_get_array_size(), __LINE__);

	lv.clear();

	lept_json lj2("[ null , false , true , 123 , \"abc\" ]");
	EXPECT_EQ_INT(Expect::LEPT_PARSE_OK, lj2.lept_parse(lv), __LINE__);
	EXPECT_EQ_INT(lept_type::LEPT_ARRAY, lv.lept_get_type(), __LINE__);
	EXPECT_EQ_SIZE_T(5, lv.lept_get_array_size(), __LINE__);
	EXPECT_EQ_INT(lept_type::LEPT_NULL, lv.lept_get_array_element(0).lept_get_type(), __LINE__);
	EXPECT_EQ_INT(lept_type::LEPT_FALSE, lv.lept_get_array_element(1).lept_get_type(), __LINE__);
	EXPECT_EQ_INT(lept_type::LEPT_TRUE, lv.lept_get_array_element(2).lept_get_type(), __LINE__);
	EXPECT_EQ_INT(lept_type::LEPT_NUMBER, lv.lept_get_array_element(3).lept_get_type(), __LINE__);
	EXPECT_EQ_INT(lept_type::LEPT_STRING, lv.lept_get_array_element(4).lept_get_type(), __LINE__);
	EXPECT_EQ_DOUBLE(123.0, lv.lept_get_array_element(3).lept_get_number(), __LINE__);
	EXPECT_EQ_STRING("abc", lv.lept_get_array_element(4).lept_get_string(), lv.lept_get_array_element(4).lept_get_string_length(), __LINE__);
	lv.clear();

	lept_json lj3("[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]");
	EXPECT_EQ_INT(Expect::LEPT_PARSE_OK, lj3.lept_parse(lv), __LINE__);
	EXPECT_EQ_INT(lept_type::LEPT_ARRAY, lv.lept_get_type(), __LINE__);
	EXPECT_EQ_SIZE_T(4, lv.lept_get_array_size(), __LINE__);
	for (int i = 0; i < 4; i++) {
		lept_value a = lv.lept_get_array_element(i);
		EXPECT_EQ_INT(lept_type::LEPT_ARRAY, a.lept_get_type(), __LINE__);
		EXPECT_EQ_SIZE_T(i, a.lept_get_array_size(), __LINE__);
		for (int j = 0; j < i; j++) {
			lept_value e = a.lept_get_array_element(j);
			EXPECT_EQ_INT(lept_type::LEPT_NUMBER, e.lept_get_type(), __LINE__);
			EXPECT_EQ_DOUBLE((double)j, e.lept_get_number(), __LINE__);
		}
	}
	lv.clear();
}

void test_parse_object() {
	lept_value lv;
	lept_json lj(" { } ");

	EXPECT_EQ_INT(Expect::LEPT_PARSE_OK, lj.lept_parse(lv), __LINE__);
	EXPECT_EQ_INT(lept_type::LEPT_OBJECT, lv.lept_get_type(), __LINE__);
	EXPECT_EQ_SIZE_T(0, lv.lept_get_object_size(), __LINE__);

	lj.reset_json(" { "
		"\"n\" : null , "
		"\"f\" : false , "
		"\"t\" : true , "
		"\"i\" : 123 , "
		"\"s\" : \"abc\" , "
		"\"a\" : [ 1 , 2 , 3 ] , "
		"\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3}"
		" } ");
	lv.clear();

	EXPECT_EQ_INT(Expect::LEPT_PARSE_OK, lj.lept_parse(lv), __LINE__);
	EXPECT_EQ_INT(lept_type::LEPT_OBJECT, lv.lept_get_type(), __LINE__);
	EXPECT_EQ_SIZE_T(7, lv.lept_get_object_size(), __LINE__);
	EXPECT_EQ_STRING("n", lv.lept_get_object_key(0), lv.lept_get_object_key_length(0), __LINE__);
	EXPECT_EQ_INT(lept_type::LEPT_NULL, lv.lept_get_object_member_value(0).lept_get_type(), __LINE__);
	EXPECT_EQ_STRING("f", lv.lept_get_object_key(1), lv.lept_get_object_key_length(1), __LINE__);
	EXPECT_EQ_INT(lept_type::LEPT_FALSE, lv.lept_get_object_member_value(1).lept_get_type(), __LINE__);
	EXPECT_EQ_STRING("t", lv.lept_get_object_key(2), lv.lept_get_object_key_length(2), __LINE__);
	EXPECT_EQ_INT(lept_type::LEPT_TRUE, lv.lept_get_object_member_value(2).lept_get_type(), __LINE__);
	EXPECT_EQ_STRING("i", lv.lept_get_object_key(3), lv.lept_get_object_key_length(3), __LINE__);
	EXPECT_EQ_INT(lept_type::LEPT_NUMBER, lv.lept_get_object_member_value(3).lept_get_type(), __LINE__);
	EXPECT_EQ_DOUBLE(123.0, lv.lept_get_object_member_value(3).lept_get_number(), __LINE__);
	EXPECT_EQ_STRING("s", lv.lept_get_object_key(4), lv.lept_get_object_key_length(4), __LINE__);
	EXPECT_EQ_INT(lept_type::LEPT_STRING, lv.lept_get_object_member_value(4).lept_get_type(), __LINE__);
	EXPECT_EQ_STRING("abc", lv.lept_get_object_member_value(4).lept_get_string(), lv.lept_get_object_member_value(4).lept_get_string_length(), __LINE__);
	EXPECT_EQ_STRING("a", lv.lept_get_object_key(5), lv.lept_get_object_key_length(5), __LINE__);
	EXPECT_EQ_INT(lept_type::LEPT_ARRAY, lv.lept_get_object_member_value(5).lept_get_type(), __LINE__);
	EXPECT_EQ_SIZE_T(3, lv.lept_get_object_member_value(5).lept_get_array_size(), __LINE__);
	for (int i = 0; i < 3; i++) {
		lept_value e = lv.lept_get_object_member_value(5).lept_get_array_element(i);
		EXPECT_EQ_INT(lept_type::LEPT_NUMBER, e.lept_get_type(), __LINE__);
		EXPECT_EQ_DOUBLE(i + 1.0, e.lept_get_number(), __LINE__);
	}
	EXPECT_EQ_STRING("o", lv.lept_get_object_key(6), lv.lept_get_object_key_length(6), __LINE__);
	{
		lept_value m_value = lv.lept_get_object_member_value(6);
		EXPECT_EQ_INT(lept_type::LEPT_OBJECT, m_value.lept_get_type(), __LINE__);
		EXPECT_EQ_SIZE_T(3, m_value.lept_get_object_size(), __LINE__);

		for (int i = 0; i < 3; i++) {
			lept_value o_value = m_value.lept_get_object_member_value(i);
			EXPECT_TRUE('1' + i == m_value.lept_get_object_key(i)[0], __LINE__);
			EXPECT_EQ_SIZE_T(1, m_value.lept_get_object_key_length(i), __LINE__);
			EXPECT_EQ_INT(lept_type::LEPT_NUMBER, o_value.lept_get_type(), __LINE__);
			EXPECT_EQ_DOUBLE(i + 1.0, o_value.lept_get_number(), __LINE__);
		}
	}
}

void test_parse_expect_value() {
	TEST_ERROR(Expect::LEPT_PARSE_EXPECT_VALUE, "", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_EXPECT_VALUE, " ", __LINE__);
}

void test_parse_invalid_value() {
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_VALUE, "nul", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_VALUE, "?", __LINE__);

#if 1
	// invalid number
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_VALUE, "+0", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_VALUE, "+1", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_VALUE, ".123", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_VALUE, "1.", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_VALUE, "INF", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_VALUE, "inf", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_VALUE, "NAN", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_VALUE, "nan", __LINE__);
#endif

	// invalid value in array
#if 1
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_VALUE, "[1,]", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_VALUE, "[\"a\", nul]", __LINE__);
#endif
}

void test_parse_root_not_singular() {
	TEST_ERROR(Expect::LEPT_PARSE_ROOT_NOT_SINGULAR, "null x", __LINE__);

#if 1
	//invalid number
	TEST_ERROR(Expect::LEPT_PARSE_ROOT_NOT_SINGULAR, "0123", __LINE__); // after 0 should be '.', 'E', 'e' or nothing
	TEST_ERROR(Expect::LEPT_PARSE_ROOT_NOT_SINGULAR, "0x0", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_ROOT_NOT_SINGULAR, "0x123", __LINE__);
#endif
}

void test_parse_number_too_big() {
#if 1
	TEST_ERROR(Expect::LEPT_PARSE_NUMBER_TOO_BIG, "1e309", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_NUMBER_TOO_BIG, "-1e309", __LINE__);
#endif
}

void test_parse_miss_quotation_mark() {
	TEST_ERROR(Expect::LEPT_PARSE_MISS_QUOTATION_MARK, "\"", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_MISS_QUOTATION_MARK, "\"abc", __LINE__);
}

void test_parse_invalid_string_escape() {
#if 1
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"", __LINE__);
#endif
}

void test_parse_invalid_string_char() {
#if 1
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_STRING_CHAR, "\"\x01\"", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_STRING_CHAR, "\"\x1F\"", __LINE__);
#endif
}

void test_parse_invalid_unicode_hex() {
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u\"", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u00/0\"", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"", __LINE__);
}

void test_parse_invalid_unicode_surrogate() {
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"", __LINE__);
}

void test_parse_miss_camma_or_square_bracket() {
#if 1
	TEST_ERROR(Expect::LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]", __LINE__);
#endif
}

void test_parse_miss_key() {
	TEST_ERROR(Expect::LEPT_PARSE_MISS_KEY, "{:1,", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_MISS_KEY, "{1:1,", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_MISS_KEY, "{true:1,", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_MISS_KEY, "{false:1,", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_MISS_KEY, "{null:1,", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_MISS_KEY, "{[]:1,", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_MISS_KEY, "{{}:1,", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_MISS_KEY, "{\"a\":1,", __LINE__);
}

void test_parse_miss_colon() {
	TEST_ERROR(Expect::LEPT_PARSE_MISS_COLON, "{\"a\"}", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_MISS_COLON, "{\"a\", \"b\"}", __LINE__);
}

void test_parse_miss_comma_or_curly_bracket() {
	TEST_ERROR(Expect::LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1]", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1 \"b\"", __LINE__);
	TEST_ERROR(Expect::LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":{}", __LINE__);
}

static void test_access_null() {
	lept_value lv;
	lv.lept_set_string("a");
	lv.lept_set_null();
	EXPECT_EQ_INT(lept_type::LEPT_NULL, lv.lept_get_type(), __LINE__);
}

static void test_access_boolean() {
	lept_value lv;
	lv.lept_set_string("abc");
	lv.lept_set_boolean(true);
	EXPECT_TRUE(lv.lept_get_boolean(), __LINE__);
	lv.lept_set_boolean(false);
	EXPECT_FALSE(lv.lept_get_boolean(), __LINE__);
}

static void test_access_number() {
	lept_value lv;
	lv.lept_set_string("a");
	lv.lept_set_number(100.0);
	EXPECT_EQ_DOUBLE(100.0, lv.lept_get_number(), __LINE__);
}

static void test_access_string() {
	lept_value lv;
	lv.lept_set_string("");
	EXPECT_EQ_STRING("", lv.lept_get_string(), lv.lept_get_string_length(), __LINE__);
	lv.lept_set_string("Hello");
	EXPECT_EQ_STRING("Hello", lv.lept_get_string(), lv.lept_get_string_length(), __LINE__);
}

void test_parse() {
	test_parse_null();
	test_parse_true();
	test_parse_false();
	test_parse_number();
	test_parse_string();
	test_parse_array();
#if 1
	test_parse_object();
#endif

	test_parse_expect_value();
	test_parse_invalid_value();
	test_parse_root_not_singular();
	test_parse_number_too_big();
	test_parse_miss_quotation_mark();
	test_parse_invalid_string_escape();
	test_parse_invalid_string_char();
	test_parse_invalid_unicode_hex();
	test_parse_invalid_unicode_surrogate();
	test_parse_miss_camma_or_square_bracket();
#if 1
	test_parse_miss_key();
	test_parse_miss_colon();
	test_parse_miss_comma_or_curly_bracket();
#endif
}

void TEST_ROUNDTRIP(const char* json) {
	lept_value lv;
	lept_json lj(json);
	std::string json2;

	EXPECT_EQ_INT(Expect::LEPT_PARSE_OK, lj.lept_parse(lv), __LINE__);
	json2 = lj.lept_stringify(lv);
	EXPECT_EQ_STRING(json, json2, json2.size(), __LINE__);
}

void test_stringify_number() {
	TEST_ROUNDTRIP("0");
	TEST_ROUNDTRIP("-0");
	TEST_ROUNDTRIP("1");
	TEST_ROUNDTRIP("-1");
	TEST_ROUNDTRIP("1.5");
	TEST_ROUNDTRIP("-1.5");
	TEST_ROUNDTRIP("3.25");
	TEST_ROUNDTRIP("1e+20");
	TEST_ROUNDTRIP("1.234e+20");
	TEST_ROUNDTRIP("1.234e-20");

	TEST_ROUNDTRIP("1.0000000000000002"); //the smallest number > 1 
	TEST_ROUNDTRIP("4.9406564584124654e-324"); //  minimum denormal 
	TEST_ROUNDTRIP("-4.9406564584124654e-324");
	TEST_ROUNDTRIP("2.2250738585072009e-308");  // Max subnormal double 
	TEST_ROUNDTRIP("-2.2250738585072009e-308");
	TEST_ROUNDTRIP("2.2250738585072014e-308");  // Min normal positive double 
	TEST_ROUNDTRIP("-2.2250738585072014e-308");
	TEST_ROUNDTRIP("1.7976931348623157e+308");  // Max double 
	TEST_ROUNDTRIP("-1.7976931348623157e+308");
}

void test_stringify_string() {
	TEST_ROUNDTRIP("\"\"");
	TEST_ROUNDTRIP("\"Hello\"");
	TEST_ROUNDTRIP("\"Hello\\nWorld\"");
	TEST_ROUNDTRIP("\"\\\" \\\\ / \\b \\f \\n \\r \\t\"");
	TEST_ROUNDTRIP("\"Hello\\u0000World\"");
}

void test_stringify_array() {
	TEST_ROUNDTRIP("[]");
	TEST_ROUNDTRIP("[null,false,true,123,\"abc\",[1,2,3]]");
}

void test_stringify_object() {
	TEST_ROUNDTRIP("{}");
	TEST_ROUNDTRIP("{\"n\":null,\"f\":false,\"t\":true,\"i\":123,\"s\":\"abc\",\"a\":[1,2,3],\"o\":{\"1\":1,\"2\":2,\"3\":3}}");
}

void test_stringify() {
	TEST_ROUNDTRIP("null");
	TEST_ROUNDTRIP("false");
	TEST_ROUNDTRIP("true");
	test_stringify_number();
	test_stringify_string();
	test_stringify_array();
	test_stringify_object();
}

bool lept_is_equal(const lept_value& lhs, const lept_value& rhs) {
	size_t i;
	if (lhs.lept_get_type() != rhs.lept_get_type())
		return false;
	switch (lhs.lept_get_type()) {
	case lept_type::LEPT_STRING:
		return lhs.lept_get_string_length() == rhs.lept_get_string_length() &&
			memcmp(lhs.lept_get_string().data(), rhs.lept_get_string().data(), lhs.lept_get_string_length()) == 0;
	case lept_type::LEPT_NUMBER:
		return lhs.lept_get_number() == rhs.lept_get_number();
	case lept_type::LEPT_ARRAY:
		if (lhs.lept_get_array_size() != rhs.lept_get_array_size())
			return false;
		for (i = 0; i < lhs.lept_get_array_size(); i++)
			if (!lept_is_equal(lhs.lept_get_array_element(i), rhs.lept_get_array_element(i)))
				return false;
		return true;
	case lept_type::LEPT_OBJECT:
		if (lhs.lept_get_object_size() != rhs.lept_get_object_size())
			return false;
		for (i = 0; i < lhs.lept_get_object_size(); ++i)
			if (!lept_is_equal(lhs.lept_get_object_member_value(i), rhs.lept_get_object_member_value(rhs.lept_find_object_member_index(lhs.lept_get_object_key(i))))) {
				return false;
			}
		return true;
	default:
		return true;
	}
}

void TEST_EQUAL(const char* json1, const char* json2, bool equality) {
	lept_value lv1, lv2;
	lept_json lj(json1);
	EXPECT_EQ_INT(Expect::LEPT_PARSE_OK, lj.lept_parse(lv1), __LINE__);
	lj.reset_json(json2);
	EXPECT_EQ_INT(Expect::LEPT_PARSE_OK, lj.lept_parse(lv2), __LINE__);
	EXPECT_EQ_INT(equality, lept_is_equal(lv1, lv2), __LINE__);
}

void test_equal() {
	TEST_EQUAL("true", "true", 1);
	TEST_EQUAL("true", "false", 0);
	TEST_EQUAL("false", "false", 1);
	TEST_EQUAL("null", "null", 1);
	TEST_EQUAL("null", "0", 0);
	TEST_EQUAL("123", "123", 1);
	TEST_EQUAL("123", "456", 0);
	TEST_EQUAL("\"abc\"", "\"abc\"", 1);
	TEST_EQUAL("\"abc\"", "\"abcd\"", 0);
	TEST_EQUAL("[]", "[]", 1);
	TEST_EQUAL("[]", "null", 0);
	TEST_EQUAL("[1, 2, 3]", "[1, 2, 3]", 1);
	TEST_EQUAL("[1, 2, 3]", "[1, 2, 3, 4]", 0);
	TEST_EQUAL("[[]]", "[[]]", 1);
	TEST_EQUAL("{}", "{}", 1);
	TEST_EQUAL("{}", "null", 0);
	TEST_EQUAL("{}", "[]", 0);
	TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":2}", 1);
	TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"b\":2,\"a\":1}", 1);
	TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":3}", 0);
	TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":2,\"c\":3}", 0);
	TEST_EQUAL("{\"a\":{\"b\":{\"c\":{}}}}", "{\"a\":{\"b\":{\"c\":{}}}}", 1);
	TEST_EQUAL("{\"a\":{\"b\":{\"c\":{}}}}", "{\"a\":{\"b\":{\"c\":[]}}}", 0);
}

void test_copy() {
	lept_value lv1, lv2;
	lept_json lj("{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3]}");
	lj.lept_parse(lv1);
	lj.lept_copy(lv2, lv1);
	EXPECT_TRUE(lept_is_equal(lv2, lv1), __LINE__);
}

void test_move() {
	lept_value lv1, lv2, lv3;
	lept_json lj("{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3]}");
	lj.lept_parse(lv1);
	lj.lept_copy(lv2, lv1);
	lj.lept_move(lv3, lv2);
	EXPECT_EQ_INT(lept_type::LEPT_NULL, lv2.lept_get_type(), __LINE__);
	EXPECT_TRUE(lept_is_equal(lv3, lv1), __LINE__);
}

void test_swap() {
	lept_value lv1, lv2;
	lept_json lj("Hello");
	lv1.lept_set_string("Hello");
	lv2.lept_set_string("World!");
	lj.lept_swap(lv1, lv2);
	EXPECT_EQ_STRING("World!", lv1.lept_get_string(), lv1.lept_get_string_length(), __LINE__);
	EXPECT_EQ_STRING("Hello", lv2.lept_get_string(), lv2.lept_get_string_length(), __LINE__);
}

void test_access_array() {
	lept_value lv1, lv2;
	size_t i, j;
	for (j = 0; j <= 5; j += 5) {
		lv1.lept_set_array_capacity({}, j);
		EXPECT_EQ_SIZE_T(0, lv1.lept_get_array_size(), __LINE__);
		for (i = 0; i < 10; i++) {
			lv2.lept_set_number(i);
			lv1.lept_push_array_element(lv2);
			lv2.clear();
		}

		EXPECT_EQ_SIZE_T(10, lv1.lept_get_array_size(), __LINE__);
		for (i = 0; i < 10; i++)
			EXPECT_EQ_DOUBLE((double)i, lv1.lept_get_array_element(i).lept_get_number(), __LINE__);
	}

	lv1.lept_pop_array_element();
	EXPECT_EQ_SIZE_T(9, lv1.lept_get_array_size(), __LINE__);
	for (i = 0; i < 9; i++)
		EXPECT_EQ_DOUBLE((double)i, lv1.lept_get_array_element(i).lept_get_number(), __LINE__);

	lv1.lept_erase_array_element(4, 0);
	EXPECT_EQ_SIZE_T(9, lv1.lept_get_array_size(), __LINE__);
	for (i = 0; i < 9; i++)
		EXPECT_EQ_DOUBLE((double)i, lv1.lept_get_array_element(i).lept_get_number(), __LINE__);

	lv1.lept_erase_array_element(8, 1);
	EXPECT_EQ_SIZE_T(8, lv1.lept_get_array_size(), __LINE__);
	for (i = 0; i < 8; i++)
		EXPECT_EQ_DOUBLE((double)i, lv1.lept_get_array_element(i).lept_get_number(), __LINE__);

	lv1.lept_erase_array_element(0, 2);
	EXPECT_EQ_SIZE_T(6, lv1.lept_get_array_size(), __LINE__);
	for (i = 0; i < 6; i++)
		EXPECT_EQ_DOUBLE((double)i + 2, lv1.lept_get_array_element(i).lept_get_number(), __LINE__);

#if 1
	for (i = 0; i < 2; i++) {
		lv2.lept_set_number(i);
		lv1.lept_insert_array_element(lv2, i);
		lv2.clear();
	}
#endif
	EXPECT_EQ_SIZE_T(8, lv1.lept_get_array_size(), __LINE__);
	for (i = 0; i < 8; i++)
		EXPECT_EQ_DOUBLE((double)i, lv1.lept_get_array_element(i).lept_get_number(), __LINE__);

	EXPECT_TRUE(lv1.lept_get_array_capacity() > 8, __LINE__);
	lv1.lept_shrink_array();
	EXPECT_EQ_SIZE_T(8, lv1.lept_get_array_capacity(), __LINE__);
	EXPECT_EQ_SIZE_T(8, lv1.lept_get_array_size(), __LINE__);
	for (i = 0; i < 8; i++)
		EXPECT_EQ_DOUBLE((double)i, lv1.lept_get_array_element(i).lept_get_number(), __LINE__);

	i = lv1.lept_get_array_capacity();
	lv1.lept_clear_array();
	EXPECT_EQ_SIZE_T(0, lv1.lept_get_array_size(), __LINE__);
	EXPECT_EQ_SIZE_T(i, lv1.lept_get_array_capacity(), __LINE__);
	lv1.lept_shrink_array();
	EXPECT_EQ_SIZE_T(0, lv1.lept_get_array_capacity(), __LINE__);
}

void test_access_object() {
#if 1
	lept_value object, mem_value, pv;
	size_t i, j, index;

	for (j = 0; j <= 5; j++) {
		object.lept_set_object({});
		EXPECT_EQ_SIZE_T(0, object.lept_get_object_size(), __LINE__);
		for (i = 0; i < 10; i++) {
			std::string key = "a";
			key[0] += i;
			mem_value.lept_set_number(i);
			object.lept_set_object_member(std::make_pair(key, mem_value));
			mem_value.clear();
		}
		EXPECT_EQ_SIZE_T(10, object.lept_get_object_size(), __LINE__);
		for (i = 0; i < 10; i++) {
			std::string key = "a";
			key[0] += i;
			index = object.lept_find_object_member_index(key);
			EXPECT_TRUE(index != LEPT_KEY_NOT_EXIST, __LINE__);
			pv = object.lept_get_object_member_value(index);
			EXPECT_EQ_DOUBLE((double)i, pv.lept_get_number(), __LINE__);
		}
	}

	index = object.lept_find_object_member_index("j");
	EXPECT_TRUE(index != LEPT_KEY_NOT_EXIST, __LINE__);
	object.lept_remove_object_member(index);
	index = object.lept_find_object_member_index("j");
	EXPECT_TRUE(index == LEPT_KEY_NOT_EXIST, __LINE__);
	EXPECT_EQ_SIZE_T(9, object.lept_get_object_size(), __LINE__);

	index = object.lept_find_object_member_index("a");
	EXPECT_TRUE(index != LEPT_KEY_NOT_EXIST, __LINE__);
	object.lept_remove_object_member(index);
	index = object.lept_find_object_member_index("a");
	EXPECT_TRUE(index == LEPT_KEY_NOT_EXIST, __LINE__);
	EXPECT_EQ_SIZE_T(8, object.lept_get_object_size(), __LINE__);

	object.lept_shrink_object();
	EXPECT_EQ_SIZE_T(8, object.lept_get_object_size(), __LINE__);
	for (i = 0; i < 8; i++) {
		std::string key = "a";
		key[0] += i + 1;
		EXPECT_EQ_DOUBLE((double)i + 1, object.lept_get_object_member_value(object.lept_find_object_member_index(key)).lept_get_number(), __LINE__);
	}

	i = object.lept_get_object_capacity();
	object.lept_clear_object();
	EXPECT_EQ_SIZE_T(0, object.lept_get_object_size(), __LINE__);
	EXPECT_EQ_SIZE_T(i, object.lept_get_object_capacity(), __LINE__);
	object.lept_shrink_object();
	EXPECT_EQ_SIZE_T(0, object.lept_get_object_capacity(), __LINE__);
#endif
}

void test_access() {
	test_access_null();
	test_access_boolean();
	test_access_number();
	test_access_string();
	test_access_array();
	test_access_object();
}

int main() {
//#ifdef WIN32
//	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//#endif
	test_parse();
	test_stringify();
	test_equal();
	test_copy();
	test_move();
	test_swap();
	test_access();
	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
	return main_ret;
}