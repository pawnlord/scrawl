# scripts
**S**imple, **C**rappy, **R**esource **I**ntensive, **P**oorly-made **T**rain-wreck of a **S**cript  
A portable, python-esque scripting language made in pure C.

# building
Building should be universal on all platforms.  
`gcc src/*.c -o scripts`  

# usage
There is no feeding in a file yet. That should be comming eventually.  
Just run the command and a prompt will appear.

## operations  
Currently, there aren't any functions. But I do have variables and operators (`+`, `-`, `*`, `=`) working.  
Division hasn't been added yet since that would require floats  
  
example:  
```
# comment
i = 1 # make new variable i, set variable i to 1
a = i + 1 # make new variable a, set variable a to 1 more than i (2)
i += 2 # set i to 2 more than i (3)
a = i - 1 # set a to 1 less than i (2)
i -= 2 # set i to 2 less than i (1)
i = -1 # set i to a negative (-1)
a = i * 2 # set a to 2 times i (-2)
i *= 3 # set i to 2 times i (-3)
```