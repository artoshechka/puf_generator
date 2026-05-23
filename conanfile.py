from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps


class PufGenerator(ConanFile):
    name = "puf_generator"
    version = "0.1.0"
    settings = "os", "compiler", "build_type", "arch"

    def requirements(self):
        # На baremetal-целях (микроконтроллерах) тесты не собираются,
        # поэтому зависимость от gtest не требуется.
        if self.settings.os != "baremetal":
            self.requires("gtest/1.14.0")

    def generate(self):
        # Генерируем тулчейн и файлы зависимостей CMake
        # для последующего запуска сборщика.
        tc = CMakeToolchain(self)
        tc.generate()
        deps = CMakeDeps(self)
        deps.generate()
