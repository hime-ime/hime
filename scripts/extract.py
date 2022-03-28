#!/usr/bin/env python

# Script to extract user dictionary into McBopomofo's format
#
# Usage:
#
# 1) export tsin dict into ~/tsin32
#
# 2) print diff between original dictionary and exported dict
#
# ./scripts/extract.py data/tsin.src ~/tsin32 > ~/tsin32-user-dict.txt

import sys
from typing import List


def drop_freq(lines: List[str]) -> List[str]:
    # drop word frequency (last one)
    return [' '.join(line.split()[:-1]) for line in lines]


def conv(line: str) -> str:
    key, *value = line.split()
    # McBopomofo uses '-' as separator
    # https://github.com/openvanilla/McBopomofo/wiki/%E4%BD%BF%E7%94%A8%E6%89%8B%E5%86%8A#%E6%89%8B%E5%8B%95%E5%8A%A0%E8%A9%9E
    bopomofo = '-'.join(value)
    bopomofo = bopomofo.replace('1', '˙')
    bopomofo = bopomofo.replace('2', 'ˊ')
    bopomofo = bopomofo.replace('3', 'ˇ')
    bopomofo = bopomofo.replace('4', 'ˋ')

    return f'{key} {bopomofo}'


def main() -> None:
    lines = []
    with open(sys.argv[1]) as f:
        lines = f.readlines()

    original = drop_freq(lines)
    original_dict = {l.split()[0]: l for l in original}

    with open(sys.argv[2]) as f:
        lines = f.readlines()

    mine = drop_freq(lines)
    for l in mine:
        key = l.split()[0]
        if key not in original_dict:
            print(conv(l))


if __name__ == '__main__':
    main()
