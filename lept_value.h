#pragma once

#include <iostream>
#include <cstring>
#include <cassert>
#include <vector>
#include <map>

enum class lept_type {
	LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT
};

enum class Expect {
	LEPT_PARSE_OK = 0,
	LEPT_PARSE_EXPECT_VALUE,
	LEPT_PARSE_INVALID_VALUE,
	LEPT_PARSE_ROOT_NOT_SINGULAR,
	LEPT_PARSE_NUMBER_TOO_BIG,
	LEPT_PARSE_INVALID_STRING_CHAR,
	LEPT_PARSE_MISS_QUOTATION,
	LEPT_PARSE_MISS_QUOTATION_MARK,
	LEPT_PARSE_INVALID_STRING_ESCAPE,
	LEPT_PARSE_INVALID_UNICODE_HEX,
	LEPT_PARSE_INVALID_UNICODE_SURROGATE,
	LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
	LEPT_PARSE_MISS_KEY,
	LEPT_PARSE_MISS_COLON,
	LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET
};

template <typename Enumeration>
auto as_integer(Enumeration const value) -> typename std::underlying_type<Enumeration>::type
{
	return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

std::ostream& operator<<(std::ostream& out, const lept_type& lt) {
	out << as_integer(lt);
	return out;
}

std::ostream& operator<<(std::ostream& out, const Expect& lt) {
	out << as_integer(lt);
	return out;
}

using std::string;
const size_t LEPT_KEY_NOT_EXIST = ((size_t)-1); //size_t??unsigned ??-1???????unsigned?????????????????


class lept_value {
private:
	std::vector<lept_value> arr;
	std::vector<std::pair<std::string, lept_value>> object;
	std::string str;
	double number;
	lept_type type;
public:
	lept_value() : str(""), number(0), type(lept_type::LEPT_NULL) { };

	void set_type(lept_type type) {
		this->type = type;
	}
	void set_number(double number) {
		this->number = number;
	}
	bool lept_get_boolean() {
		assert(this->type == lept_type::LEPT_TRUE || this->type == lept_type::LEPT_FALSE);
		return (this->type == lept_type::LEPT_TRUE) ? true : false;
	}
	void lept_set_boolean(bool b) {
		this->type = b ? lept_type::LEPT_TRUE : lept_type::LEPT_FALSE;
	}

	void lept_set_null();
	lept_type lept_get_type() const;
	void lept_set_number(double n);
	double lept_get_number() const;

	void lept_set_string(const string& str);
	void lept_set_array(const std::vector<lept_value>& arr);
	void lept_set_array_capacity(const std::vector < lept_value>& arr, size_t capacity);
	size_t lept_get_array_capacity() const;
	size_t lept_get_array_size() const;
	lept_value lept_get_array_element(size_t index) const;
	void lept_push_array_element(const lept_value& element);
	void lept_pop_array_element();
	void lept_erase_array_element(size_t start, size_t count);
	void lept_insert_array_element(const lept_value& e, size_t index);
	void lept_shrink_array();
	void lept_clear_array();

	string lept_get_string() const;
	size_t lept_get_string_length() const;

	size_t lept_get_object_size() const;
	void lept_set_object(const std::vector<std::pair<std::string, lept_value>>& object_arr);
	std::string lept_get_object_key(size_t index) const;
	size_t lept_get_object_key_length(size_t index) const;
	lept_value lept_get_object_member_value(size_t index) const;
	size_t lept_find_object_member_index(const std::string& key) const;
	void lept_set_object_member(std::pair<std::string, lept_value> member);;
	void lept_remove_object_member(size_t index);
	void lept_shrink_object();
	size_t lept_get_object_capacity() const;
	void lept_clear_object();

	void clear();
};

void lept_value::lept_set_number(double n) {
	set_number(n);
	this->type = lept_type::LEPT_NUMBER;
	this->str.clear();
}

void lept_value::lept_set_null() {
	this->str.clear();
	this->arr.clear();
	this->number = 0.0;
	this->object.clear();
	this->type = lept_type::LEPT_NULL;
}

lept_type lept_value::lept_get_type() const {
	return this->type;
}

double lept_value::lept_get_number() const {
	assert(this->type == lept_type::LEPT_NUMBER);
	return this->number;
}

void lept_value::lept_set_string(const string& str) {
	this->str = str;
	this->type = lept_type::LEPT_STRING;
	this->number = 0.0;
}

string lept_value::lept_get_string() const {
	assert(this->type == lept_type::LEPT_STRING);
	return this->str;
}

size_t lept_value::lept_get_string_length() const {
	assert(this->type == lept_type::LEPT_STRING);
	return this->str.size();
}

size_t lept_value::lept_get_array_size() const {
	assert(this->type == lept_type::LEPT_ARRAY);
	return this->arr.size();
}

size_t lept_value::lept_get_array_capacity() const {
	assert(this->type == lept_type::LEPT_ARRAY);
	return this->arr.capacity();
}

void lept_value::lept_set_array(const std::vector<lept_value>& arr) {
	this->arr = arr;
	this->type = lept_type::LEPT_ARRAY;
	this->number = 0.0;
	this->str = "";
}

void lept_value::lept_set_array_capacity(const std::vector<lept_value>& arr, size_t capacity) {
	lept_set_array(arr);
	this->arr.reserve(capacity);
}

lept_value lept_value::lept_get_array_element(size_t index) const {
	assert(this->type == lept_type::LEPT_ARRAY);
	assert(index < this->arr.size());
	return this->arr[index];
}

void lept_value::lept_push_array_element(const lept_value& element) {
	assert(this->type == lept_type::LEPT_ARRAY);
	this->arr.push_back(element);
}

void lept_value::lept_pop_array_element() {
	assert(this->type == lept_type::LEPT_ARRAY);
	this->arr.pop_back();
}

void lept_value::lept_insert_array_element(const lept_value& e, size_t index) {
	assert(this->type == lept_type::LEPT_ARRAY && index <= this->arr.size());
	auto it = this->arr.begin() + index;
	this->arr.insert(it, e);
}

void lept_value::lept_erase_array_element(size_t start, size_t count) {
	assert(this->type == lept_type::LEPT_ARRAY);
	size_t num = 0;
	auto it = this->arr.begin() + start;
	while (num < count) {
		it = this->arr.erase(it);
		++num;
	}
}

void lept_value::lept_shrink_array() {
	assert(this->type == lept_type::LEPT_ARRAY);
	this->arr.shrink_to_fit();
}

void lept_value::lept_clear_array() {
	assert(this->type == lept_type::LEPT_ARRAY);
	this->arr.clear();
}

size_t lept_value::lept_get_object_size() const {
	assert(this->type == lept_type::LEPT_OBJECT);
	return this->object.size();
}

void lept_value::lept_set_object(const std::vector<std::pair<std::string, lept_value>>& object_arr) {
	this->type = lept_type::LEPT_OBJECT;
	this->object = object_arr;
}

std::string lept_value::lept_get_object_key(size_t index) const {
	assert(this->type == lept_type::LEPT_OBJECT);
	return this->object[index].first;
}

size_t lept_value::lept_get_object_key_length(size_t index) const {
	assert(this->type == lept_type::LEPT_OBJECT);
	return this->object[index].first.size();
}

lept_value lept_value::lept_get_object_member_value(size_t index) const {
	assert(this->type == lept_type::LEPT_OBJECT);
	return this->object[index].second;
}

size_t lept_value::lept_find_object_member_index(const std::string& key) const {
	assert(this->type == lept_type::LEPT_OBJECT);
	size_t key_size = key.size();
	for (size_t i = 0; i < this->object.size(); i++) {
		if (this->object[i].first.size() == key_size && memcmp(this->object[i].first.data(), key.data(), key_size) == 0) {
			return i;
		}
	}
	return LEPT_KEY_NOT_EXIST;
}

void lept_value::lept_set_object_member(std::pair<std::string, lept_value> member) {
	assert(this->type == lept_type::LEPT_OBJECT);
	this->object.push_back(member);
}

void lept_value::lept_remove_object_member(size_t index) {
	assert(this->type == lept_type::LEPT_OBJECT);
	this->object.erase(this->object.begin() + index);
}

void lept_value::lept_shrink_object() {
	assert(this->type == lept_type::LEPT_OBJECT);
	this->object.shrink_to_fit();
}

size_t lept_value::lept_get_object_capacity() const {
	assert(this->type == lept_type::LEPT_OBJECT);
	return this->object.capacity();
}

void lept_value::lept_clear_object() {
	assert(this->type == lept_type::LEPT_OBJECT);
	this->object.clear();
}

void lept_value::clear() {
	this->arr.clear();
	this->str.clear();
	this->number = 0.0;
	this->object.clear();
	this->type = lept_type::LEPT_NULL;
}






