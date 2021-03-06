#include <iostream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <codecvt>
#include <locale>
#include <regex>

using namespace std;

const wchar_t allowed_symbols[] = L"АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЬЮЯ0123456789 .-#";
const int mapped_values[] = {10,20,30,40,15,25,11,21,31,41,35,45,12,22,32,42,13,23,33,43,14,24,34,16,26,36,17,27,37,
                             18,28,38,19,29,39,1,2,3,4,5,6,7,8,9};

enum Operation { Encrypt = 1, Decrypt };

inline istream& operator>>(istream& in, Operation& operation) {
    int val;
    in >> val;
    operation = static_cast<Operation>(val);
    return in;
}

class encryptor {
    unordered_map<wchar_t, int> mappings;

public:
    explicit encryptor(const unordered_map<wchar_t, int>& mappings) : mappings(mappings) {}

    int operator()(const wchar_t& symbol) {
        return mappings.at(symbol);
    }
};

class decryptor {
    unordered_map<int, wchar_t> mappings;

public:
    explicit decryptor(const unordered_map<int, wchar_t>& mappings) : mappings(mappings) {}

    wchar_t operator()(const int& symbol) {
        return mappings.at(symbol);
    }
};

template<typename Map, typename Input>
void validate_input(const Map& map, const Input& input) {
    if (input.size() > 300) {
        throw runtime_error("Illegal plain text length");
    }
    for (auto val : input) {
        if (map.find(val) == map.end()) {
            throw runtime_error("Illegal symbol detected");
        }
    }
}

//necessary because Windows doesn't natively support wide character streams
inline ostream& operator<<(ostream& out, const wstring& utf16) {
    wstring_convert<codecvt_utf8<wchar_t>, wchar_t> converter;
    string utf8 = converter.to_bytes(utf16);
    out << utf8;
    return out;
}

//necessary because Windows doesn't natively support wide character streams
inline istream& operator>>(istream& in, wstring& utf16) {
    wstring_convert<codecvt_utf8<wchar_t>, wchar_t> converter;
    string utf8;
    getline(cin, utf8);
    if (!utf8.empty()) {
        utf16 = converter.from_bytes(utf8);
    }
    return in;
}

int main() {
    unordered_map<wchar_t, int> symbol_to_number;
    unordered_map<int, wchar_t> number_to_symbol;
    int n = wcslen(allowed_symbols);
    int n1 = size(mapped_values);

    for (int i = 0; n == n1 && i < n; i++) {
        symbol_to_number.emplace(allowed_symbols[i], mapped_values[i]);
        number_to_symbol.emplace(mapped_values[i], allowed_symbols[i]);
    }

    cout << "Enter input: ";
    wstring input;
    cin >> input;

    while (input != L"exit") {
        cout << "Enter operation (1: Encrypt, 2: Decrypt): ";
        Operation operation;
        cin >> operation;

        if (operation == Decrypt) {
            //needed because wstring_token_iterator isn't supported by some systems
            wstring_convert<codecvt_utf8<wchar_t>, wchar_t> converter;
            string utf8 = converter.to_bytes(input);

            vector<int> parsedInput;
            regex space(" ");
            transform(sregex_token_iterator(utf8.begin(), utf8.end(), space, -1),
                      sregex_token_iterator(),
                      back_inserter(parsedInput),
                      [](auto& str){ return stoi(str); });

            validate_input(number_to_symbol, parsedInput);

            wstring result;
            transform(parsedInput.begin(), parsedInput.end(), back_inserter(result), decryptor(number_to_symbol));

            cout << result << endl;
        } else {
            validate_input(symbol_to_number, input);

            vector<int> result;
            transform(input.begin(), input.end(), back_inserter(result), encryptor(symbol_to_number));

            copy(result.begin(), result.end(), ostream_iterator<char>(cout, " "));
            cout << endl;
        }

        input.clear();
        cout << "Enter input: ";
        getchar();
        cin >> input;
    }

    return 0;
}
