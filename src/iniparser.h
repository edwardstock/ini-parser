/**
 * iniparser
 * iniparser.h.h
 *
 * @author Eduard Maximovich <edward.vstock@gmail.com>
 * @link https://github.com/edwardstock
 */

#ifndef INIPARSER_INIPARSER_H_H
#define INIPARSER_INIPARSER_H_H

#include <string>
#include <ostream>
#include <istream>
#include <sstream>
#include <unordered_map>
#include <utility>
#include <vector>
#include <regex>
#include <fstream>
#include <toolboxpp.h>

namespace INIConfig {

class Value {
 public:
    Value(std::string &value);
    Value(const char *value);
    Value();
    ~Value();

    std::string get() const;
    int getInt() const;
    long getLong() const;
    double getReal() const;
    bool getBool() const;

    bool operator==(const Value &another);
    bool operator==(const std::string &another);
    bool operator==(long another);
    bool operator==(double another);
    bool operator==(int another);
    bool operator==(bool another);
    friend std::ostream &operator<<(std::ostream &os, const Value &val) {
        os << val.get();
        return os;
    }

 private:
    std::string value;
    std::string newValue;
};

class Row {
 public:
    Row(const std::string &key, Value &&value, u_long line);
    Row(const std::string &key, u_long line);
    // move
    Row(Row &&another) noexcept;
    // copy
    Row(const Row &another);

    Row &operator=(Row &&another) noexcept;
    Row &operator=(const Row &another);
    bool operator>(const Row &another) const;
    bool operator<(const Row &another) const;

    ~Row();

    void setIsCommented(bool isCommented);

    bool isCommented() const;
    bool isArray() const;

    void addValue(Value &&val);
    void addValue(const Value &val);

    std::string getKey() const;
    u_long getLine() const;
    Value getValue() const;
    std::vector<Value> getValues() const;

 private:
    u_long line = 1;
    std::string key;
    std::vector<Value> value;
    bool commented = false;

    void destroy() {
    }
};

class Section {
 public:
    explicit Section(const std::string &n);
    ~Section();

    std::string getName() const;
    std::vector<Row> getRows() const;
    void addRow(Row &&row);
    bool hasRowKey(const Row &row) const;
    bool hasRowKey(const std::string &key) const;
    bool hasRow(const Row &row) const;
    bool hasRow(const std::string &key);

    bool operator==(const Section &section);
    bool operator==(const std::string &name);
    Row *operator[](const char *key);
    Row *operator[](const std::string &key);

    Row *getRow(const Row &anotherRow);
    Row *getRow(const std::string &key);

 private:
    std::string name;
    std::vector<Row> rows;
};

class Parser {

 private:
    std::unordered_map<std::string, Section *> sections;
    std::unordered_map<std::string, Row *> rows;

    /**
     * group 0: full match, 1: section
     */
    const static std::string P_SECTION;
    /**
     * (?:\;|\#)          - optional: comment before line (; or #)
     * ([a-z0-9_\-\+\.]+) - required: key
     * ?(?:\[\])?         - optional: square braces
     * (?:[ ]*)           - optional: space char[0,] before "="
     * \=                 - required: equal mark
     * (?:[ ]*)           - optional: space char[0,] after "="
     * (.*)               - required: value
     *
     * Matches:
     * group 0: full match, 1: key, 2: value
     */
    const static std::string P_ROW;
    const static std::string P_COMMENT;
    const static std::string P_ARR_VALUE;
 public:
    Parser(const std::string &file, bool withSections = true);

    void parse(const std::string &file);

    bool hasSection(const std::string &name) const;
    bool hasSection(const Section *section) const;

    Row *getRow(const std::string &section, const std::string &key) const;
    Row *getRow(const std::string &name) const;
    Section *getSection(const std::string &name) const;

    Value getValue(const std::string &section, const std::string &key) const;
    Value getValue(const std::string &name) const;

    void dump(std::ostream &out);
    void dump();
};

}

#endif //INIPARSER_INIPARSER_H_H
