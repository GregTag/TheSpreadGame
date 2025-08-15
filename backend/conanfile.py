from conan import ConanFile
from conan.tools.cmake import cmake_layout


class SpreadServerConan(ConanFile):
    name = "spread_server"
    version = "0.0.0"
    description = "Conan consumer for SpreadServer app"
    license = "MIT"
    settings = "os", "compiler", "build_type", "arch"

    # Conan v2 generators for CMake integration
    generators = ("CMakeDeps", "CMakeToolchain")

    # Project dependencies from ConanCenter
    requires = (
        "boost/1.88.0",
        "nlohmann_json/3.12.0",
    )

    # Optional: tweak Boost to avoid unnecessary components
    default_options = {
        # Keep as shared/static default; leave header_only=False since we need Boost.System
        "boost/*:without_python": True,
        "boost/*:without_math": True,
        "boost/*:without_graph": True,
        "boost/*:without_graph_parallel": True,
        "boost/*:without_locale": True,
        "boost/*:without_log": True,
        "boost/*:without_mpi": True,
        "boost/*:without_nowide": True,
        "boost/*:without_program_options": True,
        "boost/*:without_serialization": True,
        "boost/*:without_test": True,
        "boost/*:without_timer": True,
        "boost/*:without_type_erasure": True,
        "boost/*:without_wave": True,
        "boost/*:with_stacktrace_backtrace": False,
    }

    def layout(self):
        # Standard CMake layout: build/ for artifacts
        cmake_layout(self)
