# 1. Makefile

Makefile 是一个程序构建工具，可在 Unix、Linux 及其版本上运行。它有助于简化可能需要各种模块的构建程序可执行文件。要确定模块需要如何一起编译或重新编译，**make**借助用户定义的 makefile。



**make**程序允许使用类似于变量的宏。宏在 Makefile 中定义为 = 对。下面显示了一个示例 -

```makefile
MACROS  = -me
PSROFF  = groff -Tps
DITROFF = groff -Tdvi
CFLAGS  = -O -systype bsd43
LIBS    = "-lncurses -lm -lsdl"
MYFACE  = ":*)"
```





# Makefile - 依赖

## 简述

最终的二进制文件依赖于各种源代码和源头文件是很常见的。依赖关系很重要，因为它们让**make**知道任何目标的来源。考虑以下示例 -

```makefile
hello: main.o factorial.o hello.o
   $(CC) main.o factorial.o hello.o -o hello
```

在这里，我们告诉**make** hello 依赖于 main.o、factorial.o 和 hello.o 文件。因此，每当这些目标文件中的任何一个发生变化时，**make**将采取行动。

同时，我们需要告诉**make**如何准备 .o 文件。因此，我们还需要如下定义这些依赖项 -

```makefile
main.o: main.cpp functions.h
   $(CC) -c main.cpp
factorial.o: factorial.cpp functions.h
   $(CC) -c factorial.cpp
hello.o: hello.cpp functions.h
   $(CC) -c hello.cpp
```
