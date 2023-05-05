#pragma once

#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#include <cstdbg.h>
#endif

#include <cstring>
#include <cassert>
#include <cmath>
#include <codecvt>
#include <sstream>
#include <iomanip>

#include "lept_value.h"

class lept_json {
private:
	const char* json;
private:
	void lept_parse_whitespace();

	Expect lept_parse_value(lept_value& v);
	Expect lept_parse_literal(lept_value& v, const char* literal, lept_type type);
	Expect lept_parse_number(lept_value& v);
	Expect lept_parse_string_raw(std::string& str);
	Expect lept_parse_string(lept_value& v);
	Expect lept_parse_array(lept_value& v);
	Expect lept_parse_object(lept_value& v);

	const char* lept_parse_hex4(unsigned& u);
	void lept_encode_utf8(std::string& strBuf, unsigned u);

	bool check_next_char() {
		if (*json != ' ' && *json != '\t' && *json != '\n' && *json != '\r' && *json != '\0') {
			return false;
		}
		return true;
	}
	void EXPECT(char ch) {
		assert(*json == ch);
		++json;
	}
	bool ISDIGIT(char ch) {
		if ((ch) >= '0' && (ch) <= '9')
			return true;
		else
			return false;
	}
	bool ISDIGIT1TO9(char ch) {
		if ((ch) >= '1' && (ch) <= '9')
			return true;
		else
			return false;
	}
	Expect STRING_ERROR(Expect error) {
		return error;
	}

	std::string hex2str(unsigned short code) const;
	void lept_stringify_string(std::string& json_str, const std::string& s) const;

public:
	lept_json(const char* js) : json(js) {};
	lept_json(const lept_json&) = delete;
	lept_json(const lept_json&&) = delete;
	Expect lept_parse(lept_value& v);
	std::string lept_stringify(const lept_value& v) const;
	void lept_copy(lept_value& dst, const lept_value& src) const;
	void lept_move(lept_value& dst, lept_value& src) const;
	void lept_swap(lept_value& dst, lept_value& src) const;
	void reset_json(const char* json) {
		this->json = json;
	}
};

Expect lept_json::lept_parse_literal(lept_value& v, const char* ch, lept_type type) {
	assert(ch != nullptr);
	EXPECT(*ch);
	++ch;
	while ((*ch) != '\0') {
		if (*(this->json) != *ch) {
			return Expect::LEPT_PARSE_INVALID_VALUE;
		}
		++this->json;
		++ch;
	}
	v.set_type(type);
	return Expect::LEPT_PARSE_OK;
}

Expect lept_json::lept_parse_number(lept_value& v) {
	const char* p = json;
	if (*p == '-')p++;
	if (*p == '0')p++;
	else {
		if (!ISDIGIT1TO9(*p))return Expect::LEPT_PARSE_INVALID_VALUE;
		for (p++; ISDIGIT(*p); p++);
	}

	if (*p == '.') {
		p++;
		if (!ISDIGIT(*p)) return Expect::LEPT_PARSE_INVALID_VALUE;
		for (p++; ISDIGIT(*p); p++);
	}
	if (*p == 'e' || *p == 'E') {
		p++;
		if (*p == '+' || *p == '-') p++;
		if (!ISDIGIT(*p)) return Expect::LEPT_PARSE_INVALID_VALUE;
		for (p++; ISDIGIT(*p); p++);
	}

	double val = strtod(this->json, nullptr);

	this->json = p;

	if (val == HUGE_VALF || val == HUGE_VAL || val == HUGE_VALL || val == -INFINITY) {
		return Expect::LEPT_PARSE_NUMBER_TOO_BIG;
	}
	v.lept_set_number(val);
	return Expect::LEPT_PARSE_OK;
}

Expect lept_json::lept_parse_string(lept_value& v) {
	std::string str;
	auto ret = lept_parse_string_raw(str);
	if (ret == Expect::LEPT_PARSE_OK) {
		v.lept_set_string(str);
	}
	return ret;
}

Expect lept_json::lept_parse_string_raw(std::string& str) {
	std::string str_buf;
	EXPECT('\"');
	char next_ch = 0;
	unsigned u = 0;
	unsigned codePoint = 0;
	while (1) {
		char ch = *json++;
		switch (ch) {
		case '\"':
			str = str_buf;
			str_buf.clear();
			return Expect::LEPT_PARSE_OK;
		case '\0':
			str_buf.clear();
			return Expect::LEPT_PARSE_MISS_QUOTATION_MARK;
		case '\\':
			next_ch = *json++;
			switch (next_ch) {
			case '\"': str_buf.push_back('\"'); break;
			case '/':  str_buf.push_back('/');  break;
			case '\\': str_buf.push_back('\\'); break;
			case 'b':  str_buf.push_back('\b'); break;
			case 'f':  str_buf.push_back('\f'); break;
			case 'n':  str_buf.push_back('\n'); break;
			case 'r':  str_buf.push_back('\r'); break;
			case 't':  str_buf.push_back('\t'); break;
			case 'u':
				if (!(json = lept_parse_hex4(u)))
					return STRING_ERROR(Expect::LEPT_PARSE_INVALID_UNICODE_HEX);
				codePoint = u;
				if (u >= 0xD800 && u <= 0xDBFF) { 
					codePoint = 0x10000 + (u - 0xD800) * 0x400;
					if (*json++ == '\\') {
						if (*json++ == 'u') {
							if (!(json = lept_parse_hex4(u)))
								return STRING_ERROR(Expect::LEPT_PARSE_INVALID_UNICODE_HEX);
							if (u >= 0xDC00 && u <= 0xDFFF) { 
								codePoint += (u - 0xDC00);
							}
							else {
								return STRING_ERROR(Expect::LEPT_PARSE_INVALID_UNICODE_SURROGATE);
							}
						}
						else {
							return STRING_ERROR(Expect::LEPT_PARSE_INVALID_UNICODE_SURROGATE);
						}
					}
					else {
						return STRING_ERROR(Expect::LEPT_PARSE_INVALID_UNICODE_SURROGATE);
					}
				}
				u = codePoint;
				lept_encode_utf8(str_buf, u);
				break;
			default:
				return Expect::LEPT_PARSE_INVALID_STRING_ESCAPE;
				break;
			}
			break;
		default:
			if (ch >= 0 && ch <= 31) {
				return Expect::LEPT_PARSE_INVALID_STRING_CHAR;
			}
			str_buf.push_back(ch);
		}
	}
	return Expect();
}

void lept_json::lept_parse_whitespace() {
	while (*json == ' ' || *json == '\t' || *json == '\n' || *json == '\r')
		json++;
}

Expect lept_json::lept_parse_value(lept_value& v) {
	switch (*this->json) {
	case 'n': return lept_parse_literal(v, "null", lept_type::LEPT_NULL);
	case 't': return lept_parse_literal(v, "true", lept_type::LEPT_TRUE);
	case 'f': return lept_parse_literal(v, "false", lept_type::LEPT_FALSE);
	case '\"': return lept_parse_string(v);
	case '[': return lept_parse_array(v);
	case '{': return lept_parse_object(v);
	case '\0': return Expect::LEPT_PARSE_EXPECT_VALUE;
	default:
		return lept_parse_number(v);
		break;
	}
}

Expect lept_json::lept_parse(lept_value& v) {
	v.lept_set_null();

	lept_parse_whitespace();

	auto ret = lept_parse_value(v);
	if (ret == Expect::LEPT_PARSE_OK) {
		lept_parse_whitespace();
		if (*this->json != '\0') {
			v.lept_set_null();
			ret = Expect::LEPT_PARSE_ROOT_NOT_SINGULAR;
			//若 json 在一个值之后，空白之后还有其它字符，则要返回 LEPT_PARSE_ROOT_NOT_SINGULAR
		}
	}
	return ret;
}

const char* lept_json::lept_parse_hex4(unsigned& u) {
	u = 0;

	for (int i = 0; i < 4; i++) {
		char ch = *json++;
		u <<= 4;
		if (ch >= '0' && ch <= '9') u |= ch - '0';
		else if (ch >= 'A' && ch <= 'F') u |= ch - ('A' - 10);
		else if (ch >= 'a' && ch <= 'f') u |= ch - ('a' - 10);
		else return NULL;
	}
	return json;
}

void lept_json::lept_encode_utf8(std::string& str_buf, unsigned u) {
	if (u <= 0x7F)
        str_buf += static_cast<char> (u & 0xFF);
    else if (u <= 0x7FF) {
        str_buf += static_cast<char> (0xC0 | ((u >> 6) & 0xFF)); /* 0xC0 = 11000000 */
        str_buf += static_cast<char> (0x80 | ( u	   & 0x3F));
	}
    else if (u <= 0xFFFF) {
        str_buf += static_cast<char> (0xE0 | ((u >> 12) & 0xFF)); /* 0xE0 = 11100000 */
        str_buf += static_cast<char> (0x80 | ((u >>  6) & 0x3F)); /* 0x80 = 10000000 */
        str_buf += static_cast<char> (0x80 | ( u        & 0x3F)); /* 0x3F = 00111111 */
    }
    else {
        assert(u <= 0x10FFFF);
        str_buf += static_cast<char> (0xF0 | ((u >> 18) & 0xFF)); /* 0xF0 = 11110000 */
        str_buf += static_cast<char> (0x80 | ((u >> 12) & 0x3F));
        str_buf += static_cast<char> (0x80 | ((u >>  6) & 0x3F));
        str_buf += static_cast<char> (0x80 | ( u        & 0x3F));
	}
}

Expect lept_json::lept_parse_array(lept_value& v) {
	EXPECT('[');
	std::vector<lept_value> arr_buf;
	lept_parse_whitespace();
	if (*json == ']') {
		++json;
		v.lept_set_array({});
		return Expect::LEPT_PARSE_OK;
	}
	while (1) {
		lept_parse_whitespace();
		lept_value temp_v;
		auto ret = lept_parse_value(temp_v);
		if (ret != Expect::LEPT_PARSE_OK) {
			return ret;
		}
		arr_buf.push_back(temp_v);
		lept_parse_whitespace();
		if (*json == ',')
			json++;
		else if (*json == ']') {
			json++;
			v.lept_set_array(arr_buf);
			return Expect::LEPT_PARSE_OK;
		}
		else {
			return Expect::LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
		}
	}
}

Expect lept_json::lept_parse_object(lept_value& v) {
	EXPECT('{');
	std::vector<std::pair<std::string, lept_value>> object_buf;
	lept_parse_whitespace();
	if (*json == '}') {
		++json;
		v.lept_set_object({});
		return Expect::LEPT_PARSE_OK;
	}
	while (1) {
		std::pair<std::string, lept_value> member;
		lept_parse_whitespace();
		if (*json != '\"') {
			return Expect::LEPT_PARSE_MISS_KEY;
		}
		std::string m_key;
		lept_value m_value;
		auto ret = lept_parse_string_raw(m_key);
		if (ret != Expect::LEPT_PARSE_OK) {
			return ret;
		}
		lept_parse_whitespace();
		if (*json != ':') {
			return Expect::LEPT_PARSE_MISS_COLON;
		}
		++json;
		lept_parse_whitespace();
		ret = lept_parse_value(m_value);
		if (ret != Expect::LEPT_PARSE_OK) {
			return ret;
		}
		member = std::make_pair(m_key, m_value);
		object_buf.push_back(member);
		lept_parse_whitespace();

		if (*json == ',')
			json++;
		else if (*json == '}') {
			json++;
			v.lept_set_object(object_buf);
			return Expect::LEPT_PARSE_OK;
		}
		else {
			return Expect::LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
		}
	}
}

std::string lept_json::hex2str(unsigned short code) const {
	std::string str = "++++";
	for (int i = 3; i >= 0; --i, code >>= 4) {
		if ((code & 0xf) <= 9)
			str[i] = (code & 0xf) + '0';
		else
			str[i] = (code & 0xf) + 'A' - 0x0a;
	}
	return str;
}

void lept_json::lept_stringify_string(std::string& json_str, const std::string& s) const {
	json_str += '\"';
    for(auto it = s.begin(); it < s.end(); ++it) {
        unsigned char ch = *it;
        switch (ch) {
            case '\"': json_str += "\\\""; break;
            case '\\': json_str += "\\\\"; break;
            case '\b': json_str += "\\b";  break;
            case '\f': json_str += "\\f";  break;
            case '\n': json_str += "\\n";  break;
            case '\r': json_str += "\\r";  break;
            case '\t': json_str += "\\t";  break;
            default:
                if (ch < 0x20) {
                    char buffer[7] = {0};
                    sprintf(buffer, "\\u%04X", ch);
                    json_str += buffer;
                }
                else
                    json_str += *it;
        }
    }
    json_str += '\"';
}

std::string lept_json::lept_stringify(const lept_value& v) const {
	size_t i;
	std::string stringify_json;
	switch (v.lept_get_type()) {
	case lept_type::LEPT_NULL:  stringify_json = "null";  break;
	case lept_type::LEPT_FALSE: stringify_json = "false"; break;
	case lept_type::LEPT_TRUE:  stringify_json = "true";  break;
	case lept_type::LEPT_NUMBER: {
		std::stringstream ss;
		ss << std::setprecision(17) << v.lept_get_number();
		stringify_json = ss.str();
		break;
	}
	case lept_type::LEPT_STRING: {
		lept_stringify_string(stringify_json, v.lept_get_string());
		break;
	}
	case lept_type::LEPT_ARRAY:
		stringify_json.push_back('[');
		for (i = 0; i < v.lept_get_array_size(); i++) {
			if (i > 0) {
				stringify_json.push_back(',');
			}
			stringify_json += lept_stringify(v.lept_get_array_element(i));
		}
		stringify_json.push_back(']');
		break;
	case lept_type::LEPT_OBJECT:
		stringify_json.push_back('{');
		for (i = 0; i < v.lept_get_object_size(); ++i) {
			if (i > 0)
				stringify_json.push_back(',');
			lept_stringify_string(stringify_json, v.lept_get_object_key(i));
			stringify_json.push_back(':');
			stringify_json += lept_stringify(v.lept_get_object_member_value(i));
		}
		stringify_json.push_back('}');
		break;
	default: assert(0 && "invalid type");
	}
	return stringify_json;
}

void lept_json::lept_copy(lept_value& dst, const lept_value& src) const {
	dst = src;
}

void lept_json::lept_move(lept_value& dst, lept_value& src) const {
	dst = std::move(src);
	src.clear();
}

void lept_json::lept_swap(lept_value& dst, lept_value& src) const {
	lept_value temp = src;
	src = dst;
	dst = temp;
}