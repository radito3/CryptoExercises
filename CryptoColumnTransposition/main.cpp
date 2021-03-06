#include <iostream>
#include <codecvt>
#include <locale>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <set>
#include <valarray>

const wchar_t symbols[] = L"АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЬЮЯ 0123456789";

inline int index_of(const wchar_t& symbol) {
    for (int i = 0; i < wcslen(symbols); i++) {
        if (symbols[i] == symbol) {
            return i;
        }
    }
    return -1;
}

inline std::ostream& operator<<(std::ostream& out, const std::wstring& utf16) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    std::string utf8 = converter.to_bytes(utf16);
    out << utf8;
    return out;
}

inline std::ostream& operator<<(std::ostream& out, const wchar_t& utf16) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    std::string utf8 = converter.to_bytes(utf16);
    out << utf8;
    return out;
}

inline std::istream& operator>>(std::istream& in, std::wstring& utf16) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    std::string utf8;
    getline(std::cin, utf8);
    if (!utf8.empty()) {
        utf16 = converter.from_bytes(utf8);
    }
    return in;
}

class encryptor {
    std::wstring input_;
    std::wstring key;
    size_t row_length;

    std::wstring pad_input() {
        if (input_.size() % key.size() != 0) {
            std::wstring padded_input(input_);
            fill_n(back_inserter(padded_input), key.size() - (input_.size() % key.size()), L' ');
            return padded_input;
        }
        return input_;
    }

    struct index_sorter {
        bool operator()(const wchar_t& left, const wchar_t& right) const {
            return index_of(left) < index_of(right);
        }
    };

    std::vector<int> parse_key_to_column_indices() {
        std::set<wchar_t, index_sorter> ordered_symbols(key.begin(), key.end());
        std::unordered_map<wchar_t, int> key_indices;
        int i = 0;
        for (const auto& symbol : ordered_symbols) {
            key_indices.emplace(symbol, i++);
        }
        std::vector<int> result;
        for (auto& ch : key) {
            result.push_back(key_indices.at(ch));
        }
        return result;
    }

public:
    encryptor(std::wstring input, std::wstring key) : input_(std::move(input)), key(std::move(key)) {
        row_length = this->key.size();
    }

    std::wstring encrypt() {
        std::wstring input = pad_input();
        std::valarray<wchar_t> in(input.data(), input.size());
        std::vector<int> transposed_column_indices = parse_key_to_column_indices();
        std::wstring result;

        for (size_t col_idx = 0; col_idx < row_length; ++col_idx) {
            auto slice = in[std::slice(transposed_column_indices[col_idx], input.size() / row_length, row_length)];
            std::valarray<wchar_t> column(slice);
            result.insert(result.end(), std::begin(column), std::end(column));
        }
        return result;
    }
};

int main() {
    std::cout << "Enter input: ";
    std::wstring input;
    std::cin >> input;

    std::cout << "Enter key: ";
    std::wstring key;
    std::cin >> key;

    std::cout << "Enter operation (1: Encrypt, 2: Decrypt): ";
    int operation;
    std::cin >> operation;

    encryptor enc(input, key);
    std::wstring result = enc.encrypt();
    std::cout << result << std::endl;
    return 0;
}
