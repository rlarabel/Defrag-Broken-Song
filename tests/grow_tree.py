#!/usr/bin/env python3

from pathlib import Path
import os
import random
import shutil
import sys

random.seed(sys.argv[2])

with open(sys.argv[1], "rb") as f:
	b = f.read()

FILE_LENGTH = len(b)
SUFFIX = ".bin"

used = 0
file_num = 0

if os.path.isdir("partials"):
    shutil.rmtree("partials")
os.mkdir("partials")

if os.path.isdir("dirs"):
    shutil.rmtree("dirs")
os.mkdir("dirs")

os.chdir("partials")

while used < FILE_LENGTH:
	bite = random.randint(45000, 90000)

	f = open(f"{file_num:02d}" + SUFFIX, "wb")
	f.write(b[:bite])
	f.close()

	b = b[bite:]

	used += bite
	file_num += 1

print(f"  {file_num - 1} FILES CREATED...")
file_num = 0

while True:
	try:
		src = f"{file_num:02d}" + SUFFIX
		os.stat(src)

		chain = 0
		while chain <= 0 or chain >= 5:
			chain = int(round(random.gauss(2.5, 2)) + 1)

		dst = "../dirs/"

		for i in range(0, chain):
			dst += "dir" + str(random.randint(0, 4)) + "/"

		# print("> COPYING FROM " + src + " TO " + dst)
		try:
			Path(dst).mkdir(parents=True, exist_ok=True)
		except FileExistsError as e:
			pass
		except OSError as e:
			continue

		try:
			shutil.copyfile(src, dst + src)
		except OSError as er:
			# print("! I COULDN'T COPY THERE")
			continue

		if random.randint(0, 6) == 6:
			dst += "dir0"
			link_to = ""
			for i in range(0, chain):
				link_to += "../"
			link_to += "dir" + str(random.randint(0, 4))
			try:
				os.symlink(link_to, dst)
				# print("> SYMLINKED " + dst + " TO " + link_to)
			except:
				# print("! ABORTED SYMLINK")
				pass

		file_num += 1
	except FileNotFoundError as e:
		# print("! HORRIBLE ERROR: " + str(e))
		break

os.chdir("..")
shutil.rmtree("partials")

print("√ TREE GROWN")
