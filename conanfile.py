from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps


class PufGenerator(ConanFile):
    name = "puf_generator"
    version = "0.1.0"
    settings = "os", "compiler", "build_type", "arch"

    def requirements(self):
        if self.settings.os != "baremetal":
            self.requires("gtest/1.14.0")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()
        deps = CMakeDeps(self)
        deps.generate()
