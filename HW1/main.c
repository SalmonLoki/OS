#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>

int main(int argc, char** argv) {
	char* s;

	while (7) {
        s = NULL;
        size_t bufferSize = 0;
        printf("ヽ(♡‿♡)ノ >> ");

		size_t sLenght = (size_t) getline(&s, &bufferSize, stdin);//возвращ.кол-во прочит.симв. 1 - указ.куда класть 2-длина 1ого арг.  3 - файл чтения


		if (sLenght == -1) break;//конец потока ввода

		size_t argAmount = 0;
		for (size_t i = 0; i < sLenght; i++) {
            if ((i > 0) && (!isspace(s[i-1])))
			    if (isspace(s[i])) argAmount++;
		}


		s[sLenght-1] = '\0';
		char* args[argAmount + 2];//массив ссылок на аргументы
		args[0] = s;
		size_t argPointer = 1;//argPointer - место ля следущ.арг. в вмассиве

		for (size_t i = 0; i < sLenght - 1; i++) {
			if (isspace(s[i])) {
				s[i] = '\0';
                if (!isspace(s[i+1]))
				    args[argPointer++] = s + i + 1;
			}
		}
		args[argPointer] = NULL;
		pid_t taskID = fork();

		if (taskID == -1) {
			printf("Execution failed\n");//что-то не так!!!!!!!!
            return -1;
		}
		if (taskID == 0) {//в дочерн.пр.
			execvp(s, args);
			fprintf(stderr, "exec failed: %d \n", errno); //что-то не так!!!!!!!!
			return -1;
		}
		int taskOut = 0; //снова в род
		waitpid(taskID, &taskOut, 0);
        printf("Program returned %d \n", taskOut);

		free(s);
	}
	printf("\n");
}
