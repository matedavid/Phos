import os
import sys

if __name__ == "__main__":
    args = sys.argv[1:]

    if len(args) < 1:
        print("Usage: python configure_mono.py <mono_directory>")
        sys.exit(1)

    mono_path = args[0]

    flags = ""
    if sys.platform == "win32":
        flags = "--host=x86_64-w64-mingw32 --enable-msvc --disable-boehm"

    os.system(f"cd {mono_path}")
    os.system(f"./autogen.sh --prefix={mono_path}/build ${flags}")
    os.system(f"make")
    os.system(f"make install")
