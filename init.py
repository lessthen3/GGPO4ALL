import subprocess
import os
import argparse
import platform

def CreateColouredText(fp_SampleText: str, fp_DesiredColour: str) -> str:

    fp_DesiredColour = fp_DesiredColour.lower()

    f_ListOfColours = {
        "black": '\033[30m', "red": '\033[31m', "green": '\033[32m',
        "yellow": '\033[33m', "blue": '\033[34m', "magenta": '\033[35m',
        "cyan": '\033[36m', "white": '\033[37m'
    }

    if fp_DesiredColour not in f_ListOfColours:
        print(CreateColouredText("[Warning]: no valid input detected for CreateColouredText, returned original text in all lower-case", "yellow"))
        return fp_SampleText
    
    else:
        return f"{f_ListOfColours.get(fp_DesiredColour, '')}{fp_SampleText}\033[0m"

def detect_platform() -> str:
    system = platform.system()
    if system == "Windows":
        return "win"
    elif system == "Darwin":
        return "osx"
    elif system == "Linux":
        return "linux"
    else:
        raise RuntimeError("Unsupported platform!")

def run_cmake(fp_BuildType: str, fp_Generator: str) -> bool:

    f_PlatformName = detect_platform()
    f_TargetBuildDirectory = f"build/{f_PlatformName}"

    # Ensure the base build directory exists
    os.makedirs(f_TargetBuildDirectory, exist_ok=True)

    f_GeneratorMap = {
        "vs2022": "Visual Studio 17 2022",
        "xcode": "Xcode",
        "ninja": "Ninja",
        "ninja-mc": "Ninja Multi-Config",
        "unix": "Unix Makefiles",
        "unix-cd": "CodeBlocks - Unix Makefiles",
        "unix-eclipse": "Eclipse CDT4 - Unix Makefiles"
    }

    if fp_Generator not in f_GeneratorMap:
        print(CreateColouredText("[ERROR]: Invalid Generator Selected, PLEASE PICK A VALID GENERATOR", "red"))
        return False
    
    #Determine if we need `--config`
    f_IsMultiConfig = fp_Generator in ["vs2022", "xcode", "ninja-mc"]

    f_CMakeConfigCommand = ['cmake', '-S', '.', '-B', 'build', '-G', f_GeneratorMap[fp_Generator]]

    if not f_IsMultiConfig:
        if fp_BuildType == "both":
            print(CreateColouredText("[ERROR]: Invalid build type selected: YOU CANNOT USE BOTH WHEN GENERATING FOR A SINGLE CONFIG GENERATOR", "red"))
            return False
        else:
            f_CMakeConfigCommand += ['-DCMAKE_BUILD_TYPE=' + fp_BuildType.capitalize()]

    #Step 1: CMake Project Generation
    try:
        print(CreateColouredText(f"[INFO]: Running CMake project generation for {f_GeneratorMap[fp_Generator]}...", "green"))

        subprocess.run(
            f_CMakeConfigCommand,
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )

    except subprocess.CalledProcessError as err:
        print(CreateColouredText("[ERROR]: CMake project generation failed!", "red"))
        print(CreateColouredText(err.stdout.decode(), "yellow"))
        print(CreateColouredText(err.stderr.decode(), "yellow"))
        return False

    print(CreateColouredText("[SUCCESS]: CMake project generation completed!", "cyan"))

    #Step 2: Run CMake Build Process
    if not f_IsMultiConfig:
        try:
            print(CreateColouredText(f"[INFO]: Running CMake single config build for {fp_BuildType}...", "green"))

            subprocess.run(
                ['cmake', '--build', 'build'],
                check=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE
            )

        except subprocess.CalledProcessError as err:
            print(CreateColouredText(f"[ERROR]: CMake single config {fp_BuildType} build process failed!", "red"))
            print(CreateColouredText(err.stdout.decode(), "yellow"))
            print(CreateColouredText(err.stderr.decode(), "yellow"))
            return False

        print(CreateColouredText(f"[SUCCESS]: {fp_BuildType} build completed!", "cyan"))

        return True #return immediately since we don't need to go through the --config commands for single config generators

    if( fp_BuildType == "debug" or fp_BuildType == "both" ):
        try:
            print(CreateColouredText("[INFO]: Running CMake build for Debug...", "green"))

            subprocess.run(
                ['cmake', '--build', 'build', '--config', 'Debug'],
                check=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE
            )

        except subprocess.CalledProcessError as err:
            print(CreateColouredText("[ERROR]: CMake debug build process failed!", "red"))
            print(CreateColouredText(err.stdout.decode(), "yellow"))
            print(CreateColouredText(err.stderr.decode(), "yellow"))
            return False

        print(CreateColouredText("[SUCCESS]: Debug build completed!", "cyan"))

    if( fp_BuildType == "release" or fp_BuildType == "both" ):
        try:
            print(CreateColouredText("[INFO]: Running CMake build for Release...", "green"))

            subprocess.run(
                ['cmake', '--build', 'build', '--config', 'Release'],
                check=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE
            )

        except subprocess.CalledProcessError as err:
            print(CreateColouredText("[ERROR]: CMake release build process failed!", "red"))
            print(CreateColouredText(err.stdout.decode(), "yellow"))
            print(CreateColouredText(err.stderr.decode(), "yellow"))
            return False

        print(CreateColouredText("[SUCCESS]: Release build completed!", "cyan"))

    return True

def main() -> bool:

    usage_message = "init.py --[build_type: release, debug or both] -G [desired_generator]"

    parser = argparse.ArgumentParser(
        description=CreateColouredText('Used for Building GGPO4ALL from Source', 'magenta'), 
        usage=usage_message, 
        add_help=True,
        formatter_class=argparse.RawTextHelpFormatter
    )

    parser.add_argument(
        '--release', 
        action='store_true', 
        help=CreateColouredText('Used for a release build', 'magenta')
    )

    parser.add_argument(
        '--debug', 
        action='store_true', 
        help=CreateColouredText('Used for a debug build', 'magenta')
    )

    parser.add_argument(
        '--both', 
        action='store_true', 
        help=CreateColouredText('Used to build both a debug and release build', 'magenta')
    )

    parser.add_argument(
        '-G', 
        nargs=1,
        metavar="[generator]",
        help=CreateColouredText('Used to set the project file generator, options are as follows:', 'magenta') + "\n" + \
                "\t" + CreateColouredText('-G vs2022 ', 'blue') + CreateColouredText('Generates solution for Visual Studio 17 2022', 'cyan') + "\n" + \
                "\t" + CreateColouredText('-G xcode ', 'blue') + CreateColouredText('Generates project files for Xcode', 'cyan') + "\n" + \
                "\t" + CreateColouredText('-G ninja ', 'blue') + CreateColouredText('Generates project files using Ninja', 'cyan') + "\n" + \
                "\t" + CreateColouredText('-G ninja-mc ', 'blue') + CreateColouredText('For Ninja Multi-Config', 'cyan') + "\n" + \
                "\t" + CreateColouredText('-G unix ', 'blue') + CreateColouredText('For Unix Makefiles', 'cyan') + "\n" + \
                "\t" + CreateColouredText('-G unix-eclipse ', 'blue') + CreateColouredText('Generate Unix Makefiles for Eclipse CDT', 'cyan') + "\n" + \
                "\t" + CreateColouredText('-G unix-cd ', 'blue') + CreateColouredText('Generates Unix Makefiles for CodeBlocks', 'cyan')
    )   
    
    args = parser.parse_args()

    f_IsSetupSuccessful = False

    if(not args.debug and not args.release and not args.both):
        print(CreateColouredText("[ERROR]: No valid build type input detected, use -h or --help if you're unfamiliar", "red"))
        return False
        
    if(not args.G):
        print(CreateColouredText("[ERROR]: YOU DIDN'T USE -G FLAG BROTHER", "red"))
        return False

    f_DesiredGenerator = args.G[0].lower() #convert to all lower case for easier handling

    if(args.debug):

        f_IsSetupSuccessful = run_cmake("debug", f_DesiredGenerator)

        if f_IsSetupSuccessful:
            print(CreateColouredText("CMakeLists.txt succesfully read and compiled for debug, your CMake project should be good to go!", "green"))
        
    elif(args.release):

        f_IsSetupSuccessful = run_cmake("release", f_DesiredGenerator)

        if f_IsSetupSuccessful:
            print(CreateColouredText("CMakeLists.txt succesfully read and compiled for release, your CMake project should be good to go!", "green"))

    elif(args.both):

        f_IsSetupSuccessful = run_cmake("both", f_DesiredGenerator)
        
        if f_IsSetupSuccessful:
            print(CreateColouredText("CMakeLists.txt succesfully read and compiled for debug and release, your CMake project should be good to go!", "green"))

    if not f_IsSetupSuccessful:
        return False

    print(CreateColouredText("done!", "magenta"))
    return True


if __name__ == "__main__":

    if platform.system() == "Windows":
        os.system('color') #enable ANSI colour codes

    if not main():
        print(CreateColouredText("[ERROR]: execution of full build process was unsuccessful", "red"))

#Rawr OwO