# Compiler

This project is a compiler that converts a simple C-style language to x86_64 assembly.

## Usage

Building this project is as simple as cloning the repository and running the **make** command. This will compile the source code and output an executable called **./lang**.

Once the executable **./lang** is created, you can compile a source file to assembly code by redirecting that file as input to **./lang**. Suppose you had a source file called *code.txt*. You compile it with the following command

*./lang < code.txt*

This will generate the assembly file *a.asm*. Once the assembly file is generated, you can convert that to an executable by running the **asm64** bash script. In this example, syntax for using **asm64** is as follows.

*./asm64 a*

## Syntax

Syntax for this language is very similar to C, although it is far less complex.

### main

Every source file must have a **main** function. The **main** function must take exactly two integer parameters. They can be named whatever you like. There can only be one return statement in main, and it must be the last statement in **main**. The return value must always be an integer, but it does not need to be a constant.

```c++
int main(int a, int b){
  return a + b;
}
```

### Variables

This language only allows for local variables of type *int* or *int\**. Furthermore, they must always be initialized to a constant when defined. For pointers, this must be *nullptr*. After they are initially declared, the can be reassigned to other values like how you would in C.

```c++
int main(int a, int b){
  int integer = 10;
  int* pointer = nullptr;
  integer = a + b;
  pointer = &a;
  return *pointer;
}
```

You may also dynamically allocate arrays of integers.

```c++
int main(int a, int b){
  int* array = nullptr;
  array = new int [2];
  array[0] = a;
  array[1] = b;
  
  a = array[1];
  
  delete [] array;
  return a;
}
```

Currently, this language does not support things like `a = -1`. Instead, you would have to do `a = 0 - 1`.

### Arithmetic

This language supports addition, subtraction, multiplication, division and modulo.

```c++
int main(int a, int b){
  a = a + b;
  int c = 2 * (10 / 2) % 5;
  return 0;
}
```

### Control Flow

This language supports while loops and if else clauses. There is no support for else if at the moment. The available comparisons are **==**, **!=**, **<** and **>**.

```c++
int main(int a, int b){
  int c = 10;
  
  if (a > 10){
    c = 20;
  }
  
  while (c > 0){
    b = b + c;
    c = c - 1;
  }
  
  return b;
}
```

### Output

The *express* function can be used to print expressions to standard output.

```c++
int main(int a, int b){
  int c = 10;
  express(c);
  return 0;
}
```

The output would be `10`.

### Functions

You can define other functions besides **main**. These functions must be defined in the source file before main is defined. They can have whatever parameters you like, as long as they are of type *int* or *int\**. They must return an integer in the same way main returns an integer. When calling a function, you must assign the returned value to a variable. The following is an example of calculating Fibonacci numbers.

```c++
int fib(int x){
  int result = x;
  
  if (x > 1){
    result = fib(x - 1) + fib(x - 2);
  }
  
  return result;
}

int main(int a, int b){
  a = fib(6);
  express(a);
  return 0;
}

```

The output would be `8`. Since the return statement must be the last statement to appear in the function, this language does not allow for tail recursion.

## Running the program

The two parameters of the main function store two integers that are passed in from the command line. An example of how to run an executable is as follows.

*./a 1 2*

Incorrectly passing in arguments will likely result in a segmentation fault.

