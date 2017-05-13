#!/usr/bin/env python3

import sys
import os
import xml.etree.ElementTree as ET
from collections import defaultdict

#from termcolor import colored
# Copy-paste: ---------------------------------------------
ATTRIBUTES = dict(list(zip(
    ['bold', 'dark', '','underline', 'blink', '', 'reverse', 'concealed'],
    list(range(1, 9)))))
del ATTRIBUTES['']

HIGHLIGHTS = dict(list(zip(
    ['on_grey', 'on_red', 'on_green', 'on_yellow', 'on_blue', 'on_magenta', 'on_cyan', 'on_white'],
     list(range(40, 48)))))

COLORS = dict(list(zip(
    ['grey', 'red', 'green', 'yellow', 'blue', 'magenta', 'cyan', 'white'],
     list(range(30, 38)))))

RESET = '\033[0m'

def colored(text, color=None, on_color=None, attrs=None):
    if os.getenv('ANSI_COLORS_DISABLED') is None:
        fmt_str = '\033[%dm%s'
        if color is not None:
            text = fmt_str % (COLORS[color], text)

        if on_color is not None:
            text = fmt_str % (HIGHLIGHTS[on_color], text)

        if attrs is not None:
            for attr in attrs:
                text = fmt_str % (ATTRIBUTES[attr], text)

        text += RESET
    return text

# End of termcolor.colored --------------------------------


index_to_status = {i: status for i, status in 
            enumerate(['notrun', 'run', 'error', 'failure'])}
status_to_index = {v: k for k,v in index_to_status.items()}

colors = {
    'notrun': 'grey',
    'run': 'green',
    'error': 'yellow',
    'failure': 'red',
}

symbols = {
    'notrun': '/',
    'run': '✓',
    'error': '!',
    'failure': 'X',
}

colored_symbols = {k: colored(sym, colors[k], attrs=['bold'])
                   for k, sym in symbols.items()}

# Slight modifications for text
colors['notrun'] = None

def recursive_dict():
    return defaultdict(recursive_dict)

def build_tree(el):
    if el.tag.lower() == 'testcase':
        return el
    else:
        tree = recursive_dict()
        for child in el:
            name = child.attrib['name']
            # Fix for weird gtest naming convention
            name = '__'.join(reversed(name.split('/')))
            pieces = name.split('__')
            result_holder = tree
            for p in pieces[:-1]:
                result_holder = tree[p]
            result_holder[pieces[-1]] = build_tree(child)
        return tree

def status_index(el):
    if isinstance(el, defaultdict):
        return max((status_index(v) for v in el.values()), default=status_to_index['notrun'])
    else:
        failure = status_to_index['failure'] if any(c.tag.lower() == 'failure' for c in el) else 0
        return max(failure, status_to_index[el.attrib['status']])


def print_tree(name, tree, depth=0, maxdepth=30):
    stat = index_to_status[status_index(tree)]
    indent = '  ' + depth*'  '
    if isinstance(tree, defaultdict):
        s = indent + colored_symbols[stat] + ' ' + colored(name, colors[stat])
        if depth+1 >= maxdepth:
            s += ' [' + ' '.join(colored_symbols[index_to_status[status_index(v)]]
                    for v in tree.values()) + ']'
            print(s)
            for name, child in tree.items():
                if status_index(child) > status_to_index['run']:
                    print_tree(name, child, depth=depth+1, maxdepth=maxdepth)
        else:
            print(s)
            for name, child in tree.items():
                print_tree(name, child, depth=depth+1, maxdepth=maxdepth)
    else:
        print(indent + colored_symbols[stat] + ' ' + colored(name, colors[stat]))


if len(sys.argv) <= 1:
    print("Not enough arguments. Usage: print.py <filename>")
    sys.exit(1)

tree = build_tree(ET.parse(sys.argv[1]).getroot())
print_tree("All tests", tree)

