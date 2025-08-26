from PIL import Image
import sys
import os

def ico32_to_header(input_path, out_path, varname="icon_ico"):
    # load and resize
    img = Image.open(input_path).convert("RGBA")
    if img.size != (32, 32):
        img = img.resize((32, 32), Image.LANCZOS)

    # save a temporary .ico
    tmp_ico = out_path + ".tmp.ico"
    img.save(tmp_ico, format="ICO", sizes=[(32, 32)])

    # read raw bytes
    with open(tmp_ico, "rb") as f:
        data = f.read()
    os.remove(tmp_ico)

    # dump to header
    with open(out_path, "w") as f:
        f.write(f"// Generated from {input_path}, 32x32 ICO\n")
        f.write(f"unsigned char {varname}[] = {{\n")
        for i, b in enumerate(data):
            if i % 12 == 0:
                f.write("    ")
            f.write(f"{b}, ")
            if i % 12 == 11:
                f.write("\n")
        if len(data) % 12 != 0:
            f.write("\n")
        f.write("};\n")
        f.write(f"const unsigned int {varname}_len = {len(data)};\n")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python ico32_to_header.py input.(ico|png) output.h")
        sys.exit(1)
    ico32_to_header(sys.argv[1], sys.argv[2])
