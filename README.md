# aasm-x86-assembler
This assembler was created as a project for understanding the working of the Assemblers during Summer of 2016.  
It provides a subset of the functionalities given in full-fledged assemblers.  
It works in 16-bit real mode adressing and produces a .com file under Windows.  

### Conventions:
imm8/16 - 8 or 16 bit immediate data  
reg8/16 - 8 or 16 bit registers  
mem - Memory location that stores data of appropriate size  
label - label used for jumping   

### Syntax:
- The general format of instructions is [LABEL:] MNEMONIC OPERAND [, OPERAND]   
- All mnemonics or registers have to be written in lower case.   
- All labels have to end with a colon.   
Ex - Label: mov ax, bx
- All immediate data should be either single ASCII character or hexadecimal number appened by h  
Ex - mov al, 'A'
     mov, 45h  

### Allowed insdtructions:
MOV reg8/16, reg8/16  
MOV reg8, imm8  
MOV reg16, imm16  
MOV reg8/16, mem  
  
LEA reg16, mem  
  
JMP label  