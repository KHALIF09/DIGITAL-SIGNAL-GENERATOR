#pragma once
#include <vector>
#include <string>
#include <tuple>

class DigitalSignalGenerator {
public:
    DigitalSignalGenerator(double bit_duration = 1.0, int sampling_rate = 100);

    // Encoders / decoders
    std::string pcm_encode(const std::vector<double>& analog_signal, int n_bits = 8) const;
    std::string delta_modulation(const std::vector<double>& analog_signal, double step_size = 0.1) const;

    // Manacher
    std::tuple<std::string,int,int> longest_palindrome_manacher(const std::string& data_stream) const;

    // Line encodings -> return pair<time, signal>
    std::pair<std::vector<double>, std::vector<double>> nrz_l(const std::string& data) const;
    std::pair<std::vector<double>, std::vector<double>> nrz_i(const std::string& data) const;
    std::pair<std::vector<double>, std::vector<double>> manchester(const std::string& data) const;
    std::pair<std::vector<double>, std::vector<double>> differential_manchester(const std::string& data) const;
    std::pair<std::vector<double>, std::vector<double>> ami(const std::string& data) const;

    // Decoders
    std::string decode_nrz_l(const std::vector<double>& signal) const;
    std::string decode_nrz_i(const std::vector<double>& signal) const;
    std::string decode_manchester(const std::vector<double>& signal) const;
    std::string decode_differential_manchester(const std::vector<double>& signal) const;
    std::string decode_ami(const std::vector<double>& signal) const;

    // Scrambling
    std::string b8zs_scramble(const std::string& data) const;
    std::string hdb3_scramble(const std::string& data) const;

    // Utilities
    std::vector<std::pair<int,int>> find_zero_sequences(const std::string& data) const;

    // config
    double bit_duration;
    int sampling_rate;
};