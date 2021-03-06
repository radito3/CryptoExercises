#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <codecvt>
#include <locale>
#include <future>
#include <atomic>

using namespace std;

//const wchar_t all_symbols[] = L"АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЬЮЯ ";
const wchar_t all_symbols[] = L"АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЬЮЯABCDEFGIJKLMNOPQRSTUVWXYZ0123456789 \"-*";
//const wchar_t all_symbols[] = L"ГЗЛРУЧЬАДИМРФШЮБЕЙНСХЩЯВЖКОТЦЪ ";

class encryptor {
    unordered_map<wchar_t, int> index_cache;
    vector<wchar_t> symbol_set;

public:
    encryptor(const unordered_map<wchar_t, int>& index_cache, const vector<wchar_t>& set)
        : index_cache(index_cache), symbol_set(set) {}

    wchar_t operator()(const wchar_t& symbol) {
        int shift_index= index_cache.at(symbol) + 3;
        if (shift_index >= index_cache.size()) {
            shift_index -= index_cache.size();
        }
        return symbol_set[shift_index];
    }
};

class decryptor {
    unordered_map<wchar_t, int> index_cache;
    vector<wchar_t> symbol_set;

public:
    decryptor(const unordered_map<wchar_t, int>& index_cache, const vector<wchar_t>& set)
        : index_cache(index_cache), symbol_set(set) {}

    wchar_t operator()(const wchar_t& symbol) {
        int shift_index = index_cache.at(symbol) - 3;
        if (shift_index < 0) {
            shift_index += index_cache.size();
        }
        return symbol_set[shift_index];
    }
};

void validate_input(const wstring& input) {
    if (input.length() > 80) {
        throw runtime_error("Illegal plain text length");
    }

    int n = wcslen(all_symbols);
    //for O(1) time checking for symbols
    unordered_set<wchar_t> symbols(all_symbols, all_symbols + n);

    for (auto symbol : input) {
        if (symbols.find(symbol) == symbols.end()) {
            throw runtime_error("Illegal symbol detected");
        }
    }
}

enum Operation { Encrypt = 1, Decrypt };

inline istream& operator>>(istream& in, Operation& operation) {
    int val;
    in >> val;
    operation = static_cast<Operation>(val);
    return in;
}

wstring do_cipher(const wstring& input, Operation operation, const unordered_map<wchar_t, int>& index_cache,
                  const vector<wchar_t>& symbol_set) {
    wstring result;
    if (operation == Encrypt) {
        transform(input.begin(), input.end(), back_inserter(result), encryptor(index_cache, symbol_set));
    } else {
        transform(input.begin(), input.end(), back_inserter(result), decryptor(index_cache, symbol_set));
    }
    return result;
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

template <typename T>
inline size_t index(const vector<T>& array, const T& elem) {
    auto it = find(array.begin(), array.end(), elem);
    if (it == array.end()) {
        return -1;
    }
    return distance(array.begin(), it);
}

class permutation_checker {
    atomic_bool found_permutation;
    volatile bool is_done_forward = false;
    volatile bool is_done_backward = false;

    wstring_convert<codecvt_utf8<wchar_t>, wchar_t> converter;

    const wstring input;
    const wstring wanted_result;

    const int symbol_set_size = wcslen(all_symbols);

    void display_set(const vector<wchar_t>& set) {
        cout << "Резултатно множество: { ";
        for (const wchar_t& symbol : set) {
            string result = converter.to_bytes(symbol);
            cout << result << ' ';
        }
        cout << " }" << endl;
    }

public:
    permutation_checker() : found_permutation(false), input(L"СЕМИНАР"), wanted_result(L"ХДБРЕПС") {}

    bool is_finished() volatile {
        return found_permutation || (is_done_backward && is_done_forward);
    }

    void check_forward(vector<wchar_t> set) {
        do {
            if (found_permutation) {
                break;
            }
            bool found = false;
            for (size_t i = 0; i < input.size(); ++i) {
                size_t in_index = index(set, input[i]) + 3;
                if (in_index >= symbol_set_size) {
                    in_index -= symbol_set_size;
                }
                size_t out_index = index(set, wanted_result[i]);

                if (i == input.size() - 1 && in_index == out_index) {
                    found = true;
                    break;
                }
                if (in_index != out_index) {
                    break;
                }
            }
            if (found) {
                found_permutation = true;
                display_set(set);
                break;
            }
        } while (next_permutation(set.begin(), set.end()));
        is_done_forward = true;
    }

    void check_backward(vector<wchar_t> set) {
        do {
            if (found_permutation) {
                break;
            }
            bool found = false;
            for (size_t i = 0; i < input.size(); ++i) {
                size_t in_index = index(set, input[i]) + 3;
                if (in_index >= symbol_set_size) {
                    in_index -= symbol_set_size;
                }
                size_t out_index = index(set, wanted_result[i]);

                if (i == input.size() - 1 && in_index == out_index) {
                    found = true;
                    break;
                }
                if (in_index != out_index) {
                    break;
                }
            }
            if (found) {
                found_permutation = true;
                display_set(set);
                break;
            }
        } while (prev_permutation(set.begin(), set.end()));
        is_done_backward = true;
    }
};

int main() {
    unordered_map<wchar_t, int> index_cache;
    int n = wcslen(all_symbols);
    for (int i = 0; i < n; i++) {
        index_cache.emplace(all_symbols[i], i);
    }

    cout << "Enter input: ";
    wstring input;
    cin >> input;

    while (input != L"exit") {
        cout << "Enter operation (1: Encrypt, 2: Decrypt): ";
        Operation operation;
        cin >> operation;

        validate_input(input);
        wstring result = do_cipher(input, operation, index_cache, vector<wchar_t>(all_symbols, all_symbols + wcslen(all_symbols)));
        cout << result << endl;

        input.clear();
        cout << "Enter input: ";
        getchar();
        cin >> input;
    }

//    permutation_checker checker;
//    vector<wchar_t> set(all_symbols, all_symbols + n);
//
//    thread t1(&permutation_checker::check_forward, ref(checker), set);
//    thread t2(&permutation_checker::check_backward, ref(checker), set);
//
//    cout << "started..." << endl;
//
//    while (!checker.is_finished()) {
//        this_thread::sleep_for(chrono::seconds(10));
//    }

    return 0;
}
