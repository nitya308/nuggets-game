## Code style and quality

Grade: 13/15

Summary:  The following feedback is meant to highlight some of my primary observations; Look for comments with keyword "STYLE" or "GRADER" in your Makefile and source-code files.

---

```
    client.c: If you must use a maximum size, use a `const` value
 -1 client.c: Should use `const char*` for protocol keywords
 -1 client.c: Unchecked return value from `message_loop`, `printw`,
                `refresh`, `initscr`, `cbreak`, `noecho`
  
```

--- 

Recall the style rubric which referred to the following points:

* code should be well-organized with sensible filenames and subdirectories
* brief `README.md` in each directory
* clear Makefile for clean and build
* clear and consistent style
* clear code design, functional decomposition, naming
* clear code logic and flow
* good choice of, and use of, data structures
* good modularity (strong cohesion, loose coupling)
* good in-code documentation (comments)
* code is consistent with documentation (IMPLEMENTATION.md)
* defensive programming (error checking, malloc, etc.)
* no compiler warnings
* no memory leaks/errors caused by student code
* no output to stdout other than what is required to play the game

