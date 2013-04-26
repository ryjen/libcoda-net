project "arg3net"
    kind "StaticLib"
    files {
        "*.h",
        "*.cpp",
    }
    excludes {
        "*.test.cpp"
    }