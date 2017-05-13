#include<stdio.h>
#include <unistd.h>

main()
{
        int pid;

        pid=fork();
        
            if(pid==0)
                { 
                  int inVar =0;
                  static int a = 10,args;
                   inVar = a+1;
                  
                  printf("\nI am child Parent PID is %d Executing  \n",getppid());
		  /*  Until this cat /proc/*pid/maps returns same for both parent and child*/
		  
                  execv("./temp.o", args);
		  /* Here cat /proc/*pid/maps returns different for both parent and child*/
                  
                  while(1);
		  /* while loop to avoid exit of the child */
                }
            else
               { printf("\nI am parent child PID is %d\n",pid);
                 while(1);
  		 /* while loop to avoid exit of the Parent */
                }
}
