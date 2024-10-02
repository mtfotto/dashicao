# <center style="color: yellow;">大市操</center>

## 简介

大市操是一款由钱虎CPP开发的高性能并发式多进程任务执行框架。软件支持通过使用YAML配置文件对参数进行配置。用户只需要填写一个YAML配置文件并在运行时指定即可将任务从单进程变为多进程并发执行。

本软件仅支持Linux操作系统。

## 构建程序

1. 使用git下载代码并进入项目路径
```
git clone 仓库地址
cd dashicao
```

2. 本项目构建依赖[CMake](http://www.cmake.org)，如果未安装[CMake](http://www.cmake.org)请先安装[CMake](http://www.cmake.org)。
```bash
cmake .
make
```

3. 构建成功，在控制台中输入`./dashicao`验证安装是否成功。
```bash
./dashicao
```

## 编写配置文件

配置文件是软件的灵魂。只有指定了配置文件内的参数软件才可以正式工作。
使用`./dashicao -i`或者`./dashicao --init`初始化配置文件到`config.yaml`中。
```bash
./dashicao -i # 也可以是./dashicao --init
```
为了演示配置文件应该如何填写，配置文件中默认对[大市唱](https://dashichang.work/)网站进行文件下载操作，仅供用户参考。

### 配置文件说明

文件中必须要有的参数包括：
```text
vars              # 代表接下来的配置中要使用的宏
logOutputConsole  # 是否将日志输出到控制台
logger_dir        # 日志存到哪个文件里面
logger_level      # 日志级别
redirect          # 程序重定向到哪去
random_seed       # 随机种子
loop              # 每个进程是否循环执行
process_num_base  # 所有任务的分配进程数量基数
cmds              # 要运行的指令
```
其中配置文件具体如何填写以及每个值具体的用法均已写在默认的`config.yaml`文件当中，故在此不再赘述。

## 启动程序
注意将config.yaml换成你自己的配置文件并且确认已经配置完毕
```bash
./dashicao -c config.yaml
# 或者
./dashicao -cconfig.yaml
# 或者
./dashicao --config config.yaml
# 或者
./dashicao --configconfig.yaml
```