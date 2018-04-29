#include <iostream>
#include <sys/mman.h>
#include <string.h>

int main(int argc, char* argv[]) {

    //function's code
    unsigned char func_code[] = {0x55, 0x48, 0x89, 0xe5, 0x89, 0x7d, 0xfc, 0x8b, 0x45, 0xfc, 0x83, 0xc0, 0x01, 0x5d, 0xc3};
    size_t size = sizeof(func_code);

    //Выделить память с помощью mmap(2)
    //отображение в адресное пространство процесса
    void * new_memory = mmap( NULL, //желаемый адрес начала участка отбраженной памяти
                             size, //количество байт, которое нужно отобразить в память
                             PROT_READ|PROT_WRITE|PROT_EXEC, //число, определяющее степень защищённости отображенного участка памяти
                             MAP_PRIVATE | MAP_ANONYMOUS, //атрибуты области
                             -1, // дескриптор файла, который нужно отобразить
                             0 //смещение отображенного участка от начала файла
    );
    if (new_memory == (void *) -1) {
        perror("Невозможно выделить память");
        exit(errno);
    }

    //Записать в выделенную память машинный код, соответсвующий функции
    memcpy(new_memory, func_code, size);

    //Изменить права на выделенную память - чтение и исполнение. See: mprotect(2)
    if (mprotect(new_memory, size, PROT_EXEC|PROT_READ)) {
        perror("Невозможно выполнить mprotect");
        exit(errno);
    }

    //Вызвать функцию по указателю на выделенную память
    int c =  ((int(*)(int))new_memory) (10);
    std::cout << "Result of inc: " << c << std::endl;


    //Освободить выделенную память
    //отключение отображения объекта в адресное пространство процесса
    int m  = munmap(new_memory, sizeof(func_code));
    if (m == -1) {
        perror("Невозможно выполнить munmap");
        exit(errno);
    }
}
