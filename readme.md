# potoroo

A preprocessor and kind of a build system for scripts and any other text files.

[VS Code integration](./vscode-integration.md)

## exapmles

See [processor test potorooJobs](./test/system/processor/potorooJobs) and [processor test src](./test/system/processor) or the [jdhp](https://github.com/oblaser/json-defined-html-page) project.

## cli arguments

```
potoroo [-jf FILE] [--force-jf]
potoroo -if FILE (-od DIR | -of FILE) [options]
```

| arg | description |
|:---|:---|
| `-jf FILE` | Specify a jobfile |
| `--force-jf` | Force jobfile to be processed even if errors occured while parsing it |
| `-if FILE` | Input file |
| `-of FILE` | Output file |
| `-od DIR` | Output directory (same filename) |
| `-t TAG` | Specify the tag |
| `-Werror` | Handles warnings as errors (only in processor, the jobfile parser is unaffected by this option). Results in not writing the output file if any warning occured. |
| `-Wsup LIST` | Suppresses the reporting of the specified warnings. LIST is a comma separated (no spaces) list of integer warning IDs. (Only in processor, the jobfile parser is unaffected by this option. May be useful in combination with `-Werror`) |
| `--write-error-line TEXT` | Instead of deleting the output file on error, writes _TEXT_ to it |
| `--copy` | Copy, replaces the existing file only if it is older than the input file |
| `--copy-ow` | Copy, overwrites the existing file |

The default jobfile `./potorooJobs` is processed, if no FILE argument is passed.


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
| `bash` | `##p` |
| `batch` | `@rem #p` |
| `custom:CTAG` | `CTAG` |


## preprocessor keywords
### include
Replaces the instruction line with the content of the specified file. The path is either absolute or relative to the current file.
```
//#p include "js/code.js"
//#p include 'js/moreCode.js'
```
If included with single quotes, the file is not preprocessed.

### ins
Removes the instruction, wich results in adding a single line of code.
```
//#p ins c = Math.sqrt(a*a + b*b);
```

### rm
The instruction lines and all lines between them are deleted.
```
//#p rm
code to be
deleted
//#p endrm
```

### rmn
Deletes the instruction line and the n following lines. (n = 1..9)
```
//#p rmn n
```
