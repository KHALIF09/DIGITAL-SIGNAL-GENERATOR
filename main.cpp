#include "DigitalSignalGenerator.hpp"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <implot.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main(int, char**) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;
#if defined(__APPLE__)
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
#else
    const char* glsl_version = "#version 130";
#endif

    GLFWwindow* window = glfwCreateWindow(1280, 900, "Digital Signal Generator (ImGui + ImPlot)", nullptr, nullptr);
    if (window == nullptr) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // State
    DigitalSignalGenerator gen(1.0, 200);
    char binary_input_c[1024] = "1100100100110";
    bool input_is_digital = true;
    int pcm_bits = 8;
    double dm_step = 0.15;
    int sampling_rate = gen.sampling_rate;
    int bit_duration_int = 1;

    // encoding scheme
    int encoding_idx = 0;
    const char* encoding_names[] = {"NRZ-L","NRZ-I","Manchester","Diff Manchester","AMI"};
    std::string current_data;
    std::vector<double> current_time, current_signal;
    std::string output_report;

    bool use_scrambling = false;
    int scramble_idx = 0; // 0 B8ZS, 1 HDB3

    // main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Left panel: controls
        ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Text("Input Type");
        ImGui::RadioButton("Digital Input", &input_is_digital, 1); ImGui::SameLine();
        ImGui::RadioButton("Analog Input (PCM/DM)", &input_is_digital, 0);

        ImGui::Separator();
        ImGui::InputText("Binary Data", binary_input_c, IM_ARRAYSIZE(binary_input_c));
        ImGui::InputInt("Sampling Rate", &sampling_rate);
        if (sampling_rate < 10) sampling_rate = 10;
        gen.sampling_rate = sampling_rate;

        ImGui::Separator();
        ImGui::Text("Analog Modulation (when analog chosen)");
        ImGui::InputInt("PCM bits", &pcm_bits);
        ImGui::InputDouble("DM step", &dm_step, 0.01, 0.1, "%.3f");

        ImGui::Separator();
        ImGui::Text("Line Encoding");
        ImGui::Combo("Scheme", &encoding_idx, encoding_names, IM_ARRAYSIZE(encoding_names));

        ImGui::Checkbox("Apply Scrambling (AMI only)", &use_scrambling);
        ImGui::Combo("Scrambling", &scramble_idx, "B8ZS\0HDB3\0");

        if (ImGui::Button("Generate Signal")) {
            output_report.clear();
            if (input_is_digital) {
                current_data = std::string(binary_input_c);
                // sanitize
                current_data.erase(std::remove_if(current_data.begin(), current_data.end(), [](char c){return c!='0' && c!='1';}), current_data.end());
                if (current_data.empty()) current_data = "0";
            } else {
                // generate simple analog signal (sine)
                std::vector<double> analog;
                int N = 50;
                analog.reserve(N);
                for (int i=0;i<N;++i) analog.push_back(std::sin(2.0*M_PI*(double)i/(double)N));
                current_data = (pcm_bits>0) ? gen.pcm_encode(analog, pcm_bits) : gen.delta_modulation(analog, dm_step);
            }

            // palindrome
            auto [pal, start, plen] = gen.longest_palindrome_manacher(current_data);

            // encode
            switch(encoding_idx) {
                case 0: tie(current_time, current_signal) = gen.nrz_l(current_data); break;
                case 1: tie(current_time, current_signal) = gen.nrz_i(current_data); break;
                case 2: tie(current_time, current_signal) = gen.manchester(current_data); break;
                case 3: tie(current_time, current_signal) = gen.differential_manchester(current_data); break;
                case 4: tie(current_time, current_signal) = gen.ami(current_data); break;
            }

            std::string scrambled;
            if (encoding_idx==4 && use_scrambling) {
                scrambled = (scramble_idx==0) ? gen.b8zs_scramble(current_data) : gen.hdb3_scramble(current_data);
            }

            std::ostringstream rep;
            rep << "================ SIGNAL GENERATION REPORT ================\n";
            rep << "Input Data: " << (current_data.size() > 80 ? current_data.substr(0,80) + "..." : current_data) << "\n";
            rep << "Bits: " << current_data.size() << "\n";
            rep << "Encoding: " << encoding_names[encoding_idx] << "\n\n";
            rep << "---------------- PALINDROME ----------------\n";
            rep << "Longest palindrome: \"" << pal << "\" start=" << start << " len=" << plen << "\n\n";
            if (!scrambled.empty()) {
                rep << "---------------- SCRAMBLING ----------------\n";
                rep << "Type: " << (scramble_idx==0 ? "B8ZS" : "HDB3") << "\n";
                rep << "Scrambled: " << (scrambled.size() > 80 ? scrambled.substr(0,80)+"..." : scrambled) << "\n\n";
            }
            double mean = 0.0, stddev = 0.0;
            if (!current_signal.empty()) {
                mean = std::accumulate(current_signal.begin(), current_signal.end(), 0.0) / current_signal.size();
                double var = 0.0;
                for (double v : current_signal) var += (v-mean)*(v-mean);
                var /= current_signal.size();
                stddev = std::sqrt(var);
            }
            rep << "Signal Mean: " << mean << " Std: " << stddev << "\n";
            rep << "Click Decode to decode the plotted signal.\n";
            output_report = rep.str();
        }

        ImGui::SameLine();
        if (ImGui::Button("Clear")) {
            current_data.clear();
            current_time.clear();
            current_signal.clear();
            output_report.clear();
            strcpy(binary_input_c, "1100100100110");
        }

        ImGui::Separator();
        if (ImGui::Button("Decode Signal")) {
            if (current_signal.empty()) {
                output_report = "Generate signal first.\n";
            } else {
                std::string decoded;
                switch(encoding_idx) {
                    case 0: decoded = gen.decode_nrz_l(current_signal); break;
                    case 1: decoded = gen.decode_nrz_i(current_signal); break;
                    case 2: decoded = gen.decode_manchester(current_signal); break;
                    case 3: decoded = gen.decode_differential_manchester(current_signal); break;
                    case 4: decoded = gen.decode_ami(current_signal); break;
                }
                // compute accuracy
                size_t matches = 0;
                for (size_t i=0;i< std::min(decoded.size(), current_data.size()); ++i) if (decoded[i]==current_data[i]) ++matches;
                double acc = current_data.empty() ? 0.0 : (100.0 * (double)matches / (double)current_data.size());
                std::ostringstream rep;
                rep << "================ DECODING REPORT ================\n";
                rep << "Original : " << (current_data.size() > 80 ? current_data.substr(0,80)+"..." : current_data) << "\n";
                rep << "Decoded  : " << (decoded.size() > 80 ? decoded.substr(0,80)+"..." : decoded) << "\n";
                rep << "Correct: " << matches << "/" << current_data.size() << "  Accuracy: " << std::fixed << std::setprecision(2) << acc << "%\n";
                output_report = rep.str();
            }
        }

        ImGui::End();

        // Right: plotting & output
        ImGui::Begin("Signal & Output", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        if (!current_time.empty() && !current_signal.empty()) {
            if (ImPlot::BeginPlot("Signal Plot", ImVec2(-1,300))) {
                ImPlot::SetupAxes("Time", "Voltage");
                ImPlot::SetupAxisLimits(ImAxis_X1, current_time.front(), current_time.back(), ImGuiCond_Always);
                ImPlot::SetupAxisLimits(ImAxis_Y1, -1.5, 1.5, ImGuiCond_Always);
                ImPlot::PlotLine("Signal", current_time.data(), current_signal.data(), (int)current_signal.size());
                ImPlot::EndPlot();
            }
        } else {
            ImGui::TextWrapped("No signal generated yet. Click Generate Signal.");
        }

        ImGui::Separator();
        ImGui::TextUnformatted(output_report.c_str());
        ImGui::End();

        // Render
        ImGui::Render();
        int display_w, display_h;
        glfwMakeContextCurrent(window);
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0,0,display_w, display_h);
        glClearColor(0.1f,0.1f,0.12f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}