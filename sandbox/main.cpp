#include <Myve/MyveEngine.h>
#include <Myve/Core/Exception.h>
#include <iostream>
#include <cstdlib>

int main() {
    try {
        myve::EngineConfig config{
            .windowTitle = "Myve Engine - Sandbox",
            .windowWidth = 1280,
            .windowHeight = 720,
            .resizable = true,
            .enableValidation = true
        };

        myve::MyveEngine engine(config);
        engine.run();
    } catch (const myve::MyveException& e) {
        std::cerr << "Erro fatal MyveException:\n" << e.what() << '\n';
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        std::cerr << "Erro padrão capturado:\n" << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
