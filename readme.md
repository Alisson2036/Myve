# Myve

Engine gráfica e framework de renderização moderna em desenvolvimento com **C++20** e a API **Vulkan**, projetada com foco em arquitetura limpa, alta performance e suporte multiplataforma (Windows e Linux).

O projeto é a evolução direta do [WindowFramework](https://github.com/SEU_USUARIO/WindowFramework), substituindo o pipeline anterior baseado em DirectX 11 por uma camada de abstração moderna sobre Vulkan e build automatizado com CMake.

---

### Tecnologias

* **Linguagem:** C++20
* **API Gráfica:** Vulkan (LunarG SDK 1.3+)
* **Gerenciador de Janelas & Input:** GLFW 3.4
* **Build System:** CMake (3.24+)

---

### Pré-requisitos

* **LunarG Vulkan SDK** (1.3 ou superior) instalado no sistema.
* Compilador C++20 compatível:
  * **Windows:** Visual Studio 2022 / 2026 (MSVC v143+) ou Clang.
  * **Linux:** GCC 12+ ou Clang 15+.
* **CMake 3.24+** e **Ninja**.

---

### Como Compilar

#### Windows (Visual Studio)
1. Abra o Visual Studio e selecione **"Abrir uma Pasta Local"** apontando para a raiz do repositório.
2. O CMake configurará o ambiente automaticamente.
3. Selecione o target `VulkanBootstrap` (ou `Myve`) e compile (`F5` ou `Ctrl + Shift + B`).

#### Linux / CLI
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -G Ninja
cmake --build build -j$(nproc)
./build/Myve
