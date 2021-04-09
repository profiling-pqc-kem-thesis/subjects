import gzip
import os.path
from sys import argv

def is_gunzipped(file_path):
  with open(file_path, "rb") as file:
    return file.read(2) == b"\x1f\x8b"

def main(file_path):
  with gzip.open(file_path, "rt") as file:
    for line in file:
      line = str(line).strip()
      if line.startswith("s "):
        symbol = line.split(" ")[1]
        print("symbol:", symbol)
      elif line.startswith("t "):
        ip_index, parent_index = line.split(" ")[1:3]
        print("ip index, parent index:", ip_index, parent_index)
      # TODO: Fix
      # elif line.startswith("i "):
      #   instruction_pointer, module_index = line.split(" ")[1:]
      #   print("instruction pointer, module_index:", instruction_pointer, module_index)
      elif line.startswith("+ "):
        allocation_index = line.split(" ")[1]
        print("allocating allocation index:", allocation_index)
      elif line.startswith("- "):
        allocation_index = line.split(" ")[1]
        print("freeing allocation index:", allocation_index)
      elif line.startswith("a "):
        size, trace_index = line.split(" ")[1:]
        print("size, trace index:", size, trace_index)
      elif line.startswith("c "):
        elapsed_milliseconds = line.split(" ")[1]
        print("elapsed milliseconds:", elapsed_milliseconds)
      elif line.startswith("R "):
        rss = line.split(" ")[1]
        print("rss: ", rss)
      elif line.startswith("X "):
        command_line = line.split(" ")[1:]
        print("command_line:", command_line)
      elif line.startswith("A "):
        # "from attached"
        pass
      elif line.startswith("v "):
        heaptrack_version, file_format_version = line.split(" ")[1:]
        print("heaptrack version, file format version:", heaptrack_version, file_format_version)
      elif line.startswith("x "):
        print("ignoring executable 'x'")
      elif line.startswith("I "):
        # _SC_PAGESIZE, _SC_PHYS_PAGES
        page_size, physical_pages = line.split(" ")[1:]
        print("page size, physical pages:", page_size, physical_pages)

if __name__ == "__main__":
  if len(argv) != 2:
    print("usage: {} </path/to/heaptrack-file.gz>".format(argv[0]))
    exit(1)

  file_path = argv[1]
  if not os.path.isfile(file_path):
    print("no such file: '{}'".format(file_path))
    exit(1)

  if not is_gunzipped(file_path):
    print("the file is not gunzipped: '{}'".format(file_path))
    exit(1)

  main(file_path)
