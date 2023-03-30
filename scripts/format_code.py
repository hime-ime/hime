#!/usr/bin/env python3
"""The python script calling clang-format to unify the source code format"""

import os
import re


def get_c_files_under_dir(directory_list: list[str]) -> str:
    """Get all the C/C++ files under the directory list"""
    target_files = ""
    for directory in directory_list:
        for root, _, files in os.walk(directory):
            for file in files:
                if is_c_file(file):
                    target_file = root + "/" + file
                    target_files = target_files + target_file + " "
    return target_files


def is_c_file(file: str) -> bool:
    """Check whether the extension of the file is C/C++ code"""
    return re.search(r"\.(c|cpp|h)$", file) is not None


if __name__ == "__main__":
    directories = ["data", "src"]
    os.system("clang-format-14 -i " + get_c_files_under_dir(directories) + " --verbose")
