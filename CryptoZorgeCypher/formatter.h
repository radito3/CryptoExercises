#include <tuple>
#include <vector>
#include <cctype>
#include <cstring>
#include <unordered_set>

enum format_type { INSERT_BEFORE, REPLACE, ERASE, SKIP };

class formatting_rule {
public:
    virtual ~formatting_rule() = default;

    [[nodiscard]] virtual std::tuple<format_type, char> apply(char prev, char ch) const = 0;
};

class letter_before_number : public formatting_rule {
public:
    [[nodiscard]] std::tuple<format_type, char> apply(char prev, char ch) const override {
        if (prev == '\0') {
            return { SKIP, ch };
        }
        if (isalpha(prev) && isdigit(ch)) {
            return { INSERT_BEFORE, '/' };
        }
        return {SKIP, ch };
    }
};

class number_before_letter : public formatting_rule {
public:
    [[nodiscard]] std::tuple<format_type, char> apply(char prev, char ch) const override {
        if (prev == '\0') {
            return { SKIP, ch };
        }
        if (isdigit(prev) && isalpha(ch)) {
            return { INSERT_BEFORE, '/' };
        }
        return {SKIP, ch };
    }
};

class to_upper : public formatting_rule {
public:
    [[nodiscard]] std::tuple<format_type, char> apply(char prev, char ch) const override {
        if (isalpha(ch) && !isupper(ch)) {
            return { REPLACE, toupper(ch) };
        }
        return { SKIP, ch };
    }
};

class remove_illegal_symbols : public formatting_rule {
    const std::unordered_set<char> allowed_symbols;

public:
    explicit remove_illegal_symbols(const char allowed_symbols[])
            : allowed_symbols(allowed_symbols, allowed_symbols + strlen(allowed_symbols)) {}

    [[nodiscard]] std::tuple<format_type, char> apply(char prev, char ch) const override {
        if (allowed_symbols.find(ch) != allowed_symbols.end() || isdigit(ch)) {
            return { SKIP, ch };
        }
        return { ERASE, ch };
    }
};

class formatter {
    std::vector<formatting_rule*> rules;

public:
    formatter() = default;

    formatter(const std::initializer_list<formatting_rule*>& rules) : rules(rules) {}

    ~formatter() {
        for (auto rule : rules) {
            delete rule;
        }
    }

    [[nodiscard]] std::string format_text(const std::string& text) const {
        std::string result(text);
        size_t text_size = text.size();

        for (size_t char_index = 0; char_index < text_size; ++char_index) {
            for (size_t rule_index = 0; rule_index < rules.size(); ++rule_index) {
                char previous = char_index == 0 ? '\0' : result[char_index - 1];
                auto [fmt_type, formatted_ch] = rules[rule_index]->apply(previous, result[char_index]);

                switch (fmt_type) {
                    case SKIP:
                        break;
                    case REPLACE:
                        result[char_index] = formatted_ch;
                        break;
                    case ERASE:
                        result.erase(char_index, 1);
                        --text_size;
                        if (char_index == text_size) {
                            return result;
                        }
                        rule_index = -1; //restart rules loop
                        break;
                    case INSERT_BEFORE:
                        result.reserve(1);
                        result.insert(result.begin() + char_index, formatted_ch);
                        ++text_size;
                        ++char_index;
                        break;
                }
            }
        }
        return result;
    }
};
