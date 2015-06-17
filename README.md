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
##Sample Syntax
Here is a sample (albeit contrived) of the syntax.
```C
/**
 * GENERIC SORTING CLASS
 */
class Sorter {
    
    //Store the array
    protected dynamic vArray;
    
    //Store the size of the array
    protected dynamic vSize;
    
    //Return the array
    public dynamic getArray() {
        return this->vArray;
    }

}


/**
 * BUBBLE SORT CLASS
 */
class BubbleSort inherits Sorter {
    
    //Constructor
    public dynamic BubbleSort(iArray, size) {
        this->vArray = iArray;
        this->vSize  = size;
    }
    
    //Sorting algorithm
    public dynamic sortArray() {
        swapped = true;
        for (i = this->vSize - 1; i >= 0 && swapped; i--) {
            swapped = false;
            for (j = this->vSize - 1; j >= 0; j--) {
                if (this->vArray[j - 1] > this->vArray[j]) {
                    temp = this->vArray[j];
                    this->vArray[j] = this->vArray[j - 1];
                    this->vArray[j - 1] = temp;
                    swapped = true;
                }
            }
        }
    }
    
}

/**
 * Entry point of program
 */
main {

    //Should we sort the array?
    sortArray = true;

    //Initialize array
    a = array();
    a[0]  = 5;              //5
    a[1]  = 1;              //1
    a[2]  = 6  + 7;         //13
    a[3]  = 31 - 10;        //21
    a[4]  = 3 * 5 * 2 + 4;  //34
    a[5]  = 12 / 4;         //3
    a[6]  = 2;              //2
    a[7]  = (1 == 1) + 7;   //8
    a[8]  = 1;              //1

    //Instantiate BubbleSort class object
    sort = BubbleSort(a, 9);
    
    //If sortArray is true, we sort the array
    !sortArray || sort->sortArray();
    
    //Get the array
    b = sort->getArray();
    
    x = 0;
    while (x < 9) {
        //Output element
        print (x != 0) ? ' ' : '';
        print b[x];
        x++;
    }
    print "\n";
}
```