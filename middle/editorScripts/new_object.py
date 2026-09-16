import sys
from build_project import build, coords_to_cpp_literal


def generate_obj(name, location):
    path_to_array = "../src/program/init_game.cpp"
    processed_lines = []

    inside_enum = False
    inside_array = False

    with open(path_to_array, "r") as f:
        lines = f.readlines()
        index = 0
        similar_name_count = 0
        name = name.upper()

        for line in lines:
            if name in line:
                similar_name_count += 1

        if similar_name_count > 0:
            name = name + "_" + str(similar_name_count + 1)


        for line in lines:
            if 'enum' in line:
                inside_enum = True
            if inside_enum and '}' in line:
                processed_lines.append(f"\t{name},\n")
                inside_enum = False

            if 'update' in line:
                inside_array = True

            if inside_array and 'end' in line:
                processed_lines.append("\n");
                cpp_literal = coords_to_cpp_literal(location)
                processed_lines.append(f"\tsphere({name},\n");
                processed_lines.append(f"\t\t{cpp_literal}\n")
                processed_lines.append(f"\t);\n")
                inside_array = False

            processed_lines.append(line)
            index += 1

    if len(processed_lines) == 0:
        print("ERROR: PROCESSED LINES EMPTY")
        return

    with open(path_to_array, "w") as f:
        f.writelines(processed_lines)



if __name__ == '__main__':
    name = sys.argv[1]
    location = sys.argv[2]
    generate_obj(name, location)
    #build()
    print("object added")
