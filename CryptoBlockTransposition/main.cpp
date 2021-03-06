#include <iostream>
#include <vector>
#include <codecvt>
#include <locale>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <iomanip>

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

class encryptor {
    wstring input;
    wstring key;

    wstring pad_input() {
        if (input.size() % key.size() != 0) {
            wstring padded_input(input);
            fill_n(back_inserter(padded_input), key.size() - (input.size() % key.size()), L' ');
            return padded_input;
        }
        return input;
    }

    //this is better space-wise but worse time-wise than using a hash table with the symbol indices
    // but it also scales worse
    struct index_sorter {
        bool operator()(const wchar_t& left, const wchar_t& right) const {
            return index_of(left) < index_of(right);
        }
    };

    vector<int> parse_key_to_indices() {
        unordered_map<wchar_t, int> key_indices;
        {
            set<wchar_t, index_sorter> ordered_symbols(key.begin(), key.end());
            int i = 0;
            for (auto symbol : ordered_symbols) {
                key_indices.emplace(symbol, i++);
            }
        }
        vector<int> result;
        for (auto ch : key) {
            result.push_back(key_indices.at(ch));
        }
        return result;
    }

public:
    encryptor(wstring input, wstring key) : input(std::move(input)), key(std::move(key)) {
        int set_size = wcslen(symbols);
        unordered_set<wchar_t> set(symbols, symbols + set_size);
        auto contained_in_set = [&](auto& ch) { return set.find(ch) != set.end(); };

        if (!all_of(input.begin(), input.end(), contained_in_set) ||
            !all_of(key.begin(), key.end(), contained_in_set)) {
            throw runtime_error("Illegal symbol detected");
        }
    }

    wstring encrypt() {
        wstring input_to_use = pad_input();
        vector<int> key_as_indices = parse_key_to_indices();
        wstring result;

        for (int i = 0, begin_idx = 0; i < input_to_use.size() / key.size(); i++, begin_idx += key.size()) {
            wstring partition = input_to_use.substr(begin_idx, key.size());
            vector<wchar_t> buff(key.size() * sizeof(wchar_t));

            for (int j = 0; j < key_as_indices.size(); j++) {
                buff[key_as_indices[j]] = partition[j];
            }
            result.append(buff.begin(), buff.end());
        }
        return result;
    }

    static void print_frequency_coefficient(const wstring& result) {
        unordered_map<wchar_t, int> frequency;
        for (auto& ch : result) {
            ++frequency[ch];
        }
        int frequency_sum = 0;
        for (auto& [ch, freq] : frequency) {
            frequency_sum += freq * (freq - 1);
        }
        double frequency_coeff = double(frequency_sum) / double(result.size() * (result.size() - 1));
        cout << setprecision(4) << frequency_coeff << endl;
    }
};

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

    encryptor enc(input, key);
    wstring result = enc.encrypt();
    encryptor::print_frequency_coefficient(result);

    cout << result << endl;

    return 0;
}
