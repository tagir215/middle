import os


def refactor():
    print("hello world") 
    template_lines = []
    with open("component_template.h", 'r') as file:
        template_lines = file.readlines()

    print(template_lines)

    directory = "../../assets/components"
    for filename in os.listdir(directory):
        if '.cpp' in filename:
            continue
        component_name = os.path.splitext(filename)[0]
        result = []
        for line in template_lines:
            line = line.replace("/*componentName*/", component_name)
            result.append(line)

        filepath = os.path.join(directory, filename)
        with open (filepath, 'w') as file:
            file.writelines(result)
            print(filename)


if __name__ == '__main__':
    refactor()
