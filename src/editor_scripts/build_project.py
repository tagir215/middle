import subprocess

def coords_to_cpp_literal(location_string):
    crd = [f"{x}f" for x in location_string.split(",")]
    cpp_literal = "{" + ",".join(crd) + "}"
    return cpp_literal

def build():
    sln_path = "middle.sln"
    platform = "x64"
    configuration = "Debug"
    subprocess.run([
        "msbuild",
        sln_path,
        f"/p:Configuration={configuration}",
        f"/p:Platform={platform}",
        ], check=True)

if __name__=='__main__':
    build()
