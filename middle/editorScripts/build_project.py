import subprocess
from pathlib import Path

def coords_to_cpp_literal(location_string):
    crd = [f"{x}f" for x in location_string.split(",")]
    cpp_literal = "{" + ",".join(crd) + "}"
    return cpp_literal

def build():
    build_dir = "build"

    project_root = Path(__file__).resolve().parents[2]
    build_dir = project_root / "build"

    subprocess.run(
        ["cmake", ".."],
        cwd=build_dir,
        check=True
    )

    subprocess.run(
        ["cmake", "--build", "."],
        cwd=build_dir,
        check=True
    )
    
if __name__=='__main__':
    build()
