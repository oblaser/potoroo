# potoroo

A script preprocessor, which enables you to use kind of compile switches in scrips.

> In development, see 'dev' branch for latest code.
>
> Only `LF` line endings are supported at the moment.

## exapmles

See [processor test potorooJobs](./test/system/processor/potorooJobs) and [processor test src](./test/system/processor)

## cli arguments

```
potoroo [-jf FILE] [--force-jf]
potoroo -if FILE (-od DIR | -of FILE) [-t TAG] [-Werror]
```

| arg | description |
|:---|:---|
| `-jf FILE` | Specify a jobfile |
| `--force-jf` | Force jobfile to be processed even if errors occured while parsing it |
| `-if FILE` | Input file |
| `-of FILE` | Output file |
| `-od DIR` | Output directory (same filename) |
| `-t TAG` | Specify the tag |
| `-Werror` | Handles warnings as errors (only in processor, the jobfile parser is unaffected by this flag). Results in not writing the output file if any warning occured. |
| `-h`, `--help` | Print help |
| `-v`, `--version` | Print version |

If no parameter or only `--force-jf` is provided the default jobfile "./potorooJobs" is processed.


## jobfile

Each line is interpreted as a job. Paths in the jobfile are relative to its containing directory.

```
# comments are possible
-if ./index.js -od ./deploy/
-if ./code.abc -od ./deploy/ -tag cpp
```


## tags (-t TAG)

| TAG | tag string in file |
|:---|:---|
| `cpp` | `//#p` |
| `bash` | `#//p` |
| `batch` | `@rem #p` |
| `custom:CTAG` | `CTAG` |


## preprocessor directives

Sorted by priority (1. highest).

#### 1. rm
```
//#p rm
deletes lines in between
these tags and
the tags themselves
//#p endrm
```

#### 2. rmn
```
//#p rmn n
```
Deletes n lines after this tag and the tag itself. n = 1..9

#### 3. ins
```
//#p ins c = Math.sqrt(a*a + b*b);
```
Removes the tag wich results in adding a single line of code.

## warnings and errors
```
(fileName | process):[line:[column:]] (warning | error): text
```

---

## License

GNU GPL v3

Copyright (C) 2021  Oliver Blaser

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
