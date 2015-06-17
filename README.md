#MySP Scripting Language

##What is MySP?
MySP is a scripting language created by [Jason Mace](https://www.linkedin.com/pub/jason-mace/88/673/a10).
This project is being created as part of a senior project at Brigham Young University-Idaho.

##How does it work?
MySP takes infix code and tokenizes it. The tokens are checked for syntax errors and then turned into binary expression trees that can evaluate. The trees are then turned into psuedo-assembly instruction code that is executed.
Take the following example:
```
a = 1 + 2;

[word:a] [operator:=] [number:1] [operator:+] [number:2]

   =
 /   \
a     +
    /   \
   1     2

ADD     1, 2
ASSIGN  a, <reg>
```