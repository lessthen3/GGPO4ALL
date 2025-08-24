import subprocess
import os
import argparse
import platform
import shutil
import sys

from shutil import which

############# Pretty Text Utility Function UwU #############

def CreateColouredText(fp_SampleText: str, fp_DesiredColour: str) -> str:

    fp_DesiredColour = fp_DesiredColour.lower()

    f_ListOfColours = {
        "black": '\033[30m', "red": '\033[31m', "green": '\033[32m',
        "yellow": '\033[33m', "blue": '\033[34m', "magenta": '\033[35m',
        "cyan": '\033[36m', "white": '\033[37m',

        "bright black": '\033[90m', "bright red": '\033[91m', "bright green": '\033[92m',
        "bright yellow": '\033[93m', "bright blue": '\033[94m', "bright magenta": '\033[95m',
        "bright cyan": '\033[96m', "bright white": '\033[97m'
    }

    if fp_DesiredColour not in f_ListOfColours:
        print(CreateColouredText("[Warning]: no valid input detected for CreateColouredText, returned original text in all lower-case", "yellow"))
        return fp_SampleText
    
    else:
        return f"{f_ListOfColours.get(fp_DesiredColour, '')}{fp_SampleText}\033[0m"

############# Utility for Validating Required Build Tools #############

def ensure_tool_installed(fp_ToolName: str) -> bool:

    if which(fp_ToolName) is None:
        print(CreateColouredText(f"[ERROR]: Required tool '{fp_ToolName}' not found in PATH", "red"))
        return False
    
    else:
        return True

############# Run command for live console feed #############

"""
    Runs a subprocess command and streams stdout live.
    Raises CalledProcessError if the command fails,
    attaching the full output to the exception.
"""

def run_command_with_live_output(fp_Command, fp_WorkingDirectory=".") -> None:

    f_Process = subprocess.Popen(
        fp_Command,
        cwd=fp_WorkingDirectory,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        universal_newlines=True,
        encoding="utf-8", 
        errors="replace"  
    )

    f_OutputLines = []

    try:
        for line in f_Process.stdout:
            sys.stdout.write(line)
            f_OutputLines.append(line)

        f_Process.wait()

        if f_Process.returncode != 0:
            raise subprocess.CalledProcessError(
                f_Process.returncode,
                fp_Command,
                output=''.join(f_OutputLines)
            )

    finally:
        f_Process.stdout.close()

############# Main CMake Function #############

def run_cmake(fp_BuildType: str, fp_Generator: str) -> bool:

    f_GeneratorMap = {
        "vs2022": "Visual Studio 17 2022",
        "vs2019": "Visual Studio 16 2019",
        "vs2017": "Visual Studio 15 2017",
        "vs2015": "Visual Studio 14 2015",

        "xcode": "Xcode",

        "ninja": "Ninja", #everything under here is untested so uh goodluck w that uwu
        "ninja-mc": "Ninja Multi-Config",

        "unix": "Unix Makefiles",
        "unix-cb": "CodeBlocks - Unix Makefiles",
        "unix-eclipse": "Eclipse CDT4 - Unix Makefiles",

        "mingw": "MinGW Makefiles",
        "msys": "MSYS Makefiles",
        "nmake": "NMake Makefiles",
        "nmake-jom": "NMake Makefiles JOM"
    }

    ############# Ensure Valid Generator was Selected #############

    if fp_Generator not in f_GeneratorMap:
        print(CreateColouredText("[ERROR]: Invalid Generator Selected, PLEASE PICK A VALID GENERATOR", "red"))
        return False
    
    ############# Determine if Generator is Single Config #############
    
    f_IsMultiConfig = fp_Generator in ["vs2022", "vs2019", "vs2017", "vs2015", "xcode", "ninja-mc"]

    f_CMakeConfigCommand = ['cmake', '-S', '.', '-B', 'build', '-G', f_GeneratorMap[fp_Generator], '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON']

    if not f_IsMultiConfig:

        if fp_BuildType == "Release and Debug": #Don't allow "both" configs for single config generators uwu
            
            print(CreateColouredText("[ERROR]: Invalid build type selected: YOU CANNOT USE BOTH WHEN GENERATING FOR A SINGLE CONFIG GENERATOR", "red"))
            return False
        
        else:
            f_CMakeConfigCommand += ['-DCMAKE_BUILD_TYPE=' + fp_BuildType.capitalize()]

    ############# Generate CMake Project #############

    try:
        print(CreateColouredText(f"[INFO]: Running CMake project generation for {f_GeneratorMap[fp_Generator]}...", "green"))

        run_command_with_live_output(f_CMakeConfigCommand)

    except subprocess.CalledProcessError as err:
        print(CreateColouredText("[ERROR]: CMake project generation failed!", "red"))
        print(CreateColouredText(err.output, "yellow"))
        return False

    print(CreateColouredText("[SUCCESS]: CMake project generation completed!", "cyan"))

    ############# Run CMake Build Process for Single Config #############

    if not f_IsMultiConfig:
        try:
            print(CreateColouredText(f"[INFO]: Running CMake single config build for {fp_BuildType}...", "green"))

            run_command_with_live_output(['cmake', '--build', 'build'])

        except subprocess.CalledProcessError as err:
            print(CreateColouredText(f"[ERROR]: CMake single config {fp_BuildType} build process failed!", "red"))
            print(CreateColouredText(err.output, "yellow"))

            return False

        print(CreateColouredText(f"\n[SUCCESS]: {fp_BuildType} build completed!", "cyan"))

        return True #return immediately since we don't need to go through the --config commands for single config generators

    ############# Run Debug Build #############

    if( fp_BuildType == "Debug" or fp_BuildType == "Release and Debug" ):
        try:
            print(CreateColouredText("[INFO]: Running CMake build for Debug...", "green"))

            run_command_with_live_output(['cmake', '--build', 'build', '--config', 'Debug'])

        except subprocess.CalledProcessError as err:
            print(CreateColouredText("[ERROR]: CMake debug build process failed!", "red"))
            print(CreateColouredText(err.output, "yellow"))

            return False

        print(CreateColouredText("[SUCCESS]: Debug build completed!", "cyan"))

    ############# Run Release Build #############

    if( fp_BuildType == "Release" or fp_BuildType == "Release and Debug" ):
        try:
            print(CreateColouredText("[INFO]: Running CMake build for Release...", "green"))

            run_command_with_live_output(['cmake', '--build', 'build', '--config', 'Release'])

        except subprocess.CalledProcessError as err:
            print(CreateColouredText("[ERROR]: CMake release build process failed!", "red"))
            print(CreateColouredText(err.output, "yellow"))

            return False

        print(CreateColouredText("[SUCCESS]: Release build completed!", "cyan"))

    ############# Success! #############

    print(CreateColouredText("\n[INFO]: Your CMake project should be good to go!", "green"))

    return True

############# Main Function #############

def main() -> bool:

    ############# Check for Required Build Tools in PATH #############
    
    if not ensure_tool_installed("cmake"): 
        return False

    ############# Setup Parser #############

    usage_message = \
        CreateColouredText("init.py ", 'bright magenta') + \
        CreateColouredText("--[build_type: release, debug or both] ", "bright blue") + \
        CreateColouredText("-G [desired_generator] ", "blue")

    parser = argparse.ArgumentParser(
        description=CreateColouredText('Used for Building Peach-E from Source', 'bright green'), 
        usage=usage_message, 
        add_help=True,
        formatter_class=argparse.RawTextHelpFormatter
    )

    ############# Set Parser Arguments #############

    parser.add_argument(
        '--release', 
        action='store_true', 
        help=CreateColouredText('Used for a release build', 'bright magenta')
    )

    parser.add_argument(
        '--debug', 
        action='store_true', 
        help=CreateColouredText('Used for a debug build', 'bright magenta')
    )

    parser.add_argument(
        '--both', 
        action='store_true', 
        help=CreateColouredText('Used to build both a debug and release build', 'bright magenta')
    )

    parser.add_argument(
        '--clean', 
        action='store_true', 
        help=CreateColouredText('Used to clean build artifacts from a previous run', 'bright magenta')
    )

    parser.add_argument(
        '-G', 
        nargs=1,
        metavar="[generator]",
        help=CreateColouredText('Used to set the project file generator, options are as follows:', 'bright magenta') + "\n" + \
                "\t" + CreateColouredText('-G vs2015 --> vs2022 ', 'blue') + CreateColouredText('Generates solution for Visual Studio 2015 - 2022', 'cyan') + "\n" + \
                
                "\t" + CreateColouredText('-G xcode ', 'blue') + CreateColouredText('Generates project files for Xcode', 'cyan') + "\n" + \
                
                "\t" + CreateColouredText('-G ninja ', 'blue') + CreateColouredText('Generates project files using Ninja', 'cyan') + "\n" + \
                "\t" + CreateColouredText('-G ninja-mc ', 'blue') + CreateColouredText('For Ninja Multi-Config', 'cyan') + "\n" + \
                
                "\t" + CreateColouredText('-G unix ', 'blue') + CreateColouredText('For Unix Makefiles', 'cyan') + "\n" + \
                "\t" + CreateColouredText('-G unix-eclipse ', 'blue') + CreateColouredText('Generate Unix Makefiles for Eclipse CDT', 'cyan') + "\n" + \
                "\t" + CreateColouredText('-G unix-cb ', 'blue') + CreateColouredText('Generates Unix Makefiles for CodeBlocks', 'cyan') + "\n" + \

                "\t" + CreateColouredText('-G mingw ', 'blue') + CreateColouredText('Generates MinGW Makefiles', 'cyan') + "\n" + \
                "\t" + CreateColouredText('-G msys ', 'blue') + CreateColouredText('Generates MSYS Makefiles', 'cyan') + "\n" + \
                "\t" + CreateColouredText('-G nmake ', 'blue') + CreateColouredText('Generates NMake Makefiles', 'cyan') + "\n" + \
                "\t" + CreateColouredText('-G nmake-jom ', 'blue') + CreateColouredText('Generates JOM Makefiles', 'cyan')
    )   
    
    args = parser.parse_args()
    
    ############# Validate Build Config #############

    f_BuildType = "nothing"

    if(args.debug):
        f_BuildType = "Debug"

    elif(args.release):
        f_BuildType = "Release"

    elif(args.both):
        f_BuildType = "Release and Debug"

    else:
        print(CreateColouredText("[ERROR]: No valid build type input detected, use -h or --help if you're unfamiliar", "red"))
        return False
    
    ############# Check for Generator #############
        
    if(not args.G):
        print(CreateColouredText("[ERROR]: YOU DIDN'T USE -G FLAG BROTHER", "red"))
        return False

    f_DesiredGenerator = args.G[0].lower() #convert to all lower case for easier handling

    ############# Check for --clean flag #############

    if args.clean:
        shutil.rmtree('build', ignore_errors=True)

    ############# Detect Platform #############

    f_CurrentPlatform = platform.system()

    ############# Run Build Fingers Crossed >w< #############

    if not run_cmake(f_BuildType, f_DesiredGenerator):
            return False
    
    ############# Report Build Stats #############

    print(CreateColouredText(f"[INFO]: Final Build Summary: \n", "bright green"))
    print(CreateColouredText(f"Generator: {f_DesiredGenerator}", "bright magenta"))
    print(CreateColouredText(f"Build Type: {f_BuildType}", "bright magenta"))
    print(CreateColouredText(f"Platform: {f_CurrentPlatform}\n", "bright magenta"))

    return True

############# Main Caller #############

if __name__ == "__main__":

    if platform.system() == "Windows": #enable ANSI colour codes for Windows Console
        os.system('color') 

    if not main():
        print(CreateColouredText("[ERROR]: execution of full build process was unsuccessful\n", "red"))
    else:
        print(CreateColouredText("done!\n", "magenta"))


#Rawr OwO
