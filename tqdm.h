#ifndef TQDM_H
#define TQDM_H
#include <chrono>
#include <ctime>
#include <numeric>
#include <ios>
#include <string>
#include <cstdlib>
#include <iostream>
#include <utility>
#include <vector>
#include <algorithm>

class tqdm {
    private:
        // time, iteration counters and deques for rate calculations
        std::chrono::time_point<std::chrono::system_clock> t_first = std::chrono::system_clock::now();
        std::chrono::time_point<std::chrono::system_clock> t_old = std::chrono::system_clock::now();
        int n_old = 0;
        std::vector<double> deq_t;
        std::vector<int> deq_n;
        int nupdates = 0;
        int total_ = 0;
        int period = 1;
        unsigned int smoothing = 50;
        bool use_ema = true;
        float alpha_ema = 0.1f;

        std::vector<const char*> bars = {" ", "▏", "▎", "▍", "▌", "▋", "▊", "▉", "█"};

        bool use_colors = true;
        bool color_transition = true;
        int width = 40;

        std::string right_pad = "▏";
        std::string label = "";

        static void hsv_to_rgb(float h, float s, float v, int& r, int& g, int& b) {
            if (s < 1e-6) {
                v *= 255.;
                r = static_cast< int >( v );
                g = static_cast< int >( v );
                b = static_cast< int >( v );
            }
            int i = static_cast<int>(h * 6.0);
            const float f = (h*6.f)-static_cast<float>(i);
            const int p = static_cast<int>(255.0 * (v * (1. - s)));
            const int q = static_cast<int>(255.0 * (v * (1. - s * f)));
            const int t = static_cast<int>(255.0 * (v * (1. - s * (1. - f))));
            v *= 255;
            i %= 6;
            const int vi = static_cast<int>(v);
            if (i == 0)      { r = vi; g = t;  b = p;  }
            else if (i == 1) { r = q;  g = vi; b = p;  }
            else if (i == 2) { r = p;  g = vi; b = t;  }
            else if (i == 3) { r = p;  g = q;  b = vi; }
            else if (i == 4) { r = t;  g = p;  b = vi; }
            else if (i == 5) { r = vi; g = p;  b = q;  }
        }

    public:
#ifdef _WIN32
        tqdm() noexcept :
            use_colors( true )
        {
            set_theme_basic();
        }
#endif

        void reset() {
            t_first = std::chrono::system_clock::now();
            t_old = std::chrono::system_clock::now();
            n_old = 0;
            deq_t.clear();
            deq_n.clear();
            period = 1;
            nupdates = 0;
            total_ = 0;
            label = "";
        }

        void set_theme_basic() {
            bars = { " ", "\\", "-", "/", "|", "\\", "-", "/", "|" };
            right_pad = "|";
        }

        void set_theme_line() {
#ifdef _WIN32
            bars = { " ", "_", "_", "_", "-", "-", "-", "-", "=" };
#else
            bars = {"─", "─", "─", "╾", "╾", "╾", "╾", "━", "═"};
#endif
        }
        void set_theme_circle() {
#ifdef _WIN32
            bars = { " ", ".", ".", ".", "o", "o", "o", "o", "O" };
#else
            bars = {" ", "◓", "◑", "◒", "◐", "◓", "◑", "◒", "#"};
#endif
        }
        void set_theme_braille() {
#ifdef _WIN32
            set_theme_basic();
#else
            bars = {" ", "⡀", "⡄", "⡆", "⡇", "⡏", "⡟", "⡿", "⣿" };
#endif
        }
        void set_theme_braille_spin() {
#ifdef _WIN32
            set_theme_basic();
#else
            bars = {" ", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠇", "⠿" };
#endif
        }
        void set_theme_vertical() {
#ifdef _WIN32
            bars = { " ", ".", ".", ".", ":", ":", ":", ":", "|" };
#else
            bars = {"▁", "▂", "▃", "▄", "▅", "▆", "▇", "█", "█"};
#endif
        }

        void set_label(std::string label_) { label = std::move(label_); }
        void disable_colors() {
            color_transition = false;
            use_colors = false;
        }

        void finish() {
            progress(total_,total_);
            printf("\n");
            fflush(stdout);
        }
        void progress(int curr, const int tot) {
            if(curr%period == 0) {
                total_ = tot;
                nupdates++;
                const auto now = std::chrono::system_clock::now();
                const double dt = static_cast<std::chrono::duration<double>>(now - t_old).count();
                const double dt_tot = static_cast<std::chrono::duration<double>>(now - t_first).count();
                const int dn = curr - n_old;
                n_old = curr;
                t_old = now;
                if (deq_n.size() >= smoothing) deq_n.erase(deq_n.begin());
                if (deq_t.size() >= smoothing) deq_t.erase(deq_t.begin());
                deq_t.push_back(dt);
                deq_n.push_back(dn);

                double avgrate;
                if (use_ema) {
                    avgrate = deq_n[0] / deq_t[0];
                    for (unsigned int i = 1; i < deq_t.size(); i++) {
                        double r = 1.0*deq_n[i]/deq_t[i];
                        avgrate = alpha_ema*r + (1.0-alpha_ema)*avgrate;
                    }
                } else {
	                const double dtsum = std::accumulate(deq_t.begin(),deq_t.end(),0.);
	                const int dnsum = std::accumulate(deq_n.begin(),deq_n.end(),0);
                    avgrate = dnsum/dtsum;
                }

                // learn an appropriate period length to avoid spamming stdout
                // and slowing down the loop, shoot for ~25Hz and smooth over 3 seconds
                if (nupdates > 10) {
                    period = static_cast<int>(std::min(std::max((1.0 / 25) * curr / dt_tot, 1.0), 5e5));
                    smoothing = 25*3;
                }
                double peta = (tot-curr)/avgrate;
                double pct = static_cast<double>(curr)/(tot*0.01);
                if( ( tot - curr ) <= period ) {
                    pct = 100.0;
                    avgrate = tot/dt_tot;
                    curr = tot;
                    peta = 0;
                }

                const double fills = (static_cast<double>(curr) / tot * width);
                const int ifills = static_cast<int>(fills);

                printf("\015 ");
                if (use_colors) {
                    if (color_transition) {
                        // red (hue=0) to green (hue=1/3)
                        int r = 255, g = 255, b = 255;
                        hsv_to_rgb(static_cast<float>(0.0+0.01*pct/3),0.65f,1.0f, r,g,b);
                        printf("\033[38;2;%d;%d;%dm ", r, g, b);
                    } else {
                        printf("\033[32m ");
                    }
                }
                for (int i = 0; i < ifills; i++) std::cout << bars[8];
                if (curr != tot) printf("%s",bars[(int)(8.0*(fills-ifills))]);
                for (int i = 0; i < width-ifills-1; i++) std::cout << bars[0];
                printf("%s ", right_pad.c_str());
                if (use_colors) printf("\033[1m\033[31m");
                printf("%4.1f%% ", pct);
                if (use_colors) printf("\033[34m");

                std::string unit = "Hz";
                double div = 1.;
                if (avgrate > 1e6) {
                    unit = "MHz"; div = 1.0e6;
                } else if (avgrate > 1e3) {
                    unit = "kHz"; div = 1.0e3;
                }
                printf("[%4d/%4d | %3.1f %s | %.0fs<%.0fs] ", curr,tot,  avgrate/div, unit.c_str(), dt_tot, peta);
                printf("%s ", label.c_str());
                if (use_colors) printf("\033[0m\033[32m\033[0m\015 ");

                if( ( tot - curr ) > period ) fflush(stdout);

            }
        }
};
#endif
