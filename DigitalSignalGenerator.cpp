#include "DigitalSignalGenerator.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <numeric>

DigitalSignalGenerator::DigitalSignalGenerator(double bit_duration_, int sampling_rate_)
    : bit_duration(bit_duration_), sampling_rate(sampling_rate_) { }

std::string DigitalSignalGenerator::pcm_encode(const std::vector<double>& analog_signal, int n_bits) const {
    if (analog_signal.size() < 2) throw std::invalid_argument("Signal needs at least 2 samples");
    double mn = *std::min_element(analog_signal.begin(), analog_signal.end());
    double mx = *std::max_element(analog_signal.begin(), analog_signal.end());
    double range = mx - mn;
    if (range == 0) range = 1e-10;
    int levels = 1 << n_bits;
    std::ostringstream out;
    for (double v : analog_signal) {
        double normalized = (v - mn) / range;
        int quant = static_cast<int>(std::floor(normalized * (levels - 1)));
        for (int b = n_bits - 1; b >= 0; --b) out << ((quant >> b) & 1);
    }
    return out.str();
}

std::string DigitalSignalGenerator::delta_modulation(const std::vector<double>& analog_signal, double step_size) const {
    if (analog_signal.empty()) return {};
    std::ostringstream out;
    double approximation = analog_signal.front();
    for (double sample : analog_signal) {
        if (sample > approximation) {
            out << '1';
            approximation += step_size;
        } else {
            out << '0';
            approximation -= step_size;
        }
    }
    return out.str();
}

std::tuple<std::string,int,int> DigitalSignalGenerator::longest_palindrome_manacher(const std::string& s) const {
    if (s.empty()) return {"",0,0};
    std::string t;
    t.reserve(s.size()*2 + 3);
    t.push_back('^');
    for (char c : s) { t.push_back('#'); t.push_back(c); }
    t.push_back('#');
    t.push_back('$');
    int n = static_cast<int>(t.size());
    std::vector<int> P(n,0);
    int center = 0, right = 0;
    for (int i = 1; i < n-1; ++i) {
        int mirror = 2*center - i;
        if (i < right) P[i] = std::min(right - i, P[mirror]);
        while (t[i+1+P[i]] == t[i-1-P[i]]) ++P[i];
        if (i + P[i] > right) { center = i; right = i + P[i]; }
    }
    int max_len = 0, center_idx = 0;
    for (int i = 1; i < n-1; ++i) if (P[i] > max_len) { max_len = P[i]; center_idx = i; }
    if (max_len==0) return {"",0,0};
    int start = (center_idx - max_len - 1) / 2;
    std::string pal = s.substr(start, max_len);
    return {pal, start, max_len};
}

static inline void append_samples(std::vector<double>& time, std::vector<double>& signal, double t0, double t1, int samples, double value) {
    double dt = (t1 - t0) / samples;
    for (int i = 0; i < samples; ++i) {
        time.push_back(t0 + i * dt);
        signal.push_back(value);
    }
}

std::pair<std::vector<double>, std::vector<double>> DigitalSignalGenerator::nrz_l(const std::string& data) const {
    std::vector<double> time, signal;
    int samples = sampling_rate;
    for (size_t i=0;i<data.size();++i) {
        double t0 = i * bit_duration;
        double t1 = (i+1) * bit_duration;
        double v = (data[i]=='1') ? 1.0 : -1.0;
        append_samples(time, signal, t0, t1, samples, v);
    }
    return {time, signal};
}

std::pair<std::vector<double>, std::vector<double>> DigitalSignalGenerator::nrz_i(const std::string& data) const {
    std::vector<double> time, signal;
    int samples = sampling_rate;
    double level = -1.0;
    for (size_t i=0;i<data.size();++i) {
        if (data[i]=='1') level = -level;
        double t0 = i * bit_duration;
        double t1 = (i+1) * bit_duration;
        append_samples(time, signal, t0, t1, samples, level);
    }
    return {time, signal};
}

std::pair<std::vector<double>, std::vector<double>> DigitalSignalGenerator::manchester(const std::string& data) const {
    std::vector<double> time, signal;
    int half_samples = sampling_rate / 2;
    for (size_t i=0;i<data.size();++i) {
        double t0 = i * bit_duration;
        double mid = t0 + bit_duration/2.0;
        if (data[i]=='1') {
            append_samples(time, signal, t0, mid, half_samples, -1.0);
            append_samples(time, signal, mid, t0 + bit_duration, half_samples, 1.0);
        } else {
            append_samples(time, signal, t0, mid, half_samples, 1.0);
            append_samples(time, signal, mid, t0 + bit_duration, half_samples, -1.0);
        }
    }
    return {time, signal};
}

std::pair<std::vector<double>, std::vector<double>> DigitalSignalGenerator::differential_manchester(const std::string& data) const {
    std::vector<double> time, signal;
    int half_samples = sampling_rate / 2;
    double level = 1.0;
    for (size_t i=0;i<data.size();++i) {
        double t0 = i * bit_duration;
        double mid = t0 + bit_duration/2.0;
        if (data[i]=='0') level = -level;
        append_samples(time, signal, t0, mid, half_samples, level);
        level = -level;
        append_samples(time, signal, mid, t0 + bit_duration, half_samples, level);
    }
    return {time, signal};
}

std::pair<std::vector<double>, std::vector<double>> DigitalSignalGenerator::ami(const std::string& data) const {
    std::vector<double> time, signal;
    int samples = sampling_rate;
    double last_one = -1.0;
    for (size_t i=0;i<data.size();++i) {
        double t0 = i * bit_duration;
        double t1 = (i+1) * bit_duration;
        double v = 0.0;
        if (data[i] == '1') {
            last_one = -last_one;
            v = last_one;
        }
        append_samples(time, signal, t0, t1, samples, v);
    }
    return {time, signal};
}

// decoders (simple heuristics similar to python)
std::string DigitalSignalGenerator::decode_nrz_l(const std::vector<double>& signal) const {
    int samples_per_bit = sampling_rate;
    std::string out;
    for (size_t i=0;i+samples_per_bit<=signal.size(); i += samples_per_bit) {
        double avg = std::accumulate(signal.begin()+i, signal.begin()+i+samples_per_bit, 0.0) / samples_per_bit;
        out.push_back(avg > 0.0 ? '1' : '0');
    }
    return out;
}

std::string DigitalSignalGenerator::decode_nrz_i(const std::vector<double>& signal) const {
    int s = sampling_rate;
    std::string out;
    if (signal.empty()) return out;
    double last_level = signal[0];
    for (size_t i=0;i+s<=signal.size(); i += s) {
        double avg = std::accumulate(signal.begin()+i, signal.begin()+i+s, 0.0) / s;
        char bit = (std::abs(avg - last_level) > 0.5) ? '1' : '0';
        out.push_back(bit);
        last_level = avg;
    }
    return out;
}

std::string DigitalSignalGenerator::decode_manchester(const std::vector<double>& signal) const {
    int s = sampling_rate;
    std::string out;
    for (size_t i=0;i+s<=signal.size(); i += s) {
        int half = s/2;
        double first = std::accumulate(signal.begin()+i, signal.begin()+i+half, 0.0) / half;
        double second = std::accumulate(signal.begin()+i+half, signal.begin()+i+s, 0.0) / (s-half);
        out.push_back(first < second ? '1' : '0');
    }
    return out;
}

std::string DigitalSignalGenerator::decode_differential_manchester(const std::vector<double>& signal) const {
    int s = sampling_rate;
    std::string out;
    for (size_t i=0;i+s<=signal.size(); i += s) {
        int half = s/2;
        double first = std::accumulate(signal.begin()+i, signal.begin()+i+half, 0.0) / half;
        double second = std::accumulate(signal.begin()+i+half, signal.begin()+i+s, 0.0) / (s-half);
        out.push_back(std::abs(first - second) > 0.5 ? '1' : '0');
    }
    return out;
}

std::string DigitalSignalGenerator::decode_ami(const std::vector<double>& signal) const {
    int s = sampling_rate;
    std::string out;
    for (size_t i=0;i+s<=signal.size(); i += s) {
        double avg = std::accumulate(signal.begin()+i, signal.begin()+i+s, 0.0) / s;
        out.push_back(std::abs(avg) > 0.1 ? '1' : '0');
    }
    return out;
}

std::vector<std::pair<int,int>> DigitalSignalGenerator::find_zero_sequences(const std::string& data) const {
    std::vector<std::pair<int,int>> seq;
    size_t i = 0;
    while (i < data.size()) {
        if (data[i] == '0') {
            size_t start = i;
            int count = 0;
            while (i < data.size() && data[i] == '0') { ++count; ++i; }
            seq.emplace_back((int)start, count);
        } else ++i;
    }
    return seq;
}

std::string DigitalSignalGenerator::b8zs_scramble(const std::string& data) const {
    std::string result = data;
    auto sequences = find_zero_sequences(data);
    for (auto &p : sequences) {
        int start = p.first;
        int length = p.second;
        if (length >= 8) {
            for (int j = start; j <= start + length - 8; j += 8) {
                // replace 8 bits with 000VB0VB where V/B are placeholders; we'll use 'V' and 'B' characters
                if (j + 8 <= (int)result.size()) {
                    result.replace(j, 8, "000VB0VB");
                }
            }
        }
    }
    return result;
}

std::string DigitalSignalGenerator::hdb3_scramble(const std::string& data) const {
    std::string result = data;
    auto sequences = find_zero_sequences(data);
    int ones_count = 0;
    for (auto &p : sequences) {
        int start = p.first;
        int length = p.second;
        if (length >= 4) {
            for (int j = start; j <= start + length - 4; j += 4) {
                if (j + 4 <= (int)result.size()) {
                    if (ones_count % 2 == 0) result.replace(j, 4, "000V");
                    else result.replace(j, 4, "B00V");
                    ++ones_count;
                }
            }
        }
    }
    return result;
}