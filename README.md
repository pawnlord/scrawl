# scrawl
**S**crawl **C**an **R**arely **A**utomate **W**ith **L**ittle-effort  
A portable, python-esque scripting language made in pure C.

# building
Building should be universal on all platforms.  
`gcc src/*.c -o scrawl`  
  
# usage
There is no feeding in a file yet. That should be comming eventually.  
Just run the command and a prompt will appear.

# TODO
- [x] Operations: Addition, Subtraction, Multiplication  
  - [ ] Todo: Division, Order of Operations  
 
- [x] Comparison: Less than, Greater than, Equal-to  
  - [ ] Todo: Equal-to versions  
 
- [x] Blocks: if, while  
  - [ ] Todo: for, foreach (after data structures)  
 
- [x] Built-in Functions: print, system  
  - [ ] Todo: Input  

- [ ] User-Defined Functions  

- [x] Simple Data Structure: Array or List  
  - [x] Array
  - [ ] List
  - [ ] TODO: Better Print
  - [ ] TODO: Add actual identifier

- [ ] Prove usability with Turing machine and other examples  

- [ ] General Rewrite  
  - [ ] Move functions out of parse.c  
    - [ ] General functions (allocate_strptr, free_strptr, clear_strptr)  
    - [ ] Checking functions (isbool, isnum...)  
    - [ ] Variable functions (initvar, getvar, copyvar)  
  - [ ] Remove repetition wherever applicable and practical  