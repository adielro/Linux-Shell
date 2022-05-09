#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#define MAX_LIMIT 512
FILE *fp;
char **createCommandStr (char *str);
void addToHistory (char *str);
void printHistory ();
void executePipe (char *str, int pipeCount);
void executeCommand (char *str);
void freeArgv (char **argv);
void deleteQuotation (char **str);
void handler (int sig);
void normalizeHistoryCommand (char *str, int length);
void rewriteNohup (char **argv);
char *historyCommand (char *str);
int wordsCounter (char *str);
int checkPipe (char *str);
int shouldWait (char *str);
int totalWords;
int commandsCounter;
int pipeUse;


/*
 Function recvive argv (two-dimensional string array), delete the "nohup" from the array.
*/
void
rewriteNohup (char **argv)
{
    int i;
      for (i = 0; argv[i + 1] != NULL; i++)
	{
	  strcpy (argv[i], argv[i + 1]);
	}
      argv[i] = NULL;
}

/*
 Function receive a string and length, deleting all spaces in the end of the string, by so, making it valid as history command.
*/
void
normalizeHistoryCommand (char *str, int length)
{
  for (int i = length - 1; str[i] < '0' || str[i] > '9'; i--)
    {
      if (str[i] == ' ')
	str[i] = '\0';
    }
  str[strlen (str)] = '\n';
  strcpy (str, historyCommand (str));
}

/*
 Function receive a **argv command, if it has "echo", deleting all quotes in it.
*/
void
deleteQuotation (char **str)
{
  if (strcmp (str[0], "echo") == 0)
    {
      if (str[1][0] == '"')
	{
	  int k = 0;
	  int l = 0;
	  while (str[k] != NULL)
	    k++;
	  while (str[k - 1][l] != '\0')
	    l++;
	  if (str[k - 1][l - 1] == '"')
	    {
	      for (int n = 0; str[1][n] != '\0'; n++)
		str[1][n] = str[1][n + 1];
	      if (k - 1 == 1)
		str[1][l - 2] = '\0';
	      else
		str[k - 1][l - 1] = '\0';
	    }
	}
    }
}

/*
 Using waitpid() for all sons.
*/
void
handler (int sig)
{
  waitpid (-1, NULL, WNOHANG);
}


/*
 Function receive a string, return 1 if there is no '&', else return 0.
*/
int
shouldWait (char *str)
{
  char sign;
  for (int i = 0; str[i] != '\0'; i++)
    if (str[i] != ' ' && str[i] != '\0' && str[i] != '\n')
      sign = str[i];
  if (sign != '&')
    return 1;

  return 0;
}

/*
 **************************IN ADDITION TO EX2B**************************
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
*/
void
executePipe (char *str, int pipeCount)
{
  int printCmd = 0;
  pipeUse += pipeCount;
  char firstCmd[MAX_LIMIT];
  char secondCmd[MAX_LIMIT];
  char thirdCmd[MAX_LIMIT];
  int pipeDone = 0;
  int i = 0;
  while (pipeDone < pipeCount + 1)
    {
      if (pipeDone == 0)
	{
	  for (int j = 0; str[i] != '|'; i++)
	    {
	      firstCmd[j] = str[i];
	      firstCmd[++j] = ' ';
	      firstCmd[j + 1] = '\0';
	    }
	}
      else if (pipeDone == 1)
	{
	  while (str[i] == ' ')
	    i++;
	  for (int j = 0; str[i] != '|' && str[i] != '\0'; i++)
	    {
	      secondCmd[j] = str[i];
	      secondCmd[++j] = ' ';
	      secondCmd[j + 1] = '\0';
	    }
	}
      else
	{
	  while (str[i] == ' ')
	    i++;
	  for (int j = 0; str[i] != '\0'; i++)
	    {
	      thirdCmd[j] = str[i];
	      thirdCmd[++j] = '\0';
	    }
	  int j = i - 1;
	  while (thirdCmd[j] == ' ')
	    thirdCmd[j--] = '\0';
	}
      pipeDone++;
      i++;
    }
  if (firstCmd[0] == '!')
    {
      normalizeHistoryCommand (firstCmd, strlen (firstCmd));
      printCmd = 1;
    }
  if (secondCmd[0] == '!')
    {
      normalizeHistoryCommand (secondCmd, strlen (secondCmd));
      printCmd = 1;
    }
  if (thirdCmd[0] == '!')
    {
      normalizeHistoryCommand (thirdCmd, strlen (thirdCmd));
      printCmd = 1;
    }
  if (printCmd == 1)
    {
      char pipeCommandStr[512];
      strcpy (pipeCommandStr, firstCmd);
      pipeCommandStr[strlen (pipeCommandStr) - 1] = '\0';
      strcat (pipeCommandStr, " | ");
      strcat (pipeCommandStr, secondCmd);
      if (pipeCount == 2)
	{
	  pipeCommandStr[strlen (pipeCommandStr) - 1] = '\0';
	  strcat (pipeCommandStr, " | ");
	  strcat (pipeCommandStr, thirdCmd);
	}
      printf ("%s", pipeCommandStr);
       int pipeCount = checkPipe (pipeCommandStr);
	  if (pipeCount > 2){
	    printf ("Invalid command.\n");
	      return;
	  }
	  else{
	  executePipe (pipeCommandStr, pipeCount);
        return;
	  }
    }
  int fds[2];
  int fds2[2];
  pipe (fds);
  pipe (fds2);
  if (fork () != 0)
    {
      if (fork () != 0)
	{
	  if (pipeCount == 2)
	    {
	      if (fork () != 0)
		{		//double pipe father
		  close (fds[1]);
		  close (fds[0]);
		  close (fds2[0]);
		  close (fds2[1]);
		  if (shouldWait (str) == 1)
		    while (wait (NULL) > 0);
		  addToHistory (str);
		  //addToHistory (secondCmd);
		  //addToHistory (thirdCmd);
		  return;
		}		// son #3 if needed change input
	      char **cmd3 = createCommandStr (thirdCmd);
	      deleteQuotation (cmd3);
	      if (strcmp(cmd3[0], "nohup") == 0){
	        rewriteNohup(cmd3);
	        int fd = open ("nohup.txt", O_WRONLY | O_CREAT | O_APPEND,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	        dup2 (fd, STDOUT_FILENO);
	        signal (SIGHUP, SIG_IGN);
	      }
	      dup2 (fds2[0], STDIN_FILENO);
	      close (fds[1]);
	      close (fds[0]);
	      close (fds2[0]);
	      close (fds2[1]);
	      if (strcmp(cmd3[0], "history") == 0){
      printHistory();
          freeArgv(cmd3);
          exit(1);
      }
	      if (execvp (cmd3[0], cmd3) == -1)
		{
		  perror ("execvp");
		  freeArgv (cmd3);
		  exit (1);
		}
	    }
	  else
	    {			// single pipe father
	      close (fds[1]);
	      close (fds[0]);
	      close (fds2[0]);
	      close (fds2[1]);
	      if (shouldWait (str) == 1)
		while (wait (NULL) > 0);
	      i = strlen (secondCmd) - 1;
	      while (secondCmd[i] == ' ')
		secondCmd[i--] = '\0';
	      addToHistory (str);
	   //   addToHistory (secondCmd);
	      return;
	    }
	}			// son #2 check single/double
      char **cmd2 = createCommandStr (secondCmd);
      deleteQuotation (cmd2);
      if (pipeCount == 2)
	{			// double
	  dup2 (fds[0], STDIN_FILENO);
	  dup2 (fds2[1], STDOUT_FILENO);
	  close (fds[0]);
	  close (fds[1]);
	  close (fds2[0]);
	  close (fds2[1]);
	}
      else
	{
	    if (strcmp(cmd2[0], "nohup") == 0){
	        rewriteNohup(cmd2);
	        int fd = open ("nohup.txt", O_WRONLY | O_CREAT | O_APPEND,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	        dup2 (fd, STDOUT_FILENO);
	        signal (SIGHUP, SIG_IGN);
	      }
	  dup2 (fds[0], STDIN_FILENO);
	  close (fds[1]);
	  close (fds[0]);
	  close (fds2[0]);
	  close (fds2[1]);
	}
	if (strcmp(cmd2[0], "history") == 0){
      printHistory();
          freeArgv(cmd2);
          exit(1);
      }
      if (execvp (cmd2[0], cmd2) == -1)
	{
	  perror ("execvp");
	  freeArgv (cmd2);
	  exit (1);
	}
    }
  else
    {				// son #1 change output
      char **cmd1 = createCommandStr (firstCmd);
      deleteQuotation (cmd1);
      dup2 (fds[1], STDOUT_FILENO);
      close (fds[0]);
      close (fds[1]);
      close (fds2[0]);
      close (fds2[1]);
      if (strcmp(cmd1[0], "history") == 0){
      printHistory();
          freeArgv(cmd1);
          exit(1);
      }
      else if (execvp (cmd1[0], cmd1) == -1)
	{
	  perror ("execvp");
	  freeArgv (cmd1);
	  exit (1);
	}
    }
}


/*
Function check if there's a need for pipe (single duo or none) and return 0, 1, 2 accordingly.
If there is more then double pipe required (pipeCount > 2) unable to execute and will be handle in main.
*/
int
checkPipe (char *str)
{
  int pipeCount = 0;
  for (int i = 0; str[i] != '\0'; i++)
    {
      if (str[i] == '|')
	pipeCount++;
    }
  return pipeCount;
}

/*
Function receive a string and goes through it, every time it encounter char (not space, '\0' and '\n') it check if the char before was space,
if it was so add one to wordsCount int
return wordsCount.
*/
int
wordsCounter (char *str)
{
  int i;
  int wordsCount = 0;
  for (i = 0; str[i] != '\0'; i++)
    {
      if (str[i] != ' ' && str[i] != '\0' && str[i] != '\n')
	{
	  if (i > 0 && str[i - 1] == ' ')
	    wordsCount++;
	}
    }
  if (i == 1)
    wordsCount = 0;
  else if (i > 0 && str[0] != ' ')
    wordsCount++;
  return wordsCount;
}

/*
Function receive a two dimensonial String array.
It goes through every String in the array and free it.
Finaly free the array itself.
*/
void
freeArgv (char **argv)
{
  for (int i = 0; argv[i] != NULL; i++)
    {
      free (argv[i]);
      argv[i] = NULL;

    }
  free (argv);
  argv = NULL;
}

/*
Function receive the user input (String), create a new **argv (two-dimensional String array) using the createCommandStr function
create a new process, the child process (if was made successfully) execute the command (using execvp()), if unable to execute command using perror to print details
use freeArgv function to free the **argv and child process exit.
the father process wait for the child to finish, if it was finished successfully, adding the command to history file, adding the words count to totalWords int
and free the **argv.
*/
void
executeCommand (char *str)
{
  pid_t x;
  x = fork ();
  if (x == 0)
    {
      char **argv = createCommandStr (str);
      if (argv == NULL)
	{
	  printf ("Invalid command\n");
	}
      if (strcmp (argv[0], "nohup") == 0)
	{
	    rewriteNohup(argv);
	  int fd = open ("nohup.txt", O_WRONLY | O_CREAT | O_APPEND,
			 S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	  dup2 (fd, STDOUT_FILENO);
	  signal (SIGHUP, SIG_IGN);
	}
	 if (strcmp(argv[0], "history") == 0){
      printHistory();
          freeArgv(argv);
          exit(1);
      }
      if (execvp (argv[0], argv) == -1)
	{
	  perror ("execvp");
	  freeArgv (argv);
	  exit (1);
	}
    }
  if (shouldWait (str) == 1)
    while (wait (NULL) > 0);
  addToHistory (str);
  commandsCounter++;
}

/*
Function receive a String, trying to open the History.txt file with "r" mode, if unable to open the file so print "Cannot open file" and continue with the main loop.
Then going through the string received, if at least one char is not a number (not including '!' in first spot) return "continue" and the main loop continue.
If it was a valid history command (first spot is '!' and rest are numbers), it checks if the number it not greater than the file lines count, if it is, return continue.
If the number is valid, goes through the file in order to get the line from the file, printing the command, if it was "history" command, printing history and return continue.
Else, it returns the valid command, and it will be executed later (calling executeCommand from main loop).
*/
char *
historyCommand (char *str)
{
  fp = fopen ("History.txt", "r");
  if (fp == NULL)
    {
      printf ("Cannot open file\n");
      return "continue";
    }
  char numStr[512];
  for (int i = 1; str[i] != '\n';)
    {
      if (str[i] >= '0' && str[i] <= '9')
	{
	  numStr[i - 1] = str[i];
	  numStr[i++] = '\0';
	}
      else
	{
	  printf ("Invalid command number!\n");
	  fclose (fp);
	  return "continue";
	}
    }
  int lineNum = atoi (numStr);
  char line[MAX_LIMIT] = { 0 };
  int maxLine = 0;
  while (fgets (line, MAX_LIMIT, fp))
    maxLine++;
  rewind (fp);
  if (lineNum > maxLine || lineNum <= 0)
    {
      printf ("Invalid command number!\n");
      fclose (fp);
      return "continue";
    }
  for (int i = 0; i < lineNum; i++)
    {
      fgets (line, MAX_LIMIT, fp);
    }
  fclose (fp);
  if (strcmp (line, "history\n") == 0)
    {
      addToHistory (line);
      printHistory ();
      commandsCounter++;
      return "continue";
    }
  for (int i = 0; str[i] != '\0'; i++)
    if (str[i] == '\n')
      {
	str[i] = '\0';
	break;
      }
  strcpy (str, line);
  return str;
}

/*
Function try to open the "History.txt" file, if unsuccessfully, print "Cannot open file" and continue main loop.
Create a new empty string, new int lineNum = 1, and goes through the file, print every line in this format: "lineNum: line".
Finally close file.
*/
void
printHistory ()
{
  fp = fopen ("History.txt", "r");
  if (fp == NULL)
    {
      printf ("Cannot open file");
      return;
    }
  char line[MAX_LIMIT] = { 0 };
  int lineNum = 1;
  while (fgets (line, MAX_LIMIT, fp))
    {
      printf ("%d: %s", lineNum++, line);
    }
  fclose (fp);
}

/*
Function receive a string.
Function will try to open "History.txt" file with "a" mode, if unsuccessfully, print "Cannot open file" and exit program.
If file was open successfully, writing to file in a new line the string received.
*/
void
addToHistory (char *str)
{
  int i = 0;
  while (str[i] != '\0')
    i++;
  int j = i - 1;
  while (str[i - 1] == ' ')
    str[i--] = '\0';
  if (str[i - 1] != '\n')
    {
      str[i++] = '\n';
      str[i] = '\0';
    }
  fp = fopen ("History.txt", "a");
  if (fp == NULL)
    {
      printf ("Cannot open file");
      exit (1);
    }
  fprintf (fp, "%s", str);
  fclose (fp);
}

/*
Function receive a String.
Create a new string with the size of MAX_LIMIT (512), creating a wordsCount int using the wordsCounter() function.
Then using malloc to create **argv two-dimensional String array with the size of wordsCount + 1.
Going in a for loop through the String received and check if the current char is space or '\n', if it is, using malloc for the current word in the array.
finally, put NULL in the last spot of the array and return the argv two-dimensional array.
*/
char **
createCommandStr (char *str)
{
  int i;
  char word[MAX_LIMIT];
  int wordsCount = wordsCounter (str);
  char **argv = (char **) malloc ((wordsCount + 1) * sizeof (char *));
  int j = 0;
  int dest = 0;
  for (i = 0; str[i] != '\0'; i++)
    {
      if (str[i] == ' ' || str[i] == '\n')
	{
	  if (i > 0 && str[i] == ' ' && str[i - 1] == ' ')
	    continue;
	  argv[dest] = (char *) malloc ((10) * sizeof (char));
	  strcpy (argv[dest++], word);
	  j = 0;
	}
      else
	{
	  word[j] = str[i];
	  word[++j] = '\0';
	}
    }
  if (argv[wordsCount - 1][strlen (word) - 1] == '&')
    argv[wordsCount - 1][strlen (word) - 1] = '\0';
  if (strcmp (argv[wordsCount - 1], "") == 0)
    argv[wordsCount - 1] = NULL;
  argv[wordsCount] = NULL;
  return argv;
}

/*
main - going with a while loop (endlessly loop unless user insert "done"), printing the current path (using getcwd() function) and wait for user input.
If user insert "done", printing the ammount of commands were inserted by user and number of total words in all commands and exit the loop.
If user usert "history", will call addToHistory() and printHistory() functions, increase totalWords int and continue with the while loop.
If user try to use "cd ..." function, printing "command not supported (yet)" and continue with the loop.
If user insert "!x" (x is a number), so using historyCommand() function, if command was valid, continue to next iteration loop, else, keep going with the loop.
Finaly, using the executeCommand() function and going to next iteration.
*/
int
main ()
{
  signal (SIGCHLD, handler);
  int flag = 1;
  char userS[MAX_LIMIT];
  char cwd[512];
  getcwd (cwd, sizeof (cwd));
  while (flag == 1)
    {
      printf ("%s>", cwd);
      fgets (userS, MAX_LIMIT, stdin);
      if (userS[strlen (userS) - 2] == ' ' || userS[0] == ' ')
	{
	  printf ("Invalid command\n");
	  continue;
	}
	totalWords += wordsCounter(userS);
      if (strcmp (userS, "\n") == 0)
	continue;
      if (strcmp (userS, "done\n") == 0)
	{
	  flag = 0;
	  printf ("Number of commands: %d\n", ++commandsCounter);
	  printf("Total number of words in all commands: %d !\n", --totalWords - pipeUse);
	  continue;
	}
      if (strcmp (userS, "history\n") == 0)
	{
	  addToHistory (userS);
	  printHistory ();
	  commandsCounter++;
	  continue;
	}
      if ((strlen (userS) > 3 && userS[0] == 'c' && userS[1] == 'd'
	   && userS[2] == ' ') || (userS[0] == 'c' && userS[1] == 'd'
				   && strlen (userS) == 3))
	{
	  printf ("command not supported (Yet)\n");
	  commandsCounter++;
	  continue;
	}
      int pipeCount = checkPipe (userS);
      if (pipeCount > 0)
	{
	  if (pipeCount > 2)
	    printf ("Invalid command.\n");
	  else
	    {
	      executePipe (userS, pipeCount);
	      commandsCounter++;
	    }
	  continue;
	}
      else if (userS[0] == '!' && strlen (userS) > 2)
	{
	  strcpy (userS, historyCommand (userS));
	  if (strcmp (userS, "continue") == 0)
	    continue;
	  printf ("%s", userS);
	  int pipeCount = checkPipe (userS);
	  if (pipeCount > 0)
	{
	  if (pipeCount > 2)
	    printf ("Invalid command.\n");
	  else
	    {
	      executePipe (userS, pipeCount);
	      commandsCounter++;
	    }
	  continue;
	}
	}

      executeCommand (userS);
    }
  return 0;
}
