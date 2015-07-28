// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input. 
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel. When no key is pressed, the
// program clears the screen, i.e. writes "white" in every pixel.

(START)
	// i = screen_size [word] = 32*256 = 8192
	@8192
	D=A
	@i
	M=D	

	// ptr = first word of screen
	@SCREEN
	D=A
	@ptr
	M=D

	// Read keyboard status and select color to fill the screen.
	@KBD 		
	D=M 		// D = key code
	@WHITE 		// D == 0 => No key was pressed => white.
	D;JEQ

	(BLACK)		// key pressed => black
	@color
	M=-1
	@WRITE
	0;JMP

	(WHITE)
	@color
	M=0


(WRITE)
		@i 			// Have we reached the end of the screen?
		D=M
		@START
		D; JEQ 		// i == 0 => The whole screen was covered; start again.

		@color
		D=M 		// D = color.
		@ptr
		A=M 		// A = pointer to screen
		M=D 		// Current screen word = color
		@ptr
		M=M+1 		// ptr++
		@i
		M=M-1		// i--
		@WRITE
		0;JMP