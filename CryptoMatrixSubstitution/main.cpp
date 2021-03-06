#include <iostream>
#include <vector>
#include <valarray>
#include <codecvt>
#include <locale>
#include <algorithm>

using namespace std;

const wchar_t symbols[] = L"АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЬЮЯ";

inline int index_of(const wchar_t& symbol) {
    for (int i = 0; i < wcslen(symbols); i++) {
        if (symbols[i] == symbol) {
            return i;
        }
    }
    return -1;
}

inline size_t index_in_matrix(size_t row, size_t col) {
    return row * wcslen(symbols) + col;
}

inline ostream& operator<<(ostream& out, const wstring& utf16) {
    wstring_convert<codecvt_utf8<wchar_t>, wchar_t> converter;
    string utf8 = converter.to_bytes(utf16);
    out << utf8;
    return out;
}

inline istream& operator>>(istream& in, wstring& utf16) {
    wstring_convert<codecvt_utf8<wchar_t>, wchar_t> converter;
    string utf8;
    getline(cin, utf8);
    if (!utf8.empty()) {
        utf16 = converter.from_bytes(utf8);
    }
    return in;
}

vector<wchar_t> generate_matrix() {
    size_t alphabet_size = wcslen(symbols);
    vector<wchar_t> matrix;
    valarray<wchar_t> symbol_set(symbols, alphabet_size);

    for (size_t i = 0; i < alphabet_size; i++) {
        matrix.insert(matrix.end(), begin(symbol_set), end(symbol_set));
        symbol_set = symbol_set.cshift(1);
    }
    return matrix;
}

class encryptor {
    wstring key;
    int key_index = 0;

    const vector<wchar_t> matrix;

public:
    encryptor(wstring key, const vector<wchar_t>& matrix) : key(move(key)), matrix(matrix) {}

    encryptor(wstring key, vector<wchar_t>&& matrix) : key(move(key)), matrix(move(matrix)) {}

    wchar_t operator()(const wchar_t& symbol) {
        int input_symbol_index = index_of(symbol);
        if (input_symbol_index == -1) {
            return symbol;
        }

        if (key_index >= key.size()) {
            key_index -= key.size();
        }
        int key_symbol_index = index_of(key[key_index++]);
        if (key_symbol_index == -1) {
            throw runtime_error("Illegal symbol in key detected");
        }

        size_t matrix_index = index_in_matrix(key_symbol_index, input_symbol_index);
        return matrix[matrix_index];
    }
};

wstring encrypt(const wstring& input, const wstring& key) {
    wstring result;
    transform(input.begin(), input.end(), back_inserter(result), encryptor(key, generate_matrix()));
    return result;
}

int main() {
    cout << "Enter input: ";
    wstring input;
    cin >> input;

    cout << "Enter key: ";
    wstring key;
    cin >> key;

    cout << "Enter operation (1: Encrypt, 2: Decrypt): ";
    int operation;
    cin >> operation;

    wstring result = encrypt(input, key);
    cout << result << endl;

    return 0;
}
