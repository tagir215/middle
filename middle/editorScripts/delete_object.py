import sys
from build_project import build

def delete(string):
    path_to_array = "../src/program/init_game.cpp"

    names = string.replace(" ", "").split(",")

    print(names)

    processed_lines = []
    with open(path_to_array, 'r') as f:
        lines = f.readlines()
        am_within_arguments = False
        for line in lines:
            # skip if i am inside arguments of the initialized thing 
            if am_within_arguments and ")" in line:
                am_within_arguments = False
                print(f"i am here {line}")
                continue
            if am_within_arguments:
                print(f"still am here {line}")
                continue

            # check if the name exists within the line
            name_exist = False
            for name in names:
                if name in line:
                    name_exist = True
                    break

            # we're checking here if we're opening arguments here, also we can continue
            if name_exist:
                if name_exist and "(" in line:
                    am_within_arguments = True
                print(f"{name} exists in {line}")
                continue

            # if other cases are false we can copy the old file
            processed_lines.append(line)

    if len(processed_lines) == 0:
        print("ERROR: PROCESSED LINES EMPTY")
        print(names)
        return

    consecutive_emptiness = 0
    cleaned_lines = []
    for line in processed_lines:
        if not line.strip():
            consecutive_emptiness += 1
            if consecutive_emptiness >= 2:
                continue
        else:
            consecutive_emptiness = 0

        cleaned_lines.append(line)


    with open(path_to_array, 'w') as f:
        f.writelines(cleaned_lines)

if __name__=="__main__":
    name = sys.argv[1]
    delete(name)
    #build()
    print("object(s) deleted")
