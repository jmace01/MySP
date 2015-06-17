#MySP Scripting Language

##What is MySP?
MySP is a scripting language created by [Jason Mace](https://www.linkedin.com/pub/jason-mace/88/673/a10).
This project is being created as part of a senior project at Brigham Young University-Idaho.

##How does it work?
###High level overview
MySP takes infix code and tokenizes it. The tokens are checked for syntax errors and then turned into binary expression trees that can evaluate. The trees are then turned into psuedo-assembly instruction code that is executed.
###Simplified Example
Take the following example input:
```
a = 1 + 2;
```
The following tokens are created for this statement:
```
[word:a] [operator:=] [number:1] [operator:+] [number:2]
```
Note that this is not a complete view of the tokens, just a simplistic view of them. From this, we can see that the pattern is an opperand token followed by an operator token. The first and last tokens are notibly operands. Thus, we can identify this statement as valid. The process becomes more complex with unary operations and other specific cases, but this is a basis for our examination of the given input.
From these tokens, the following binary expression tree is produced:
```
      =
    /   \
   +     a
 /   \
2     1
```
The leaf nodes are the operands and all other nodes are operators.
Traversing the tree in a depth-first manner, starting left, produces the instruction code.
```
ADD     1, 2
ASSIGN  a, <reg>
```