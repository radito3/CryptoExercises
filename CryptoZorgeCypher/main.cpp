#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <algorithm>
#include <iterator>
#include "formatter.h"

const char symbol_set[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ./";
constexpr const int DISPLAY_BATCH_SIZE = 5;
constexpr const int STARTING_SYMBOL_INDEX = 80;
constexpr const int REQUIRED_KEY_LEN = 6;

inline bool has_repeating_chars(const std::string& string) {
    int hash[256] = { 0 };
    for (char ch : string) {
        if (hash[ch] == 0) {
            hash[ch]++;
        } else if (hash[ch] == 1) {
            return true;
        }
    }
    return false;
}

class encryptor {
    std::unordered_map<char, int> most_frequent_symbols;
    const std::string allowed_symbols;

    void validate_key(const std::string& key) const {
        if (key.size() != REQUIRED_KEY_LEN) {
            throw std::logic_error("Invalid key size");
        }
        if (has_repeating_chars(key)) {
            throw std::logic_error("Key has repeating characters");
        }
        for (char ch : key) {
            if (allowed_symbols.find(ch) == std::string::npos) {
                throw std::logic_error("Key has illegal symbol");
            }
        }
    }

    static void display_matrix(const std::vector<char>& matrix, size_t row_length,
                               const std::unordered_map<char, int>& symbol_to_index) {
        size_t rows = (matrix.size() / row_length) + (matrix.size() % row_length == 0 ? 0 : 1);

        for (size_t row_idx = 0; row_idx < rows; ++row_idx) {
            size_t limit = row_idx + 1 == rows ? row_length - (row_length - (matrix.size() % row_length)) : row_length;

            for (size_t col_idx = 0; col_idx < limit; ++col_idx) {
                std::cout << matrix[row_idx * row_length + col_idx] << "  ";
            }
            std::cout << std::endl;
            for (size_t col_idx = 0; col_idx < limit; ++col_idx) {
                int idx = symbol_to_index.at(matrix[row_idx * row_length + col_idx]);
                std::cout << idx << (idx > 10 ? " " : "  ");
            }
            std::cout << std::endl;
        }
    }

public:
    encryptor() : allowed_symbols(symbol_set) {
        int i = 0;
        for (char ch : "ETAONRIS") {
            most_frequent_symbols.emplace(ch, i++);
        }
    }

    std::string encrypt(const std::string& formatted_text, const std::string& key) {
        validate_key(key);
        std::cout << "Formatted text: " << formatted_text << std::endl;

        std::unordered_map<char, int> symbol_to_index(most_frequent_symbols);
        std::vector<char> matrix(key.begin(), key.end());
        for (char ch : allowed_symbols) {
            if (key.find(ch) == std::string::npos) {
                matrix.push_back(ch);
            }
        }

        int index = STARTING_SYMBOL_INDEX;
        const size_t row_length = key.size();
        size_t rows = (matrix.size() / row_length) + (matrix.size() % row_length == 0 ? 0 : 1);

        for (size_t col_idx = 0; col_idx < row_length; ++col_idx) {
            for (size_t row_idx = 0; row_idx < rows; ++row_idx) {
                if (row_idx * row_length + col_idx >= matrix.size()) {
                    break;
                }
                char ch = matrix[row_idx * row_length + col_idx];
                if (symbol_to_index.find(ch) == symbol_to_index.end()) {
                    symbol_to_index.emplace(ch, index++);
                }
            }
        }
        std::cout << "Matrix: " << std::endl;
        display_matrix(matrix, row_length, symbol_to_index);

        std::string result;
        for (char ch : formatted_text) {
            if (isdigit(ch)) {
                result += ch;
                result += ch;
            } else {
                result += std::to_string(symbol_to_index.at(ch));
            }
        }
        return result;
    }
};

int main() {
    formatter f({new to_upper, new remove_illegal_symbols(symbol_set), new letter_before_number,
                 new number_before_letter});
    std::string text = "5 Avenue de la Vieille Ville. St. Nazzaire 43601";
    std::string key = "SOMBRE";

    std::cout << "Plain text: " << text << std::endl;
    std::cout << "Key: " << key << std::endl;

    encryptor enc;
    std::string result = enc.encrypt(f.format_text(text), key);
    std::cout << "Cypher: " << std::endl;
    for (size_t i = 0; i < result.size(); ++i) {
        if (i != 0 && i % DISPLAY_BATCH_SIZE == 0) {
            std::cout << ' ';
        }
        std::cout << result[i];
    }
    if (result.size() % DISPLAY_BATCH_SIZE != 0) {
        std::fill_n(std::ostream_iterator<char>(std::cout), DISPLAY_BATCH_SIZE - (result.size() % DISPLAY_BATCH_SIZE), '0');
    }
    std::cout << std::endl;
    return 0;
}
