#pragma once
#include <string>

namespace vliva {
class App {
public:
    App();
    void init();
    void loadModel(const std::string& path);
    void run();
};
} // namespace vliva
