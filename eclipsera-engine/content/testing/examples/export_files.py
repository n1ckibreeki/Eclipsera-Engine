import os

def files_to_single_header(file_paths, out_file="preloaded_scripts.h"):
    guard = "PRELOADED_SCRIPTS_H"

    with open(out_file, "w", encoding="utf-8") as f:
        f.write(f"#ifndef {guard}\n")
        f.write(f"#define {guard}\n\n")

        for i, path in enumerate(file_paths, start=1):
            with open(path, "rb") as infile:
                content = infile.read()

            name = f"preloaded_script_{i}"

            f.write(f"// From {os.path.basename(path)}\n")
            bytes_str = ", ".join(str(b) for b in content)
            f.write(f"const unsigned char {name}[] = {{ {bytes_str} }};\n")
            f.write(f"const size_t {name}_len = sizeof({name});\n\n")

        f.write(f"#endif // {guard}\n")


if __name__ == "__main__":
    # Explicitly ordered list of .lua files
    lua_files = [
        "visual-darkhouse.lua",
        "visual-tempform.lua",
        "visual-wireframe-sphere.lua",
        "part_example.lua",
    ]
    files_to_single_header(lua_files, out_file="preloaded_scripts.h")
