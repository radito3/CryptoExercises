#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <codecvt>
#include <locale>
#include <iomanip>

using namespace std;

const wchar_t allowed_symbols[] = L"АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЬЮЯABCDEFGIJKLMNOPQRSTUVWXYZ0123456789 \"-*";
//const wchar_t allowed_symbols[] = L"АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЬЮЯ";

class encryptor {
    wstring key;
    int key_index;
    unordered_map<wchar_t, int> index_cache;
    const int set_size = wcslen(allowed_symbols);

public:
    encryptor(wstring key, const unordered_map<wchar_t, int>& indexCache)
        : key(move(key)), index_cache(indexCache), key_index(0) {}

    wchar_t operator()(const wchar_t& symbol) {
        int input_symbol_index = index_cache.at(symbol);
        if (key_index == key.size()) {
            key_index -= key.size();
        }
        int key_symbol_index = index_cache.at(key[key_index++]);

        int result_index = (input_symbol_index + key_symbol_index) % set_size;

        return allowed_symbols[result_index];
    }
};

class decryptor {
    wstring key;
    int key_index;
    unordered_map<wchar_t, int> index_cache;
    const int set_size = wcslen(allowed_symbols);

public:
    decryptor(wstring key, const unordered_map<wchar_t, int>& indexCache)
        : key(move(key)), index_cache(indexCache), key_index(0) {}

    wchar_t operator()(const wchar_t& symbol) {
        int input_symbol_index = index_cache.at(symbol);
        if (key_index == key.size()) {
            key_index -= key.size();
        }
        int key_symbol_index = index_cache.at(key[key_index++]);

        int result_index = (input_symbol_index - key_symbol_index + set_size) % set_size;

        return allowed_symbols[result_index];
    }
};

enum Operation { Encrypt = 1, Decrypt };

inline istream& operator>>(istream& in, Operation& op) {
    int val;
    in >> val;
    op = static_cast<Operation>(val);
    return in;
}

//necessary because Windows doesn't natively support wide character streams
inline ostream& operator<<(ostream& out, const wstring& utf16) {
    wstring_convert<codecvt_utf8<wchar_t>, wchar_t> converter;
    string utf8 = converter.to_bytes(utf16);
    out << utf8;
    return out;
}

//necessary because Windows doesn't natively support wide character streams
inline ostream& operator<<(ostream& out, const wchar_t& utf16) {
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

class cipher_worker {

    unordered_map<wchar_t, int> index_cache;

    /**
     * padding algorithm:
     *  text: abc
     *  1st iteration: *abc
     *  2nd iteration: *abc*
     *  3rd iteration: **abc*
     *  ...
    */
    static void pad_input(wstring& input, const wstring& key) {
        if (input.size() % key.size() == 0) {
            return;
        }
        size_t times = key.size() - (input.size() % key.size());
        if (times % 2 == 0) {
            input.insert(0, times / 2, '*');
        } else {
            input.insert(0, (times / 2) + 1, '*');
        }
        fill_n(back_inserter(input), times / 2, '*');
    }

    void validate_input(const wstring& input, const wstring& key) {
        if (input.size() > 300) {
            throw runtime_error("Illegal input length");
        }
        if (key.empty()) {
            throw runtime_error("No key provided");
        }
        if (key.size() > input.size()) {
            throw runtime_error("Illegal key length");
        }
        auto contained_in_set = [&](auto& symbol) { return index_cache.find(symbol) != index_cache.end(); };
        if (!all_of(input.begin(), input.end(), contained_in_set) ||
                !all_of(key.begin(), key.end(), contained_in_set)) {
            throw runtime_error("Illegal symbol detected");
        }
    }

    static void trim_asterisks(wstring& result) {
        if (result.empty()) {
            return;
        }
        while (*result.begin() == L'*') {
            result.erase(0, 1);
        }
        while (*--result.end() == L'*') {
            result.erase(--result.end());
        }
    }

public:
    explicit cipher_worker(const unordered_map<wchar_t, int>& indexCache) : index_cache(indexCache) {}

    wstring operator()(wstring& input, Operation operation, const wstring& key) {
        validate_input(input, key);
        pad_input(input, key);

        wstring result;
        if (operation == Encrypt) {
            transform(input.begin(), input.end(), back_inserter(result), encryptor(key, index_cache));
        } else {
            transform(input.begin(), input.end(), back_inserter(result), decryptor(key, index_cache));
            trim_asterisks(result);
        }

        return result;
    }

    static double compute_frequency_coefficient(const wstring& result) {
        unordered_map<wchar_t, int> frequency;
        for (auto& ch : result) {
            ++frequency[ch];
        }
        int frequency_sum = 0;
        for (auto& [ch, freq] : frequency) {
            frequency_sum += freq * (freq - 1);
        }
        return double(frequency_sum) / double(result.size() * (result.size() - 1));
    }
};

int main() {
    unordered_map<wchar_t, int> indexCache;
    int n = wcslen(allowed_symbols);
    for (int i = 0; i < n; i++) {
        indexCache.emplace(allowed_symbols[i], i);
    }

    cipher_worker worker(indexCache);

    cout << "Enter input: ";
    wstring input;
    cin >> input;

    while (input != L"exit") {
        cout << "Enter key: ";
        wstring key;
        cin >> key;

        cout << "Enter operation (1: Encrypt, 2: Decrypt): ";
        Operation operation;
        cin >> operation;

        try {
            wstring result = worker(input, operation, key);

//            double frequency_coeff = cipher_worker::compute_frequency_coefficient(result);
//            cout << "Frequency coefficient: " << setprecision(4) << frequency_coeff << endl;

            cout << result << endl;
        } catch (const exception& e) {
            cerr << e.what() << endl;
        }

        input.clear();
        cout << "Enter input: ";
        getchar();
        cin >> input;
    }
    return 0;
}
