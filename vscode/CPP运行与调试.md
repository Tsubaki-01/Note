```c++
{
  "tasks": [
    {
      "type": "cppbuild",
      "label": "C/C++: g++-13 生成活动文件",
      "command": "/usr/bin/g++-13",
      "args": [
        "-fdiagnostics-color=always",
        "-g",
        "${file}",
        "-o",
        "${fileDirname}/${fileBasenameNoExtension}",
        "-L",
        "/usr/lib/x86_64-linux-gnu",
        "-lavformat"
      ],
      "options": {
        "cwd": "${fileDirname}"
      },
      "problemMatcher": ["$gcc"],
      "group": {
        "kind": "build",
        "isDefault": true
      },

      "detail": "调试器生成的任务。"
    }
  ],
  "version": "2.0.0",
  "configurations": [
    {
      "name": "(gdb) Launch",
      "type": "cppdbg",
      "request": "launch",
      "program": "${workspaceFolder}/${fileBasenameNoExtension}",
      "args": [],
      "stopAtEntry": false,
      "cwd": "${workspaceFolder}",
      "environment": [],
      "externalConsole": false,
      "MIMode": "gdb",
      "setupCommands": [
        {
          "description": "Enable pretty-printing for gdb",
          "text": "-enable-pretty-printing",
          "ignoreFailures": true
        }
      ],
      "miDebuggerPath": "/usr/bin/gdb",
      "preLaunchTask": "C/C++: g++-13 生成活动文件",
      "logging": {
        "engineLogging": true
      },
      "visualizerFile": "${workspaceFolder}/.vscode/gdb_printers.py"
    }
  ]
}

```

