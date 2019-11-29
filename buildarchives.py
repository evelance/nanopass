#!/usr/bin/env python3
import os, sys, zipfile, subprocess, shutil
from os import path as p

# USAGE
if len(sys.argv) < 2 or not (sys.argv[1] == "windows" or sys.argv[1] == "source"):
    sys.exit("Usage: python3 %s type\r\n"
             "    source:  Build an archive that includes all files in this\r\n"
             "             directory except out, .git and the archive itself\r\n"
             "    windows: Build an archive that includes all files as above,\r\n"
             "             uses make.bat to compile the program for 32 and 64\r\n"
             "             bit and adds the resulting executables." % __file__)

# Create archive of directory
def create_zip(directory, fullarcname, ignore_prefixes, extra_files):
    archive = zipfile.ZipFile(fullarcname, "w", zipfile.ZIP_DEFLATED)
    print(p.basename(fullarcname))
    for root, dirs, files in os.walk(directory):
        for file in files:
            full = p.join(root, file)
            legal = True
            for omit in ignore_prefixes:
                if full.startswith(omit):
                    legal = False
                    break
            if legal:
                arcname = p.relpath(full, directory)
                print(" + %s" % p.relpath(full, directory))
                archive.write(full, arcname)
    for (file, arcname) in extra_files:
        print(" + %s" % arcname)
        archive.write(file, arcname)
    archive.close()
    print("Archive '%s' (%.2f KB) created." % (p.basename(fullarcname), os.stat(fullarcname).st_size / 1024.))

# All paths must be relative to the location of this script
here = p.dirname(p.realpath(__file__))
src_zip = p.join(here, "nanopass_source.zip")
win_zip = p.join(here, "nanopass_windows.zip")
ignore_prefixes = [
    p.join(here, "out", ""),
    p.join(here, ".git", ""),
    p.join(here, ".gitmodules"),
    p.join(here, "passwords.npw"),
    p.join(here, "src", "tabtree", ".git"),
    p.join(here, "src", "tabtree", "main.cpp"),
    p.join(here, src_zip),
    p.join(here, win_zip) ]

# Create archive with source code
if sys.argv[1] == "source":
    create_zip(here, src_zip, ignore_prefixes, [])

# Create archive with source code and Windows executables
if sys.argv[1] == "windows":
    exe_32  = p.join(here, "out", "windows32", "nanopass.exe")
    exe_64  = p.join(here, "out", "windows64", "nanopass.exe")
    subprocess.call([p.join(here, "make.bat"), "clean"])
    subprocess.call([p.join(here, "make.bat"), "32"])
    if not p.isfile(exe_32):
        sys.exit("32-bit output binary is missing")
    subprocess.call([p.join(here, "make.bat"), "64"])
    if not p.isfile(exe_64):
        sys.exit("64-bit output binary is missing")
    create_zip(here, win_zip, ignore_prefixes, [(exe_32, "Nanopass-32bit.exe"), (exe_64, "Nanopass-64bit.exe")])

print("Done.")

