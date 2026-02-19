#!/bin/bash
# 链接器包装脚本，用于过滤掉AGL框架引用

# 获取原始链接命令
original_command="$@"

# 过滤掉-framework AGL参数
filtered_command=$(echo "$original_command" | sed 's/-framework AGL//g')

# 执行过滤后的命令
eval "$filtered_command"