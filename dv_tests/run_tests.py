#! /usr/bin/env python3

import re, sys, string, glob, argparse, os
from os.path import *

test_suites = [
"basic/",
"io/",
"matrix/",
"modules/",
"pca/",

#"plot/",
"sort"
]


def find_files(root, filetypes, exclude):          # for a root dir
	files = []
	if not exists(root):
		return files
	if isfile(root):
		files.append(root)
	else:
		for (thisdir, subshere, fileshere) in os.walk(root):    # generate dirs in tree
			#print(thisdir,subshere,fileshere)
			if any(samefile(thisdir, e) for e in exclude):
				continue
			for t in filetypes:
				tmp = glob.glob(thisdir+'/*'+t)
				for f in tmp:
					if any(samefile(f, e) for e in exclude):
						continue

					files.append(f)

	return files




# TODO(rswinkle)
# output formatting, right now just pipe stdout/err to /dev/null but
# I should provide options.  I also should make it prettier and
# provide html output ala
# http://www.robertwinkler.com/CVector/CUnitAutomated-Results.xml
#
# eventually, tie this into CI, and of course fix the failing tests
# also the 64 bit tests are pointless if you can't even run them because
# they use too much memory and crash/freeze your machine.  I'll adjust
# them so they use <= 8 GB of mem and don't take longer than ... 10 min each?

# We'll see


def main():
	parser = argparse.ArgumentParser(description="Run Davinci test suite(s)")
	parser.add_argument("-d", "--davinci", default="davinci", help="specify which davinci to test")
	parser.add_argument("-t", "--tests", nargs=1, default=test_suites, help="manually specify what test (or test directory) to run")

	args = parser.parse_args()
	print(args)


	if args.davinci != "davinci":
		args.davinci = os.path.abspath(args.davinci)
	ld_lib_paths = [os.path.abspath(p) for p in os.environ['LD_LIBRARY_PATH'].split(':')]
	os.environ['LD_LIBRARY_PATH'] = ':'.join(ld_lib_paths)


	tests = []
	for suite in args.tests:
		tests += find_files(suite, ['.dvtest'], [])
	
	topdir = os.getcwd()

	for test in tests:
		location, tmp, name = test.rpartition('/')

		os.chdir(location)

		testlen = len(test)
		print('{: <40}'.format(test[:-7]), "..... ", end='')
		#sys.stdout.flush() #could do away with this in 3.3+

		rc = os.system(args.davinci + " -fqv0 " + name + "> /dev/null 2>&1")
		if rc == 0:
			print("passed")
		elif rc == 99:
			print("skipped")
		else:
			print("failed")

		os.chdir(topdir)






if __name__ == "__main__":
	main()
