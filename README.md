Linux Shell
Authored by Adiel Rozenfeld
208349019

==Description==

In this program we demonstrating a linux shell, printing each time the current path, and waiting for user input.
Each time the user insert a command, we'll try executing it.
using a txt file for history of commands the user inserted.
when the user insert "done", printing the number of commands was succesfully used and the number of words in all commands.
In addition, also supporting pipes.

Functions:
wordsCounter:
Function receive a string and goes through it, every time it encounter char (not space, '\0' and '\n') it check if the char before was space,
if it was so add one to wordsCount int
return wordsCount.

freeArgv:
Function receive a two dimensonial String array.
It goes through every String in the array and free it.
Finaly free the array itself.

executeCommand:
Function receive the user input (String), create a new **argv (two dimensional String array) using the createCommandStr function
create a new process, the child process (if was made succesfully) execute the command (using execvp()), if unable to execute command using perror to print details
use freeArgv function to free the **argv and child process exit.
the father process wait for the child to finish, if it was finished succesfully, adding the command to history file, adding the words count to totalWords int
and free the **argv.

historyCommand:
Function receive a String, trying to open the History.txt file with "r" mode, if unable to open the file so print "Cannot open file" and continue with the main loop.
Then going through the string received, if at least one char is not a number (not including '!' in first spot) return "continue" and the main loop continue.
If it was a valid history command (first spot is '!' and rest are numbers), it check if the number it not greater then the file lines count, if it is, return continue.
If the number is valid, goes through the file in order to get the line from the file, printing the command, if it was "history" command, printing history and reuturn continue.
Else, it return the valid command and it will be executed later (calling executeCommand from main loop).

printHistory:
Function try to open the "History.txt" file, if unsuccesfully, print "Cannot open file" and continue main loop.
Create a new empty string, new int lineNum = 1, and goes through the file, print every line in this format: "lineNum: line".
Finally close file.

addToHistory:
Function receive a string.
Function will try to open "History.txt" file with "a" mode, if unsuccesfully, print "Cannot open file" and exit program.
If file was open succesfully, writing to file in a new line the string received.

createCommandStr:
Function receive a String.
Create a new string with the size of MAX_LIMIT (512), creating a wordsCount int using the wordsCounter() function.
Then using malloc to create **argv two dimensional String array with the size of wordsCount + 1.
Going in a for loop through the String received and check if the current char is space or '\n', if it is, using malloc for the current word in the array.
finaly, put NULL in the last spot of the array and return the argv two dimensional array.

checkPipe:
Function check if there's a need for pipe (single duo or none) and return 0, 1, 2 accordingly.
If there is more then double pipe required (pipeCount > 2) unable to execute and will be handle in main.

executePipe:
 Function receive the user command as a string (str) and how many pipes are needed (pipeCount).
 First adding the amount of pipes needed to the amount were used.
 Creating 3 String, each one of them represent the commands (using third only if needed).
 Using for loop inserting the commands into the string were made.
 Using createCommandStr function, convert each String into argv[] array.
 Father create processes (2 or 3, depends on how many pipes were needed).
 Creating 2 pipes.
 Single pipe: Connecting first and second son to the first pipe, son #1 output and son #2 input.
 Each son is executing their command, finally the father wait() for them to finish and return.
 If one of the commands did not work, using perror() and free the argv[] arrays.
 Double pipe:Connecting first and second son to the first pipe just as before.
 Connecting son #2 and son #3 to the second pipe, son #2 output and son #2 input.
 Each son is executing their command, finally the father wait() for them to finish and return.
 If one of the commands did not work, using perror() and free the argv[] arrays.

==Program files==

ex3 - contain the main loop and the functions.

==How to complie==

first part:
compile: gcc ex3.c -o ex3
run: ./ex3

==Input:==
receiving repeatedly Strings from the user.

==Output:==
History.txt file - contains the History of strings the user inserted.
Words and chars count printed to the console.
perror printed if command wasn't succesfully executed.
"Unavailable yet" for the "cd" command.
