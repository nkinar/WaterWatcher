import glob
import os
import sys
from pathlib import Path
import subprocess

# Constants
CONVERT_VBS = 'convert.vbs'
CSCRIPT_APP = 'cscript'
STEP_EXTENSION = 'STEP'
IPT_EXTENSION = 'ipt'


def find_files_ext(directory, ext):
    out = []
    dir_path_orig = os.path.dirname(os.path.realpath(__file__))
    os.chdir(directory)
    for f in glob.glob("*." + ext):
        out.append(f)
    os.chdir(dir_path_orig)
    return out


def main():
    if len(sys.argv) != 2:
        print('step-to-ipt.py [directory]')
        return
    directory = os.path.abspath(sys.argv[1])
    files = find_files_ext(directory, STEP_EXTENSION )
    n = len(files)
    for k in range(n):
        print('Processing file {0} out of {1}: {2}'.format(k+1, n, files[k]))
        full_filename_in = os.path.join(directory, files[k])
        fn_out = Path(files[k]).with_suffix('.' + IPT_EXTENSION)
        full_filename_out = os.path.join(directory, fn_out)
        lst_cmd = [CSCRIPT_APP, '//Nologo', CONVERT_VBS, full_filename_in, full_filename_out]
        run_files = subprocess.run(lst_cmd)
        if run_files.returncode != 0:
            print("Exit code: {0}".format(run_files.returncode))


if __name__ == '__main__':
    main()

