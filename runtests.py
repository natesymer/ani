#!/usr/bin/python3

import sys
import os
from subprocess import Popen, PIPE, STDOUT, DEVNULL

if len(sys.argv) != 3:
  print("Invalid arguments")
  exit(0)

decaf_executable = sys.argv[1]
tests_dir = sys.argv[2] #"~mbailey/310/tcheck/"

decaf_executable = os.path.abspath(os.path.expanduser(decaf_executable))
tests_dir = os.path.abspath(os.path.expanduser(tests_dir))

failed = []
timed_out = []
succeeded = []

for filename in os.listdir(tests_dir):
  pth = os.path.join(tests_dir, filename)
  fh = open(pth, "r")
  test_str = str.encode(fh.read())
  fh.close()
  p = Popen([decaf_executable, '-t'], stdin=PIPE, stdout=DEVNULL, stderr=DEVNULL)
  p.communicate(input=test_str)
  try:
    p.wait(timeout = 10)
  except TimeoutExpired:
    timed_out.append(filename)
    
  if p.returncode == 1:
    failed.append(filename)
  elif p.returncode == 0:
    succeeded.append(filename)

succeeded.sort()
failed.sort()
timed_out.sort()

if len(succeeded) > 0:
  print("SUCCEEDED: ", succeeded)

if len(failed) > 0:
  print("FAILED: ", failed)

if len(timed_out) > 0:
  print("TIMED OUT: ", timed_out) 

