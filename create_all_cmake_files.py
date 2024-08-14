import os
import sys
from argparse import ArgumentParser
import subprocess

parser = ArgumentParser()
parser.add_argument("subsystem",type=str)
options = parser.parse_args()



def run_buildfile2cmake(arg):
    if not os.path.exists(arg+"/BuildFile.xml"):
        print("Could not find ",arg+"/BuildFile.xml")
        return False
    if not os.path.exists(arg+"/src"):
        print("Could not find ",arg+"/src")
        return False
    # Construct the command to run the shell script with the given argument
    command = ['python3', 'buildfile2cmake.py', arg+"/BuildFile.xml"]
    
    # Run the shell script and capture its output
    result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        
    # Check if the script ran successfully
    if result.returncode == 0:
        # Write the output to 'CMakeLists.txt.test'
        with open(arg+'/CMakeLists.txt', 'w') as output_file:
            output_file.write(result.stdout)
    else:
        # Print the error message if the script failed
        raise RuntimeError(f"Unable to open file: {result.stderr}")
    return True

def processSubsystem(subName):

    packages = []
    for dir in os.listdir(subName):
        item_path = os.path.join(subName, dir)
        if os.path.isdir(item_path):
            if run_buildfile2cmake(item_path):
                packages.append(dir) 
            

    makefile = open(os.path.join(subName, 'CMakeLists.txt'),'w')
    for p in packages:
        makefile.write('add_subdirectory({})\n'.format(p))
    makefile.write("\nadd_custom_target({}_all)\n".format(subName))
    makefile.write("\nadd_dependencies({}_all\n".format(subName))
    for p in packages:
        makefile.write("  {}{}\n".format(subName,p))
    makefile.write(")\n")
    makefile.close()
            

processSubsystem(options.subsystem)
