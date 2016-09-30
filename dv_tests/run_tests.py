#! /usr/bin/env python3

import sys, string, glob, argparse, os
import tempfile
from subprocess import *
from os.path import *

test_suites = [
"basic/",
"io/",
"matrix/",
#"modules/",
"pca/",
"plot/",
"sort",
"cmd_line"
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
	try:
		ld_lib_paths = [os.path.abspath(p) for p in os.environ['LD_LIBRARY_PATH'].split(':')]
		os.environ['LD_LIBRARY_PATH'] = ':'.join(ld_lib_paths)
	except:
		pass


	tests = []
	for suite in args.tests:
		tests += find_files(suite, ['.dvtest', '.dvscript'], [])
	
	topdir = os.getcwd()

	ret = 0
	for test in tests:
		#print(test)
		location, tmp, testfile = test.rpartition('/')
		testname, extension = testfile.split('.')
		os.chdir(location)
		filename = testfile

		if extension == "dvscript":
			tmpfile = tempfile.NamedTemporaryFile('w', delete=False)
			tmpfile.file.write(open(testfile, 'r').read().replace('DAVINCI_EXECUTABLE', args.davinci))
			tmpfile.close()
			filename = tmpfile.name

		proc = Popen([args.davinci, "-fqv0", filename], stdout=PIPE, universal_newlines=True)
		#proc = Popen(["valgrind", "--leak-check=full", "-v", args.davinci, "-fqv0", filename], stdout=PIPE, universal_newlines=True)

		out, err = proc.communicate()
		rc = proc.returncode
		if rc == 0:
			#print('{: <40}'.format(location+'/'+testname), "..... passed")
			pass
		elif rc == 99:
			print('{: <40}'.format(location+'/'+testname), "..... skipped")
		else:
			print('{: <40}'.format(location+'/'+testname), "..... failed")
			print('stdout:\n{0}\nstderr:\n{1}'.format(out, err))
			ret = 1

		os.chdir(topdir)

	exit(ret)






if __name__ == "__main__":
	main()
