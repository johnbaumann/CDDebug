
![](social_card_PNG.png)


# Hello world! + Flappy Credits
	
	Example bare-metal code to get the pads + gpu going without using Sony's PS1 libraries.
	Edit: Now with sea creatures and a .TIM parser/uploader!

	There's also a TTY device example to log printfs over Sio.

	First:
	Big thanks to Nicolas Noble over at https://github.com/grumpycoders/pcsx-redux/ 
	for the "nugget" toolchain/build environment!
    
    
# Installation

	Brief:

		Install docker
		Whitelist this folder *
		Run the .bat

		If you don't have this option, you're using an up to date version of Win10 and can skip step 2!
		* Settings (icon) -> Resources -> File Sharing -> Little blue (+) icon -> add the folder

	Instructions:

		Install this: https://www.docker.com/

		Goto Settings | Resources | File Sharing, and add this folder

		Run buildme.bat to build.

		Docker will do a one-time download of the build image (see intro) and then do the thing.
		


# Questions


	Can I use some pre-made libraries?
		For PSYQ:
			they must be converted with this tool:	
		    https://github.com/grumpycoders/pcsx-redux/tree/main/tools/psyq-obj-parser
		For psn00b SDK:
		  	I'm not sure. Let me know how it goes!



