# Zint

	Language Interpretor writen by me.


## Why
	- I just wanted to see how i can take it

## Compile
	
	```
	gcc zint.c -o zint
	```
## Zint
### There is two different syntax for Zint
	1. Normal
	```
		fn getsquare(x):
			x = x * x
			return x
		fnend

		fn main(void):
			a = 1
			b = call getsquare a
			print Square of #a is #b
		fnend
	```
	2. Not-so-normal
	```
		|@0 0;

			!0 2;

			#0 = 1;
			#1 = !6 1;

			!2 $Square of $#0$ is $#1$;

			!1 2;

		@@|

		|@1 1;
			
			#0 = o3 #0 #0

		@@ #1|
	```

	This is on purpose as No. 1 is converted to No. 2 (They do the same thing).
###### Reason?

	The Zint interpretor executes the code shown in No. 2, and does not check for errors unless explicitly told to do so.

	This makes the code execution a little tad faster by seperating the code execution and error catching process. A quality more akin to a compiler rather than a interpretor.


## Run Script
	
	```
	./zint [FILENAME] 
	```