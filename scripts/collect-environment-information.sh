#!/usr/bin/env bash

database_path="$1"
environment_name="$2"

if [[ -z "$database_path" ]] || [[ -z "$environment_name" ]]; then
  echo "usage: $0 <database path> <environment name>"
  exit 1
fi

function assert_command() {
  name="$1"
  if ! which "$name" &> /dev/null; then
    echo "Expected command '$name' to be available"
    exit 1
  fi
}

function escape() {
  if [[ -n "$1" ]]; then
    echo "$1" | sed -s "s/'/''/g"
  else
    sed -s "s/'/''/g"
  fi
}

function run() {
  statement="$1"
  shift
  while echo "$statement" | grep -q '?'; do
    # Escape single quotes for sqlite
    value="${1//"'"/"'''"}"
    # Escape forward slashes for sed
    value="${1//"/"/"\\/"}"
    # Escape newlines
    value="${1//"\n"/"\\n"}"
    # Trim whitespace
    value="$(echo "$value" | xargs)"
    statement="$(echo "$statement" | sed "s/?/$value/")"
    shift
  done

  sqlite3 "$database_path" "$statement"
}

assert_command "sqlite3"

# If the database already exists, remove the row for the target environment.
# If the database does not exist, create it.
if [[ -f "$database_path" ]]; then
  run "delete from environments where name='?'" "$environment_name"
else
  run "create table environments (name TEXT PRIMARY KEY, uname TEXT, cores INT, threads INT, memory INT, memory_speed TEXT, cpu TEXT, cpu_features TEXT);"
fi

# Create a row for the environment
run "insert into environments (name) values ('?')" "$environment_name"

# Insert uname
run "update environments set uname='?' where name='?'" "$(uname)" "$environment_name"

# Insert lshw / sysctl information
if [[ "$(uname)" = "Darwin" ]]; then
  run "update environments set cores='?' where name='?'" "$(sysctl machdep.cpu.core_count | cut -d ':' -f2)" "$environment_name"
  run "update environments set threads='?' where name='?'" "$(sysctl machdep.cpu.thread_count | cut -d ':' -f2)" "$environment_name"
  run "update environments set memory='?' where name='?'" "$(sysctl hw.memsize | cut -d ':' -f2)" "$environment_name"
  run "update environments set cpu='?' where name='?'" "$(sysctl machdep.cpu.brand_string | cut -d ':' -f2)" "$environment_name"
  run "update environments set cpu_features='?' where name='?'" "$(sysctl machdep.cpu.features | cut -d ':' -f2)" "$environment_name"
  run "update environments set memory_speed='?' where name='?'" "$(system_profiler SPMemoryDataType | grep -i speed | head -1 | cut -d':' -f2)" "$environment_name"
else
  run "update environments set cores='?' where name='?'" "$(lscpu | grep '^CPU(s):' | cut -d':' -f2)" "$environment_name"
  run "update environments set threads='?' where name='?'" "$(($(lscpu | grep '^CPU(s):' | cut -d':' -f2) * $(lscpu | grep '^Thread(s)' | cut -d':' -f2)))" "$environment_name"
  run "update environments set memory='?' where name='?'" "$(free -b | awk '{print $2}' | head -n2 | tail -1)" "$environment_name"
  run "update environments set cpu='?' where name='?'" "$(lscpu | grep '^Model' | cut -d ':' -f2)" "$environment_name"
  run "update environments set cpu_features='?' where name='?'" "$(lscpu | grep '^Flags' | cut -d ':' -f2)" "$environment_name"
fi

# List the contents
run "select * from environments"
