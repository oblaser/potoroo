# potoroo

A preprocessor for scripts or any other text file.

## exapmles

See [processor test potorooJobs](./test/system/processor/potorooJobs) and [processor test src](./test/system/processor)

## cli arguments

```
potoroo [-jf FILE] [--force-jf]
potoroo -if FILE (-od DIR | -of FILE) [-t TAG] [options]
```

| arg | description |
|:---|:---|
| `-jf FILE` | Specify a jobfile |
| `--force-jf` | Force jobfile to be processed even if errors occured while parsing it |
| `-if FILE` | Input file |
| `-of FILE` | Output file |
| `-od DIR` | Output directory (same filename) |
| `-t TAG` | Specify the tag |

| option | description |
|:---|:---|
| `-Werror` | Handles warnings as errors (only in processor, the jobfile parser is unaffected by this option). Results in not writing the output file if any warning occured. |
| `--copy` | Copy, replaces the existing file only if it is older than the input file |
| `--copy-ow` | Copy, overwrites the existing file |

If no parameter or only `--force-jf` is provided the default jobfile `./potorooJobs` is processed.


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


## preprocessor key words
#### rm
```
//#p rm
deletes lines in between
these tags and
the tags themselves
//#p endrm
```

#### rmn
```
//#p rmn n
```
Deletes n lines after this tag and the tag itself. n = 1..9

#### ins
```
//#p ins c = Math.sqrt(a*a + b*b);
```
Removes the tag wich results in adding a single line of code.


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
