#! /usr/bin/env python
# -*- coding: utf-8 -*-

# file mergepo.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Georg Baum

# Full author contact details are available in file CREDITS

# This script takes missing translations from another set of po files and
# merges them into the po files in this source tree.


import os, re, string, sys
import polib


# we do unix/windows line trimming ourselves since it can happen that we
# are on unix, but the file has been written on windows or vice versa.
def trim_eol(line):
    " Remove end of line char(s)."
    if line[-2:-1] == '\r':
        return line[:-2]
    elif line[-1:] == '\r' or line[-1:] == '\n':
        return line[:-1]
    else:
        # file with no EOL in last line
        return line


def read(input):
    " Read utf8 input file and strip lineendings."
    lines = list()
    while 1:
        line = input.readline()
        if not line:
            break
        line = trim_eol(line)
        lines.append(line.decode('UTF-8'))
    return lines


def parse_msg(lines):
    " Extracts msgid or msgstr from lines."
    if len(lines) < 1:
        return ''
    i = lines[0].find('"')
    if i < 0:
        return ''
    msg = lines[0][i:].strip('"')
    for i in range(1, len(lines)):
        msg = msg + lines[i].strip('"')
    return msg


def translate(msgid, msgstr_lines, po2):
    msgstr = parse_msg(msgstr_lines)
    if msgstr != '':
        return 0
    other = po2.find(msgid)
    if not other:
        return 0
    if not other.translated():
        return 0
    msgstr = other.msgstr
    obsolete = (msgstr_lines[0].find('#~') == 0)
    j = msgstr_lines[0].find('"')
    # must not assign to msgstr_lines, because that would not be seen by our caller
    new_lines = polib.wrap(msgstr_lines[0][0:j+1] + msgstr, 76, drop_whitespace = False)
    del msgstr_lines[:]
    for i in range(0, len(new_lines)):
        if i == 0:
            msgstr_lines.append(new_lines[i] + '"')
        elif obsolete:
            msgstr_lines.append('#~ "' + new_lines[i] + '"')
        else:
            msgstr_lines.append('"' + new_lines[i] + '"')
    return 1


def mergepo_polib(target, source):
    changed = 0
    po1 = polib.pofile(target)
    po2 = polib.pofile(source)
    for entry in po1.untranslated_entries():
        other = po2.find(entry.msgid, include_obsolete_entries=True)
        if not other:
            continue
        if other.translated():
            entry.msgstr = other.msgstr
            changed = changed + 1
    if changed > 0:
        po1.save(target)
    return changed


def mergepo_minimaldiff(target, source):
    changed = 0
    po2 = polib.pofile(source)
    target_enc = polib.detect_encoding(target)
    # for utf8 files we can use our self written parser to minimize diffs,
    # otherwise we need to use polib
    if target_enc != 'UTF-8':
        raise
    po1 = open(target, 'rb')
    oldlines = read(po1)
    po1.close()
    newlines = []
    in_msgid = False
    in_msgstr = False
    msgstr_lines = []
    msgid_lines = []
    msgid = ''
    for line in oldlines:
        if in_msgid:
            if line.find('"') == 0 or line.find('#~ "') == 0:
                msgid_lines.append(line)
            else:
                in_msgid = False
                msgid = parse_msg(msgid_lines)
                newlines.extend(msgid_lines)
                msgid_lines = []
        elif in_msgstr:
            if line.find('"') == 0 or line.find('#~ "') == 0:
                msgstr_lines.append(line)
            else:
                in_msgstr = False
                changed = changed + translate(msgid, msgstr_lines, po2)
                newlines.extend(msgstr_lines)
                msgstr_lines = []
                msgid = ''
        if not in_msgid and not in_msgstr:
            if line.find('msgid') == 0 or line.find('#~ msgid') == 0:
                msgid_lines.append(line)
                in_msgid = True
            elif line.find('msgstr') == 0 or line.find('#~ msgstr') == 0:
                if line.find('msgstr[') == 0 or line.find('#~ msgstr[') == 0:
                    # plural forms are not implemented
                    raise
                msgstr_lines.append(line)
                in_msgstr = True
            else:
                newlines.append(line)
    if msgid != '':
        # the file ended with a msgstr
        changed = changed + translate(msgid, msgstr_lines, po2)
        newlines.extend(msgstr_lines)
        msgstr_lines = []
        msgid = ''
    if changed > 0:
        # we store .po files with unix line ends in git,
        # so do always write them even on windows
        po1 = open(target, 'wb')
        for line in newlines:
            po1.write(line.encode('UTF-8') + '\n')
    return changed


def mergepo(target, source):
    if not os.path.exists(source):
        sys.stderr.write('Skipping %s since %s does not exist.\n' % (target, source))
        return
    if not os.path.exists(target):
        sys.stderr.write('Skipping %s since %s does not exist.\n' % (target, target))
        return
    sys.stderr.write('Merging %s into %s: ' % (source, target))
    try:
        changed = mergepo_minimaldiff(target, source)
    except:
        changed = mergepo_polib(target, source)
    sys.stderr.write('Updated %d translations.\n' % changed)


def main(argv):

    toolsdir = os.path.dirname(argv[0])
    podir1 = os.path.normpath(os.path.join(toolsdir, '../../po'))
    if len(argv) <= 1:
        sys.stderr.write('''Usage: %s <dir> [lang] where dir is a directory containing the .po
       files you want to take missing translations from. If lang is not given, all languages
       are translated, otherwise only lang.\n''' % (argv[0]))
    podir2 = os.path.abspath(argv[1])

    if len(argv) > 2:
        name = argv[2] + '.po'
        mergepo(os.path.join(podir1, name), os.path.join(podir2, name))
    else:
        for i in os.listdir(podir1):
            (base, ext) = os.path.splitext(i)
            if ext != ".po":
                continue
            mergepo(os.path.join(podir1, i), os.path.join(podir2, i))

    return 0


if __name__ == "__main__":
    main(sys.argv)