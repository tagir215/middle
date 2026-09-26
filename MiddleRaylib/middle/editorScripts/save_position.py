import sys
from build_project import build, coords_to_cpp_literal

def save(names, locations):
    path_to_array = "../src/program/init_game.cpp"
    processed_lines = []

    name_array = names.split(";")
    location_array = locations.split(";")

    with open(path_to_array, 'r') as f:
        lines = f.readlines()
        found_something = False
        name_found = ""
        index_found = -1
        for line in lines:
            position_saved = False
            for index, name in enumerate(name_array):
                if name in line:
                    name_found = name
                    index_found = index
                    found_something = True

            # position is stored within {}  so parse for { lol
            if found_something and "{" in line:
                location = location_array[index]
                cpp_literal = coords_to_cpp_literal(location)
                processed_lines.append(f"\t\{cpp_literal};\n")
                found_something = False
                position_saved = True

            # if line was edited skip copying this line thing
            if not position_saved:
                processed_lines.append(line)

    if len(processed_lines) == 0:
        print("ERROR: PROCESSED LINES EMPTY")
        return

    with open(path_to_array, 'w') as f:
        f.writelines(processed_lines)

if __name__=="__main__":
    names = sys.argv[1]
    locations = sys.argv[2]
    save(names, locations)
    # build()
    print("position(s) saved")
