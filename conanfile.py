from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain, cmake_layout


class DrogonExperiment(ConanFile):
    name = "drogon-experiment"
    version = "0.1.0"
    settings = "os", "compiler", "build_type", "arch"

    def requirements(self):
        self.requires("drogon/1.9.8")
        self.test_requires("gtest/1.15.0")

    def configure(self):
        self.options["drogon"].with_boost = False
        self.options["drogon"].with_postgres = True
        self.options["drogon"].with_mysql = False
        self.options["drogon"].with_sqlite = False
        self.options["drogon"].with_redis = False

    def layout(self):
        cmake_layout(self)

    def generate(self):
        CMakeToolchain(self).generate()
        CMakeDeps(self).generate()
