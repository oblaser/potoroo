# VS Code integration

An example of the `{prjDir}\.vscode\tasks.json` file:
```
{
    // See https://go.microsoft.com/fwlink/?LinkId=733558
    // for the documentation about the tasks.json format
    "version": "2.0.0",
    "tasks":
    [
        {
            "label": "deploy src",
            "type": "shell",
            "command": "potoroo -jf ./deploy/potorooJobs",
            "problemMatcher": [],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false,
                "panel": "dedicated",
                "showReuseMessage": false,
                "clear": false
            }
        }
    ]
}
```

With this added you can use `ctrl+B` to process the job file.

If you don't have potoroo added to your `PATH` variable the `"command"` would look like this: `"C:\path\to\potoroo.exe -jf ./deploy/potorooJobs"`.

[VS Code - Integrate with External Tools via Tasks](https://code.visualstudio.com/docs/editor/tasks)
