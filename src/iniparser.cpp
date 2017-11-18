#include "../include/iniparser.h"

const std::string INIConfig::Parser::P_SECTION = R"(\[{1,1}(.+)\]{1,1})";
const std::string INIConfig::Parser::P_ROW = R"((?:\;|\#)*([a-z0-9_\-\+\.]+)?(?:\[\])?(?:[ ]*)\=(?:[ ]*)(.*))";
const std::string INIConfig::Parser::P_COMMENT = R"((?:\;|\#))";
const std::string INIConfig::Parser::P_ARR_VALUE = R"((\[\]))";

void INIConfig::Parser::parse(const std::string &file) {
    const std::regex sectionReg(P_SECTION);
    const std::regex rowReg(P_ROW);

    std::ifstream input(file);
    if (!input.is_open() || input.bad()) {
        perror("Can't open file");
        return;
    }
    std::string row, lastSection;
    u_long line = 1;

    while (std::getline(input, row)) {
        if (row.empty()) {
            line++;
            continue;
        }

        // searching for section pattern in row
        std::string mSection = toolboxpp::strings::matchRegexpFirst(sectionReg, row);

        if (!mSection.empty() && row.c_str()[0] == ';') {
            line++;
            continue;
        }

        // searching for key([])=value pattern in row
        std::smatch mRow = toolboxpp::strings::matchRegexp(rowReg, row);

        size_t rsz = mRow.size();
        bool rem = mRow.empty();

        // if is not a section and key=value detected and size of matches is correct, writing value to section
        if (mSection.empty() && !mRow.empty() && mRow.size() == 3) {
            std::string key, value;
            key = mRow[1];
            value = mRow[2];

            // if no one section were found, we create hidden __default__ section for consistency
            if (lastSection.empty()) {
                lastSection = "__default__";
                sections[lastSection] = new Section(lastSection);
            }

            Row iniRow(key, Value(value), line);
            iniRow.setIsCommented(toolboxpp::strings::hasRegex(P_COMMENT, row));
            if (sections[lastSection]->hasRowKey(iniRow) && toolboxpp::strings::hasRegex(P_ARR_VALUE, row)) {
                // We found array. Why? In current section already added a value with the same key
                sections[lastSection]
                    ->getRow(iniRow)
                    ->addValue(Value(value));

                rows[key] = sections[lastSection]->getRow(iniRow);
            } else {
                sections[lastSection]
                    ->addRow(std::move(iniRow));

                rows[key] = sections[lastSection]->getRow(key);
            }

        } else if (!mSection.empty()) {
            lastSection = mSection.empty() ? "__default__" : mSection;
            sections[lastSection] = new Section(lastSection);
        }

        line++;
    }

}
INIConfig::Parser::Parser(const std::string &file, bool withSections) {
    parse(file);
}
bool INIConfig::Parser::hasSection(const std::string &name) const {
    return sections.count(name) > 0;
}
bool INIConfig::Parser::hasSection(const INIConfig::Section *section) const {
    return hasSection(section->getName());
}
INIConfig::Row *INIConfig::Parser::getRow(const std::string &section, const std::string &key) const {
    if (!hasSection(section)) {
        return nullptr;
    }

    return getSection(section)->getRow(key);
}
INIConfig::Row *INIConfig::Parser::getRow(const std::string &name) const {
    if (rows.count(name) == 0) {
        return nullptr;
    }

    return rows.at(name);
}
INIConfig::Section *INIConfig::Parser::getSection(const std::string &name) const {
    if (!hasSection(name)) {
        return nullptr;
    }

    return sections.at(name);
}
INIConfig::Value INIConfig::Parser::getValue(const std::string &section, const std::string &key) const {
    if (getRow(section, key) == nullptr) return nullptr;

    return getRow(section, key)->getValue();
}
INIConfig::Value INIConfig::Parser::getValue(const std::string &name) const {
    if (getRow(name) == nullptr) {
        return nullptr;
    }

    return getRow(name)->getValue();
}

INIConfig::Value INIConfig::Parser::getValue(const std::string &name, std::string &&defaultValue) const {
    if (getRow(name) == nullptr) {
        return Value(std::move(defaultValue));
    }

    return getRow(name)->getValue();

}
void INIConfig::Parser::dump(std::ostream &out) {
    for (auto &s: sections) {
        out << "[" << s.second->getName() << "]" << std::endl;
        for (auto &r: s.second->getRows()) {
            out << "  ";
            for (auto &v: r.getValues()) {
                if (r.isCommented()) out << ";";
                out << r.getKey();
                if (r.isArray()) {
                    out << "[]=";
                } else {
                    out << "=";
                }

                out << v.get() << std::endl;
            }
        }
    }
}
void INIConfig::Parser::dump() {
    dump(std::cout);
}
INIConfig::Row::Row(const std::string &key, INIConfig::Value &&value, u_long line)
    : Row(key, line) {
    addValue(value);
}
INIConfig::Row::Row(const std::string &key, u_long line)
    : key(key), line(line) {
}
INIConfig::Row::Row(INIConfig::Row &&another) noexcept {
    *this = std::move(another);
}
INIConfig::Row::Row(const INIConfig::Row &another) {
    *this = another;
}
INIConfig::Row &INIConfig::Row::operator=(INIConfig::Row &&another) noexcept {
    key = std::move(another.key);
    value = std::move(another.value);
    line = another.line;
    commented = another.commented;

    return *this;
}
INIConfig::Row &INIConfig::Row::operator=(const INIConfig::Row &another) {
    if (this == &another) {
        return *this;
    }

    key = another.key;
    line = another.line;
    commented = another.commented;

    value.clear();
    value.reserve(another.value.size());
    std::copy(another.value.begin(), another.value.end(), back_inserter(value));

    return *this;
}
bool INIConfig::Row::operator>(const INIConfig::Row &another) const {
    return line > another.line;
}
bool INIConfig::Row::operator<(const INIConfig::Row &another) const {
    return !(*this > another);
}
INIConfig::Row::~Row() {
    destroy();
}
void INIConfig::Row::setIsCommented(bool isCommented) {
    commented = isCommented;
}
bool INIConfig::Row::isCommented() const {
    return commented;
}
void INIConfig::Row::addValue(INIConfig::Value &&val) {
    value.push_back(val);
}
void INIConfig::Row::addValue(const INIConfig::Value &val) {
    value.push_back(val);
}
bool INIConfig::Row::isArray() const {
    return value.size() > 1;
}
std::string INIConfig::Row::getKey() const {
    return key;
}
u_long INIConfig::Row::getLine() const {
    return line;
}
INIConfig::Value INIConfig::Row::getValue() const {
    if (value.empty()) {
        return nullptr;
    }

    return value[0];
}
std::vector<INIConfig::Value> INIConfig::Row::getValues() const {
    return value;
}
INIConfig::Value::Value(std::string &&value)
    : value(value) {
}
INIConfig::Value::Value(std::string &value)
    : value(std::move(value)) {

}
INIConfig::Value::Value(const char *value)
    : value(std::string(value)) {

}
INIConfig::Value::Value()
    : value(std::string()) {
}
INIConfig::Value::~Value() {
}
std::string INIConfig::Value::get() const {
    return value;
}
int INIConfig::Value::getInt() const {
    int out;
    try {
        out = std::stoi(get());
    }
    catch (std::invalid_argument &e) {
        out = 0;
    }

    return out;
}
long INIConfig::Value::getLong() const {
    long out;
    try {
        out = std::stol(get());
    }
    catch (std::invalid_argument &e) {
        out = 0L;
    }

    return out;
}
double INIConfig::Value::getReal() const {
    double out;
    try {
        out = std::stod(get());
    }
    catch (std::invalid_argument &e) {
        out = 0;
    }

    return out;
}
bool INIConfig::Value::getBool() const {
    return get() == "1" || get() == "true";
}
bool INIConfig::Value::operator==(const INIConfig::Value &another) {
    return get() == another.get();
}
bool INIConfig::Value::operator==(long another) {
    return getLong() == another;
}
bool INIConfig::Value::operator==(double another) {
    return getReal() == another;
}
bool INIConfig::Value::operator==(bool another) {
    return getBool() == another;
}
bool INIConfig::Value::operator==(int another) {
    return getInt() == another;
}
bool INIConfig::Value::operator==(const std::string &s) {
    const std::string in = get();
    return std::equal(s.cbegin(), s.end(), in.cbegin(), in.cend());
}
INIConfig::Section::Section(const std::string &n)
    : name(std::move(n)) {
}
INIConfig::Section::~Section() {
    rows.clear();
}
std::string INIConfig::Section::getName() const {
    return name;
}
std::vector<INIConfig::Row> INIConfig::Section::getRows() const {
    return rows;
}
void INIConfig::Section::addRow(INIConfig::Row &&row) {
    rows.push_back(row);
}
bool INIConfig::Section::hasRowKey(const INIConfig::Row &row) const {
    for (auto &r: rows) {
        if (r.getKey() == row.getKey()) return true;
    }

    return false;
}
bool INIConfig::Section::hasRowKey(const std::string &key) const {
    for (auto &r: rows) {
        if (r.getKey() == key) return true;
    }

    return false;
}
bool INIConfig::Section::hasRow(const INIConfig::Row &row) const {
    for (auto &r: rows) {
        if (r.getKey() == row.getKey()) return true;
    }

    return false;
}
bool INIConfig::Section::hasRow(const std::string &key) {
    for (auto &r: rows) {
        if (r.getKey() == key) {
            return true;
        }
    }

    return false;
}
bool INIConfig::Section::operator==(const INIConfig::Section &section) {
    return getName() == section.getName();
}
bool INIConfig::Section::operator==(const std::string &name) {
    return getName() == name;
}
INIConfig::Row *INIConfig::Section::operator[](const char *key) {
    return (*this)[std::string(key)];
}
INIConfig::Row *INIConfig::Section::operator[](const std::string &key) {
    return getRow(key);
}
INIConfig::Row *INIConfig::Section::getRow(const INIConfig::Row &anotherRow) {
    return getRow(anotherRow.getKey());
}
INIConfig::Row *INIConfig::Section::getRow(const std::string &key) {
    for (auto &r: rows) {
        if (r.getKey() == key) {
            return &r;
        }
    }

    return nullptr;
}
