#include "tqdm.h"

#ifdef _MSC_VER
#include <windows.h>
#include <thread>
#else
#include <unistd.h>
#endif

#ifdef _MSC_VER
static void usleep(int usec) {
    std::this_thread::sleep_for(std::chrono::microseconds(usec));
}
#endif

int main() {

    int N = 2000;
    tqdm bar;

    std::cout << "Overhead of loop only:" << std::endl;
    for(int i = 0; i < N; i++) {
        bar.progress(i, N);
    }
    bar.finish();


    std::cout << "Basic:" << std::endl;
    bar.reset();
    bar.set_theme_basic();
    for(int i = 0; i < N; i++) {
        bar.progress(i, N);
    }
    bar.finish();

    std::cout << "Braille:" << std::endl;
    bar.reset();
    bar.set_theme_braille();
    for(int i = 0; i < N; i++) {
        bar.progress(i, N);
    }
    bar.finish();

    std::cout << "Line:" << std::endl;
    bar.reset();
    bar.set_theme_line();
    for(int i = 0; i < N; i++) {
        bar.progress(i, N);
    }
    bar.finish();

    std::cout << "Circles:" << std::endl;
    bar.reset();
    bar.set_theme_circle();
    for(int i = 0; i < N; i++) {
        bar.progress(i, N);
    }
    bar.finish();

    bar.reset();
    std::cout << "Vertical bars:" << std::endl;
    bar.reset();
    bar.set_theme_vertical();
    for(int i = 0; i < N; i++) {
        bar.progress(i, N);
    }
    bar.finish();

    return 0;
}
